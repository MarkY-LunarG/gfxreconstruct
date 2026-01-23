/*
** Copyright (c) 2018 Valve Corporation
** Copyright (c) 2018 LunarG, Inc.
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#include <fstream>
#include <filesystem>
#include <util/date_time.h>

#include PROJECT_VERSION_HEADER_FILE

#include "decode/file_processor.h"
#include "decode/vulkan_cpp_utilities.h"
#include "format/format.h"
#include "generated/generated_vulkan_cpp_consumer.h"
#include "generated/generated_vulkan_decoder.h"
#include "util/file_path.h"
#include "util/logging.h"

#include "vulkan/vulkan_core.h"

#include "generated_tocpp_settings.h"

#if defined(WIN32)
const char kPathSep = '\\';
#else
const char kPathSep = '/';
#endif

static std::string android_template_root;
static std::string output_dirname;

// Directory structure where the Vulkan source will be generated as VulkanMain.cpp.
const std::string path_vulkanmain = "app/src/main/jni/";

// Directory structure where the image data will be generated in the output directory.
const std::string path_assets = "app/src/main/assets/";

static bool OutputDirectoryIsValid(std::string& out_dir)
{
    if (out_dir.empty())
    {
        GFXRECON_LOG_ERROR("The output directory is not specified!");
        return false;
    }

    if (gfxrecon::util::filepath::Exists(out_dir))
    {
        if (!gfxrecon::util::filepath::IsDirectory(out_dir))
        {
            GFXRECON_LOG_ERROR("Error while creating directory %s: already exists as file", out_dir.c_str());
            return false;
        }
    }
    else
    {
        int32_t result = gfxrecon::util::platform::MakeDirectory(out_dir.c_str());
        if (result < 0)
        {
            GFXRECON_LOG_ERROR("Error while creating directory %s: could not open", out_dir.c_str());
            return false;
        }
    }

    return true;
}

void ValidateAndConvertDimensionArgument(const std::string& argument, std::vector<uint32_t>& dimensions)
{
    if (!argument.empty())
    {
        std::stringstream argument_stream(argument);
        std::string       str;
        while (std::getline(argument_stream, str, ','))
        {
            size_t processed = 0;
            dimensions.push_back(std::stoi(str, &processed));
        }
    }
}

uint32_t ValidateAndConvertNumericArgument(const std::string& argument, const std::string& error_msg)
{
    uint32_t number = UINT32_MAX;
    if (!argument.empty())
    {
        size_t processed = 0;
        number           = std::stoi(argument, &processed);
        if (processed != argument.length())
        {
            GFXRECON_LOG_ERROR(error_msg.c_str());
            gfxrecon::util::Log::Release();
            exit(-1);
        }
    }

    return number;
}

static bool AndroidDirsExist(const std::string& android_dir)
{
    if (!gfxrecon::util::filepath::IsDirectory(android_dir))
    {
        return false;
    }

    std::string dir_path = gfxrecon::util::filepath::Join(android_dir, path_vulkanmain);
    if (!gfxrecon::util::filepath::IsDirectory(dir_path))
    {
        return false;
    }

    dir_path = gfxrecon::util::filepath::Join(android_dir, path_assets);
    if (!gfxrecon::util::filepath::IsDirectory(dir_path))
    {
        return false;
    }

    return true;
}

static std::string GetOutputFilename(const std::string& capture_file)
{
    std::string output_filename(capture_file);

    // Get the basename of the capture file.
    const size_t last_slash_idx = output_filename.find_last_of("\\/");
    if (last_slash_idx != std::string::npos)
    {
        output_filename.erase(0, last_slash_idx + 1);
    }

    // Replace the .gfxr extension to .cpp.
    size_t suffix_pos = output_filename.find(GFXRECON_FILE_EXTENSION);
    if (suffix_pos != std::string::npos)
    {
        output_filename = output_filename.substr(0, suffix_pos) + ".cpp";
    }

    return gfxrecon::util::filepath::Join(output_dirname, output_filename);
}

bool ProcessCapture(gfxrecon::decode::VulkanCppConsumer&      cpp_consumer,
                    const std::string&                        input_filename,
                    const std::string&                        output_filename,
                    const gfxrecon::decode::GfxToCppPlatform& target_platform,
                    const uint32_t                            frame_limit)
{
    gfxrecon::decode::FileProcessor file_processor;
    gfxrecon::decode::VulkanDecoder decoder;

    printf("Processing capture file %s contents\n", input_filename.c_str());

    if (!file_processor.Initialize(input_filename))
    {
        GFXRECON_LOG_ERROR("Initialization of file processor has failed");
        return false;
    }

    if (!cpp_consumer.Initialize(output_filename, target_platform, output_dirname))
    {
        GFXRECON_LOG_ERROR("Initialization of cpp consumer has failed");
        return false;
    }

    file_processor.AddDecoder(&decoder);
    decoder.AddConsumer(&cpp_consumer);

    bool success;

    do
    {
        printf("  Processing Frame %u\r", static_cast<uint32_t>(file_processor.GetCurrentFrameNumber()));
        fflush(stdout);
        success = file_processor.ProcessNextFrame();
    } while (success && file_processor.GetCurrentFrameNumber() <= frame_limit);
    printf("\nDone processing file\n");

    return (file_processor.GetErrorState() == gfxrecon::decode::BlockIOError::kErrorNone);
}

int main(int argc, const char** argv)
{
    std::string input_filename;

    // Create the tool settings using a smart pointer so it is automatically cleaned up on exit
    std::unique_ptr<gfxrecon::tools::ToolSettings> tool_settings =
        std::make_unique<gfxrecon::tools::ToolSettings>(gfxrecon::util::settings::kGfxrToolType_Optimize_Tool);

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

    const auto& tocpp_settings =
        gfxrecon::util::settings::SettingsManager::GetSingleton().GetSettingsStruct()->tocpp_settings;

    uint32_t command_limit = tocpp_settings.command_limit;
    uint32_t frame_limit   = tocpp_settings.frame_limit;
    android_template_root  = tocpp_settings.android_template;
    output_dirname         = tocpp_settings.tocpp_output;

    gfxrecon::decode::GfxToCppPlatform target_platform =
        tocpp_settings.platform_target.empty()
            ? gfxrecon::decode::GfxToCppPlatform::PLATFORM_XCB
            : gfxrecon::decode::kTargetPlatformByName.at(tocpp_settings.platform_target);

    // Remove the consecutive path separators from the end of the path.
    if (target_platform == gfxrecon::decode::GfxToCppPlatform::PLATFORM_ANDROID)
    {
        while (android_template_root.back() == kPathSep)
        {
            android_template_root.pop_back();
        }
    }

    if (!OutputDirectoryIsValid(output_dirname))
    {
        GFXRECON_LOG_ERROR("Invalid output directory!");
        exit(-1);
    }
    else if (target_platform == gfxrecon::decode::GfxToCppPlatform::PLATFORM_ANDROID &&
             !AndroidDirsExist(android_template_root))
    {
        GFXRECON_LOG_ERROR("The specified path to --android-template option is missing or wrong!");
        exit(-1);
    }
    else
    {
        input_filename = extra_args[0];
    }

    // Determine the output files.
    std::string output_filename;
    if (target_platform == gfxrecon::decode::GfxToCppPlatform::PLATFORM_ANDROID)
    {
        // The maximum number of directories that nftw() will hold open simultaneously.
        const int max_open_fd = 20;

        if (!gfxrecon::util::filepath::Exists(output_dirname))
        {
            GFXRECON_LOG_ERROR("Android template target directory was not found: %s", output_dirname.c_str());
        }
        else
        {
            const std::filesystem::copy_options copy_options =
                std::filesystem::copy_options::update_existing | std::filesystem::copy_options::recursive;
            std::filesystem::copy(android_template_root.c_str(), output_dirname, copy_options);
        }

        output_filename = gfxrecon::util::filepath::Join(output_dirname, path_vulkanmain + "VulkanMain.cpp");
        output_dirname  = gfxrecon::util::filepath::Join(output_dirname, path_assets);
    }
    else // target_platform == gfxrecon::decode::GfxToCppPlatform::PLATFORM_XCB
    {
        output_filename = GetOutputFilename(input_filename);
    }

    std::vector<uint32_t> dimensions;
    ValidateAndConvertDimensionArgument(tocpp_settings.max_window_dimensions, dimensions);

    gfxrecon::decode::VulkanCppConsumer cpp_consumer;
    bool                                result;

    if (tocpp_settings.captured_swapchain)
    {
        cpp_consumer.DisableVirtualSwapchain();
    }

    // On non-Android platforms, let's not split frames if we can avoid it
    if (command_limit > 0)
    {
        cpp_consumer.SetMaxCommandLimit(command_limit);
    }

    int64_t process_start_time = gfxrecon::util::datetime::GetTimestamp();
    result = ProcessCapture(cpp_consumer, input_filename, output_filename, target_platform, frame_limit);
    int64_t  process_end_time           = gfxrecon::util::datetime::GetTimestamp();
    uint32_t cpp_consumer_apicall_count = cpp_consumer.GetCurrentApiCallNumber();
    if (result)
    {
        GFXRECON_LOG_INFO("Capture file processed %u calls in %lf seconds",
                          cpp_consumer_apicall_count,
                          gfxrecon::util::datetime::ConvertTimestampToSeconds(
                              gfxrecon::util::datetime::DiffTimestamps(process_start_time, process_end_time)));
    }
    else
    {
        GFXRECON_LOG_INFO("Failed to process capture file")
    }

    gfxrecon::util::Log::Release();
    return 0;
}
