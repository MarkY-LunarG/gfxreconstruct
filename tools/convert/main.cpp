/*
** Copyright (c) 2018-2023 Valve Corporation
** Copyright (c) 2018-2023 LunarG, Inc.
** Copyright (c) 2020 Advanced Micro Devices, Inc.
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
#include <string>
#include PROJECT_VERSION_HEADER_FILE
#include "decode/json_writer.h" /// @todo move to util?
#include "decode/decode_api_detection.h"
#include "format/format.h"
#include "util/file_output_stream.h"
#include "util/file_path.h"
#include "util/platform.h"
#include "util/strings.h"

#if ENABLE_OPENXR_SUPPORT
#include "generated/generated_openxr_json_consumer.h"
#endif

#include "generated/generated_vulkan_json_consumer.h"
#include "decode/marker_json_consumer.h"
#include "decode/metadata_json_consumer.h"
#if defined(D3D12_SUPPORT)
#include "generated/generated_dx12_json_consumer.h"
#endif

#include "generated_convert_settings.h"

using gfxrecon::util::JsonFormat;

#if ENABLE_OPENXR_SUPPORT
using OpenXrJsonConsumer = gfxrecon::decode::MetadataJsonConsumer<
    gfxrecon::decode::MarkerJsonConsumer<gfxrecon::decode::OpenXrExportJsonConsumer>>;
#endif

using VulkanJsonConsumer = gfxrecon::decode::MetadataJsonConsumer<
    gfxrecon::decode::MarkerJsonConsumer<gfxrecon::decode::VulkanExportJsonConsumer>>;

#if defined(D3D12_SUPPORT)
using Dx12JsonConsumer =
    gfxrecon::decode::MetadataJsonConsumer<gfxrecon::decode::MarkerJsonConsumer<gfxrecon::decode::Dx12JsonConsumer>>;
#endif

static void GetOutputFileName(const std::string& output_dir,
                              const std::string& input_filename,
                              JsonFormat         output_format,
                              bool&              output_to_stdout,
                              std::string&       output_stem,
                              std::string&       output_filename,
                              std::string&       output_data_dir)
{
    if (!output_dir.empty())
    {
        output_filename = output_dir;
        output_to_stdout = (output_filename == "stdout");
    }
    else
    {
        output_filename   = input_filename;
        auto ext_pos      = output_filename.find_last_of(".");
        auto path_sep_pos = output_filename.find_last_of(gfxrecon::util::filepath::kPathSepStr);
        if (ext_pos != std::string::npos && (path_sep_pos == std::string::npos || ext_pos > path_sep_pos))
        {
            output_filename = output_filename.substr(0, ext_pos);
        }
        output_filename += "." + gfxrecon::util::get_json_format(output_format);

        output_to_stdout = false;
    }

    // If we're outputing to stdout, we still need to use a data filename using the
    // capture file prefix
    std::string output_base;
    if (output_to_stdout)
    {
        output_stem = gfxrecon::util::filepath::GetFilenameStem(input_filename);
        output_base = gfxrecon::util::filepath::GetBasedir(input_filename);
    }
    else
    {
        output_stem = gfxrecon::util::filepath::GetFilenameStem(output_filename);
        output_base = gfxrecon::util::filepath::GetBasedir(output_filename);
    }
    output_data_dir = gfxrecon::util::filepath::Join(output_base, output_stem);
}

static bool GetFrameIndices(const std::string& input_ranges, std::vector<uint32_t>& indices)
{
    bool indices_present = false;
    if (!input_ranges.empty())
    {
        std::vector<gfxrecon::util::UintRange> frame_ranges =
            gfxrecon::util::GetUintRanges(input_ranges.c_str(), "frames to be converted", true, true);

        for (uint32_t i = 0; i < frame_ranges.size(); ++i)
        {
            gfxrecon::util::UintRange& range = frame_ranges[i];

            uint32_t diff = range.last - range.first + 1;

            for (uint32_t j = 0; j < diff; ++j)
            {
                uint32_t frame_index = range.first + j;

                indices.push_back(frame_index);
            }
        }

        std::sort(indices.begin(), indices.end());
        std::reverse(indices.begin(), indices.end());
        indices_present = true;
    }
    return indices_present;
}

std::string FormatFrameNumber(uint32_t frame_number)
{
    std::ostringstream stream;
    stream << std::setfill('0') << std::setw(5) << frame_number;
    return stream.str();
}

int main(int argc, const char** argv)
{
    int ret_code = 0;

    // Create the tool settings using a smart pointer so it is automatically cleaned up on exit
    std::unique_ptr<gfxrecon::tools::ToolSettings> tool_settings =
        std::make_unique<gfxrecon::tools::ToolSettings>(gfxrecon::util::settings::kGfxrToolType_Convert_Tool);

    std::vector<std::string>        extra_args;
    gfxrecon::tools::CmdLineRetType ret_type = tool_settings->ProcessCommandLine(argc, argv, 1, extra_args);

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

    const auto& convert_settings =
        gfxrecon::util::settings::SettingsManager::GetSingleton().GetSettingsStruct()->convert_settings;

    if (convert_settings.json_output.empty())
    {
        exit(1);
    }

    std::string filename_stem;
    std::string output_filename;
    std::string output_dir;
    bool        output_to_stdout;
    std::string input_filename = extra_args[0];
    JsonFormat  output_format  = gfxrecon::util::get_json_format(convert_settings.json_format);

    GetOutputFileName(convert_settings.json_output,
                      input_filename,
                      output_format,
                      output_to_stdout,
                      filename_stem,
                      output_filename,
                      output_dir);

    bool                  dump_binaries  = convert_settings.include_binaries;
    bool                  expand_flags   = convert_settings.expand_flags;
    bool                  file_per_frame = convert_settings.file_per_frame;
    std::vector<uint32_t> frame_indices;
    bool                  frame_range_option = GetFrameIndices(convert_settings.frame_range, frame_indices);

    bool   is_asset_file = false;
    size_t last_dot_pos  = input_filename.find_last_of(".");
    if (last_dot_pos != std::string::npos)
    {
        if (!input_filename.compare(last_dot_pos, 5, ".gfxa"))
        {
            is_asset_file = true;
        }
    }

    gfxrecon::decode::FileProcessor file_processor;

#ifndef D3D12_SUPPORT
    bool detected_d3d12  = false;
    bool detected_vulkan = false;
    bool detected_openxr = false;
    gfxrecon::decode::DetectAPIs(input_filename, detected_d3d12, detected_vulkan, detected_openxr);

    if (!detected_vulkan && !is_asset_file)
    {
        GFXRECON_LOG_INFO("Capture file does not contain Vulkan content.  D3D12 content may be present but "
                          "gfxrecon-convert is not compiled with D3D12 support.");
        goto exit;
    }
#endif
    if (file_per_frame && output_to_stdout)
    {
        GFXRECON_LOG_WARNING("Outputting a file per frame is not consistent with outputting to stdout.");
        file_per_frame = false;
    }

    if (dump_binaries)
    {
        gfxrecon::util::filepath::MakeDirectory(output_dir);
    }

    if (file_processor.Initialize(input_filename))
    {
        std::string json_filename;
        FILE*       out_file_handle = nullptr;
        FILE*       tmp_file_handle = nullptr;

        if (file_per_frame)
        {
            uint32_t cur_frame_number = static_cast<uint32_t>(file_processor.GetCurrentFrameNumber());
            if (frame_range_option)
            {
                cur_frame_number = frame_indices.back();
            }
            json_filename = gfxrecon::util::filepath::InsertFilenamePostfix(output_filename,
                                                                            +"_" + FormatFrameNumber(cur_frame_number));
        }
        else
        {
            json_filename = output_filename;
        }

        if (output_to_stdout)
        {
            out_file_handle = stdout;
        }
        else
        {
            gfxrecon::util::platform::FileOpen(&out_file_handle, json_filename.c_str(), "w");
        }

        if (!out_file_handle)
        {
            GFXRECON_LOG_ERROR("Failed to open/create output file \"%s\"; is the path valid?", output_filename.c_str());
            ret_code = 1;
        }
        else
        {
            gfxrecon::util::FileNoLockOutputStream out_stream{ out_file_handle, false };

            VulkanJsonConsumer              vulkan_json_consumer;
            gfxrecon::decode::VulkanDecoder vulkan_decoder;
            vulkan_decoder.AddConsumer(&vulkan_json_consumer);
            file_processor.AddDecoder(&vulkan_decoder);

#if ENABLE_OPENXR_SUPPORT
            OpenXrJsonConsumer              openxr_json_consumer;
            gfxrecon::decode::OpenXrDecoder openxr_decoder;
            openxr_decoder.AddConsumer(&openxr_json_consumer);
            file_processor.AddDecoder(&openxr_decoder);
#endif

            gfxrecon::util::JsonOptions json_options;
            json_options.root_dir      = output_dir;
            json_options.data_sub_dir  = filename_stem;
            json_options.format        = output_format;
            json_options.dump_binaries = dump_binaries;
            json_options.expand_flags  = expand_flags;

            gfxrecon::decode::JsonWriter json_writer{ json_options, GetProjectVersionString(), input_filename };
            file_processor.SetAnnotationProcessor(&json_writer);

            bool              success = true;
            const std::string vulkan_version{ std::to_string(VK_VERSION_MAJOR(VK_HEADER_VERSION_COMPLETE)) + "." +
                                              std::to_string(VK_VERSION_MINOR(VK_HEADER_VERSION_COMPLETE)) + "." +
                                              std::to_string(VK_VERSION_PATCH(VK_HEADER_VERSION_COMPLETE)) };
            vulkan_json_consumer.Initialize(&json_writer, vulkan_version);

#if ENABLE_OPENXR_SUPPORT
            const std::string openxr_version{ std::to_string(XR_VERSION_MAJOR(XR_CURRENT_API_VERSION)) + "." +
                                              std::to_string(XR_VERSION_MINOR(XR_CURRENT_API_VERSION)) + "." +
                                              std::to_string(XR_VERSION_PATCH(XR_CURRENT_API_VERSION)) };
            openxr_json_consumer.Initialize(&json_writer, openxr_version);
#endif

            json_writer.StartStream(&out_stream);

            if (frame_range_option)
            {
                tmp_file_handle = tmpfile();
                if (tmp_file_handle == nullptr)
                {
                    ret_code = 1;
                    success  = false;
                    GFXRECON_LOG_ERROR("Failed to create temp file for storing non-dumped frame range info.");
                }

                // If we are supposed to start converting immediately, then direct the outputto the actual
                // output file.  Otherwise, point it at a temporary file whose contents will be ignored.
                if (frame_indices.back() == file_processor.GetCurrentFrameNumber())
                {
                    out_stream.Reset(out_file_handle);
                    frame_indices.pop_back();
                }
                else
                {
                    out_stream.Reset(tmp_file_handle);
                }
            }

#ifdef D3D12_SUPPORT
            // If D3D12_SUPPORT was set, then add DX12 consumer/decoder
            Dx12JsonConsumer              dx12_json_consumer;
            gfxrecon::decode::Dx12Decoder dx12_decoder;

            dx12_decoder.AddConsumer(&dx12_json_consumer);
            file_processor.AddDecoder(&dx12_decoder);
            auto dx12_json_flags = output_format == JsonFormat::JSON ? gfxrecon::util::kToString_Formatted
                                                                     : gfxrecon::util::kToString_Unformatted;
            dx12_json_consumer.Initialize(&json_writer);
#endif

            while (success)
            {
                success = file_processor.ProcessNextFrame();

                if (success)
                {
                    if (frame_range_option)
                    {
                        if (frame_indices.empty())
                        {
                            break;
                        }

                        // If we are supposed to start converting, then direct the output to the actual
                        // output file.  Otherwise, point it at a temporary file whose contents will be ignored.
                        if (frame_indices.back() == file_processor.GetCurrentFrameNumber())
                        {
                            out_stream.Reset(out_file_handle);
                            json_filename = gfxrecon::util::filepath::InsertFilenamePostfix(
                                output_filename, +"_" + FormatFrameNumber(frame_indices.back()));
                            frame_indices.pop_back();
                        }
                        else
                        {
                            out_stream.Reset(tmp_file_handle);
                            continue;
                        }
                    }

                    if (file_per_frame)
                    {
                        json_writer.EndStream();
                        gfxrecon::util::platform::FileClose(out_file_handle);

                        json_filename = (frame_range_option)
                                            ? json_filename
                                            : gfxrecon::util::filepath::InsertFilenamePostfix(
                                                  output_filename,
                                                  +"_" + FormatFrameNumber(file_processor.GetCurrentFrameNumber()));

                        gfxrecon::util::platform::FileOpen(&out_file_handle, json_filename.c_str(), "w");
                        success = out_file_handle != nullptr;
                        if (success)
                        {
                            out_stream.Reset(out_file_handle);
                            json_writer.StartStream(&out_stream);
                        }
                        else
                        {
                            GFXRECON_LOG_ERROR("Failed to create file: '%s'.", json_filename.c_str());
                            ret_code = 1;
                        }
                    }
                }
            }

            vulkan_json_consumer.Destroy();

#if ENABLE_OPENXR_SUPPORT
            openxr_json_consumer.Destroy();
#endif

            // If CONVERT_EXPERIMENTAL_D3D12 was set, then cleanup DX12 consumer
#ifdef D3D12_SUPPORT
            dx12_json_consumer.Destroy();
#endif

            if (tmp_file_handle != nullptr)
            {
                gfxrecon::util::platform::FileClose(tmp_file_handle);
                tmp_file_handle = nullptr;
            }

            if (!output_to_stdout)
            {
                gfxrecon::util::platform::FileClose(out_file_handle);
                out_file_handle = nullptr;
            }

            if (file_processor.GetErrorState() != gfxrecon::decode::BlockIOError::kErrorNone)
            {
                GFXRECON_LOG_ERROR("Failed to process trace.");
                ret_code = 1;
            }
        }
    }
    goto exit; // The other goto is inside an #ifdef and if compiled-out an unreferenced label warning results.
exit:
    gfxrecon::util::Log::Release();
    return ret_code;
}
