/*
** Copyright (c) 2018-2020 Valve Corporation
** Copyright (c) 2018-2025 LunarG, Inc.
** Copyright (c) 2022-2023 Advanced Micro Devices, Inc. All rights reserved.
**
** Permission is hereby granted, free of charge, to any person obtaining a
** copy of this software and associated documentation files (the "Software"),
** to deal in the Software without restriction, including without limitation
** the rights to use, copy, modify, merge, publish, distribute, sublicense,
** and/or sell copies of the Software, and to permit persons to whom the
** Software is furnished to do so, subject to the following conditions:
**
** The above copyright notice and this permission notice shall be included in
** all copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
** LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
** FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
** DEALINGS IN THE SOFTWARE.
*/

#include "replay_settings.h"

#include "application/application.h"
#include "decode/file_processor.h"
#include "decode/preload_file_processor.h"

#include "graphics/fps_info.h"
#include "util/argument_parser.h"
#include "util/feature_module_registry.h"
#include "util/logging.h"

#include "parse_dump_resources_cli.h"

#include "replay_feature.h"

#include <exception>
#include <memory>
#include <stdexcept>

static std::vector<std::unique_ptr<gfxrecon::replay::ReplayFeatureBase>> g_features;

#if defined(D3D12_SUPPORT)

extern "C"
{
    __declspec(dllexport) extern const UINT D3D12SDKVersion = 618;
}
extern "C"
{
    __declspec(dllexport) extern const char* D3D12SDKPath = reinterpret_cast<const char*>(u8".\\D3D12\\");
}

#include <conio.h>
void WaitForExit()
{
    DWORD process_list[2];
    DWORD result = GetConsoleProcessList(process_list, ARRAYSIZE(process_list));

    // If the process list contains a single entry, we assume that the console was created when the gfxrecon-replay.exe
    // process started, and will be destroyed when it exits.  In this case, we will wait on user input before exiting
    // and closing the console window to give the user a chance to read any console output.
    if (result <= 1)
    {
        GFXRECON_WRITE_CONSOLE("\nPress any key to close this window . . .");
        while (!_kbhit())
        {
            Sleep(250);
        }
    }
}
#else
void WaitForExit() {}
#endif

void RunPreProcessConsumer(const std::string& input_filename)
{
    gfxrecon::decode::FileProcessor file_processor;
    if (file_processor.Initialize(input_filename))
    {
        for (auto& feature : g_features)
        {
            feature->SetupPreProcessingPass(&file_processor);
        }

        file_processor.ProcessAllFrames();

        for (auto& feature : g_features)
        {
            feature->CompletePreProcessingPass();
        }
    }
}

const char kLayerEnvVar[] = "VK_INSTANCE_LAYERS";

int main(int argc, const char** argv)
{
    int return_code = 0;

    // Default initialize logging to report issues while loading settings.
    gfxrecon::util::Log::Init(gfxrecon::decode::kDefaultLogLevel);

    // Query the module registry for registered modules, and
    // call each generator here and put the unique_ptr into our
    // internal unique_ptr vector.
    for (const auto& registered_creator :
         gfxrecon::util::FeatureModuleRegistry<gfxrecon::replay::ReplayFeatureBase>::GetSingleton()
             .GetRegisteredFeatureCreators())
    {
        g_features.push_back(registered_creator());
    }

    gfxrecon::util::ArgumentParser arg_parser(argc, argv, kOptions, kArguments);

    if (CheckOptionPrintVersion(argv[0], arg_parser) || CheckOptionPrintUsage(argv[0], arg_parser))
    {
        gfxrecon::util::Log::Release();
        exit(0);
    }
    else if (arg_parser.IsInvalid() || (arg_parser.GetPositionalArgumentsCount() != 1))
    {
        PrintUsage(argv[0]);
        gfxrecon::util::Log::Release();
        exit(-1);
    }
    else
    {
        ProcessDisableDebugPopup(arg_parser);
    }

    // Update logging with values retrieved from command line arguments
    gfxrecon::util::Log::Settings log_settings;
    GetLogSettings(arg_parser, log_settings);
    gfxrecon::util::Log::UpdateWithSettings(log_settings);

    try
    {
        const std::vector<std::string>& positional_arguments = arg_parser.GetPositionalArguments();
        std::string                     filename             = positional_arguments[0];

        std::unique_ptr<gfxrecon::decode::FileProcessor> file_processor;

        uint32_t loop_frame        = 0;
        uint32_t loop_count        = gfxrecon::graphics::FrameLoopInfo::INFINITE_ITERATIONS;
        bool     enable_frame_loop = GetLoopFrame(arg_parser, loop_frame);
        GetLoopCount(arg_parser, loop_count);

        if (arg_parser.IsOptionSet(kPreloadMeasurementRangeOption) || enable_frame_loop)
        {
            file_processor = std::make_unique<gfxrecon::decode::PreloadFileProcessor>();
        }
        else
        {
            file_processor = std::make_unique<gfxrecon::decode::FileProcessor>();
        }

        if (!file_processor->Initialize(filename))
        {
            return_code = -1;
        }
        else
        {
            bool        has_mfr                            = false;
            bool        requires_pre_processing            = false;
            bool        quit_after_frame                   = false;
            uint32_t    quit_frame                         = std::numeric_limits<uint32_t>::max();
            uint32_t    measurement_start_frame            = 0;
            uint32_t    measurement_end_frame              = 0;
            bool        quit_after_measurement_frame_range = false;
            bool        flush_measurement_frame_range      = false;
            bool        flush_inside_measurement_range     = false;
            bool        preload_measurement_frame_range    = false;
            std::string measurement_file_name;

            // Select WSI context based on CLI
            std::string wsi_extension = GetWsiExtensionName(GetWsiPlatform(arg_parser));
            auto        application   = std::make_shared<gfxrecon::application::Application>(
                kApplicationName, file_processor.get(), wsi_extension, nullptr);

            for (auto& feature : g_features)
            {
                feature->QueryOptions(arg_parser, filename);
            }

            has_mfr = GetMeasurementFrameRange(arg_parser, measurement_start_frame, measurement_end_frame);
            GetMeasurementFilename(arg_parser, measurement_file_name);

            for (auto& feature : g_features)
            {
                feature->SetMeasurementStartFrame(measurement_start_frame);
                feature->QueryFpsInfoOptions(quit_after_measurement_frame_range,
                                             flush_measurement_frame_range,
                                             flush_inside_measurement_range,
                                             preload_measurement_frame_range,
                                             quit_after_frame);
            }
            if (quit_after_frame)
            {
                GetQuitAfterFrame(arg_parser, quit_frame);
            }

            gfxrecon::graphics::FpsInfo fps_info(static_cast<uint64_t>(measurement_start_frame),
                                                 static_cast<uint64_t>(measurement_end_frame),
                                                 has_mfr,
                                                 quit_after_measurement_frame_range,
                                                 flush_measurement_frame_range,
                                                 flush_inside_measurement_range,
                                                 preload_measurement_frame_range,
                                                 measurement_file_name,
                                                 quit_after_frame,
                                                 quit_frame);

            gfxrecon::graphics::FrameLoopInfo  fl_info;
            gfxrecon::graphics::FrameLoopInfo* fl_info_ptr = nullptr;
            if (enable_frame_loop)
            {
                fl_info     = gfxrecon::graphics::FrameLoopInfo(loop_frame, loop_count);
                fl_info_ptr = &fl_info;
                application->SetFrameLoopInfo(fl_info_ptr);
            }

            for (auto& feature : g_features)
            {
                feature->CreateConsumer(file_processor.get(), application, fl_info_ptr);
                requires_pre_processing |= feature->NeedsPreProcessingPass();
                feature->DetectAndSetupRecapture();
            }

            // NOTE: This must be called after each feature has created their consumers.
            for (auto& feature : g_features)
            {
                feature->LinkCompositionFeatures(g_features);
            }

            if (requires_pre_processing)
            {
                RunPreProcessConsumer(filename);
            }

            for (auto& feature : g_features)
            {
                feature->RegisterDecodeComponents(&fps_info);
            }

            // Warn if the capture layer is active.
            CheckActiveLayers(gfxrecon::util::platform::GetEnv(kLayerEnvVar));

            fps_info.BeginFile();

            application->SetPauseFrame(GetPauseFrame(arg_parser));
            application->SetFpsInfo(&fps_info);
            application->Run();

            // XXX if the final frame ended with a Present, this would be the *next* frame
            // Add one so that it matches the trim range frame number semantic
            fps_info.EndFile(file_processor->GetCurrentFrameNumber() + 1);

            if ((file_processor->GetCurrentFrameNumber() > 0) &&
                (file_processor->GetErrorState() == gfxrecon::decode::BlockIOError::kErrorNone))
            {
                if (file_processor->GetCurrentFrameNumber() < measurement_start_frame)
                {
                    GFXRECON_LOG_WARNING(
                        "Measurement range start frame (%u) is greater than the last replayed frame (%u). "
                        "Measurements were never started, cannot calculate measurement range FPS.",
                        measurement_start_frame,
                        file_processor->GetCurrentFrameNumber());
                }
                else
                {
                    fps_info.LogMeasurements();
                }
            }
            else if (file_processor->GetErrorState() != gfxrecon::decode::BlockIOError::kErrorNone)
            {
                GFXRECON_WRITE_CONSOLE("A failure has occurred during replay");
                return_code = -1;
            }
            else
            {
                GFXRECON_WRITE_CONSOLE("File did not contain any frames");
            }

            for (auto& feature : g_features)
            {
                feature->Destroy();
            }
        }
    }
    catch (const std::runtime_error& error)
    {
        GFXRECON_WRITE_CONSOLE("Replay has encountered a fatal error and cannot continue: %s", error.what());
        return_code = -1;
    }
    catch (const std::exception& error)
    {
        GFXRECON_WRITE_CONSOLE("Replay has encountered a fatal error and cannot continue: %s", error.what());
        return_code = -1;
    }
    catch (...)
    {
        GFXRECON_WRITE_CONSOLE("Replay failed due to an unhandled exception");
        return_code = -1;
    }

    WaitForExit();

    gfxrecon::util::Log::Release();

    return return_code;
}
