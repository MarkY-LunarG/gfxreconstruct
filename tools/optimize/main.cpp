/*
** Copyright (c) 2020 LunarG, Inc.
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

#include PROJECT_VERSION_HEADER_FILE
#include "file_optimizer.h"

#if defined(D3D12_SUPPORT)
#include "dx12_optimize_util.h"
#endif

#include "decode/decode_api_detection.h"
#include "decode/dx12_optimize_options.h"
#include "decode/file_processor.h"
#include "format/format.h"
#include "format/format_util.h"
#include "generated/generated_vulkan_decoder.h"
#include "generated/generated_vulkan_referenced_resource_consumer.h"
#include "util/logging.h"
#include "util/date_time.h"

#include "vulkan/vulkan.h"

#include "generated_optimize_settings.h"

#include <cassert>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <vector>

#if defined(WIN32)
extern "C"
{
    __declspec(dllexport) extern const UINT D3D12SDKVersion = 616;
}
extern "C"
{
    __declspec(dllexport) extern const char* D3D12SDKPath = reinterpret_cast<const char*>(u8".\\D3D12\\");
}
#endif

void GetUnreferencedResources(const std::string&                              input_filename,
                              std::unordered_set<gfxrecon::format::HandleId>* unreferenced_ids)
{
    GFXRECON_ASSERT(unreferenced_ids != nullptr);

    gfxrecon::decode::FileProcessor file_processor;
    if (file_processor.Initialize(input_filename))
    {
        gfxrecon::decode::VulkanDecoder                    decoder;
        gfxrecon::decode::VulkanReferencedResourceConsumer resref_consumer;

        decoder.AddConsumer(&resref_consumer);

        file_processor.AddDecoder(&decoder);
        file_processor.ProcessAllFrames();

        if (resref_consumer.WasNotOptimizable())
        {
            GFXRECON_WRITE_CONSOLE("File did not contain trim state setup - no optimization was performed");
            gfxrecon::util::Log::Release();
            exit(65);
        }
        else if ((file_processor.GetCurrentFrameNumber() > 0) &&
                 (file_processor.GetErrorState() == gfxrecon::decode::BlockIOError::kErrorNone))
        {
            // Get the list of resources that were included in a command buffer submission during replay.
            resref_consumer.GetReferencedResourceIds(nullptr, unreferenced_ids);
        }
        else if (file_processor.GetErrorState() != gfxrecon::decode::BlockIOError::kErrorNone)
        {
            GFXRECON_WRITE_CONSOLE("A failure has occurred during file processing");
            gfxrecon::util::Log::Release();
            exit(-1);
        }
        else
        {
            GFXRECON_WRITE_CONSOLE("File did not contain any frames");
            gfxrecon::util::Log::Release();
            exit(0);
        }
    }
}

void FilterUnreferencedResources(const std::string&                               input_filename,
                                 const std::string&                               output_filename,
                                 std::unordered_set<gfxrecon::format::HandleId>&& unreferenced_ids)
{
    gfxrecon::FileOptimizer file_processor(std::move(unreferenced_ids));
    if (file_processor.Initialize(input_filename, output_filename))
    {
        file_processor.Process();

        if (file_processor.GetErrorState() != gfxrecon::decode::BlockIOError::kErrorNone)
        {
            GFXRECON_WRITE_CONSOLE("A failure has occurred during file processing");
            gfxrecon::util::Log::Release();
            exit(-1);
        }

        GFXRECON_WRITE_CONSOLE("Resource filtering complete.");
        GFXRECON_WRITE_CONSOLE("\tOriginal file size: %" PRIu64 " bytes", file_processor.GetNumBytesRead());
        GFXRECON_WRITE_CONSOLE("\tOptimized file size: %" PRIu64 " bytes", file_processor.GetNumBytesWritten());
    }
}

void VkRemoveRedundantResources(std::string input_filename, std::string output_filename)
{
    GFXRECON_WRITE_CONSOLE("Scanning Vulkan file %s for unreferenced resources.", input_filename.c_str());
    std::unordered_set<gfxrecon::format::HandleId> unreferenced_ids;
    GetUnreferencedResources(input_filename, &unreferenced_ids);

    if (!unreferenced_ids.empty())
    {
        // Filter unreferenced ids.
        GFXRECON_WRITE_CONSOLE("Writing optimized file, removing initialization data for %" PRIu64 " unused resources.",
                               unreferenced_ids.size());
        FilterUnreferencedResources(input_filename, output_filename, std::move(unreferenced_ids));
    }
    else
    {
        GFXRECON_WRITE_CONSOLE("No unused resources detected.  A new file will not be created.",
                               input_filename.c_str());
    }
}

void RunDx12Optimizations(const std::string&                        input_filename,
                          const std::string&                        output_filename,
                          gfxrecon::decode::Dx12OptimizationOptions dx12_options)
{
#if defined(D3D12_SUPPORT)
    bool result = gfxrecon::Dx12OptimizeFile(input_filename, output_filename, dx12_options);
    if (!result)
    {
        gfxrecon::util::Log::Release();
        exit(-1);
    }
#endif
}

int main(int argc, const char** argv)
{
    int64_t start_time = gfxrecon::util::datetime::GetTimestamp();

    // Create the tool settings using a smart pointer so it is automatically cleaned up on exit
    std::unique_ptr<gfxrecon::tools::ToolSettings> tool_settings =
        std::make_unique<gfxrecon::tools::ToolSettings>(gfxrecon::util::settings::kGfxrToolType_Optimize_Tool);

    std::vector<std::string>        extra_args;
    gfxrecon::tools::CmdLineRetType ret_type = tool_settings->ProcessCommandLine(argc, argv, 2, extra_args);

    if (ret_type == gfxrecon::tools::CmdLineRetType_PrintUsage)
    {
        PrintUsage(argv[0]);
        exit(0);
    }
    else if (ret_type == gfxrecon::tools::CmdLineRetType_PrintVersion)
    {
        gfxrecon::tools::PrintVersion(argv[0]);
        exit(0);
    }
    else if (ret_type == gfxrecon::tools::CmdLineRetType_Error)
    {
        PrintUsage(argv[0]);
        exit(-1);
    }
    tool_settings->ProcessDisableDebugPopup();

    const auto& optimize_settings =
        gfxrecon::util::settings::SettingsManager::GetSingleton().GetSettingsStruct()->optimize_settings;

    try
    {
        std::string input_filename  = extra_args[0];
        std::string output_filename = extra_args[1];

        // Parameter checking and API detection
        gfxrecon::decode::Dx12OptimizationOptions dx12_options;
        dx12_options.optimize_resource_values              = optimize_settings.dxr;
        dx12_options.optimize_resource_values_experimental = optimize_settings.dxr_experimental;
        dx12_options.remove_redundant_psos                 = optimize_settings.d3d12_pso_removal;
        const auto& override_gpu                           = optimize_settings.gpu;
        if (!override_gpu.empty())
        {
            dx12_options.override_gpu_index = std::stoi(override_gpu);
        }

        if (dx12_options.optimize_resource_values_experimental)
        {
            GFXRECON_WRITE_CONSOLE("Running experimental DXR optimization. This mode is experimental, and should only "
                                   "be used if --dxr did not produce a valid capture file.");
            dx12_options.optimize_resource_values = true;
        }

        // Automatic mode. User specified no options.
        if ((dx12_options.optimize_resource_values == false) && (dx12_options.remove_redundant_psos == false))
        {
            bool detected_d3d12  = false;
            bool detected_vulkan = false;
            bool detected_openxr = false;
            gfxrecon::decode::DetectAPIs(input_filename, detected_d3d12, detected_vulkan, detected_openxr);

            if ((!detected_d3d12) && (!detected_vulkan))
            {
                // Detect with no block limit
                gfxrecon::decode::DetectAPIs(input_filename, detected_d3d12, detected_vulkan, detected_openxr, true);
            }

            if (detected_d3d12)
            {
                dx12_options.optimize_resource_values = true;
                dx12_options.remove_redundant_psos    = true;
                RunDx12Optimizations(input_filename, output_filename, dx12_options);
            }
            else if (detected_vulkan)
            {
                VkRemoveRedundantResources(input_filename, output_filename);
            }
#if ENABLE_OPENXR_SUPPORT
            else if (detected_openxr)
            {
                GFXRECON_LOG_INFO("No optimizations defined for OpenXR capture files");
            }
#endif
            else
            {
                GFXRECON_LOG_ERROR("Could not detect graphics API. Aborting optimization.")
            }
        }
        // Manual mode. Follow user instructions.
        else
        {
            RunDx12Optimizations(input_filename, output_filename, dx12_options);
        }
    }
    catch (const std::runtime_error& error)
    {
        GFXRECON_WRITE_CONSOLE("File processing has encountered a fatal error and cannot continue: %s", error.what());
        gfxrecon::util::Log::Release();
        return -1;
    }
    catch (...)
    {
        GFXRECON_WRITE_CONSOLE("File processing failed due to an unhandled exception");
        gfxrecon::util::Log::Release();
        return -1;
    }

    int64_t end_time        = gfxrecon::util::datetime::GetTimestamp();
    int     time_in_seconds = static_cast<int>(gfxrecon::util::datetime::ConvertTimestampToSeconds(
        gfxrecon::util::datetime::DiffTimestamps(start_time, end_time)));
    GFXRECON_WRITE_CONSOLE("File processing time: %d seconds", time_in_seconds);

    gfxrecon::util::Log::Release();
    return 0;
}
