/*
** Copyright (c) 2019-2025 LunarG, Inc.
** Copyright (c) 2021-2025 Advanced Micro Devices, Inc. All rights reserved.
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

#include "tool_settings.h"

#include <sstream>

const char kOldLoaderLayerEnableEnvVar[] = "VK_INSTANCE_LAYERS";
const char kNewLoaderLayerEnableEnvVar[] = "VK_LOADER_LAYERS_ENABLE";
const char kLoaderLayerDisableEnvVar[]   = "VK_LOADER_LAYERS_DISABLE";

GFXRECON_BEGIN_NAMESPACE(gfxrecon)
GFXRECON_BEGIN_NAMESPACE(tools)

ToolSettings::ToolSettings(gfxrecon::util::settings::GfxrToolType tool_type) : tool_type_(tool_type)
{
    gfxrecon::util::Log::Init(gfxrecon::decode::kDefaultLogLevel);
    gfxrecon::util::settings::SettingsManager::InitSingleton(format::ApiFamily_None);
}

ToolSettings::~ToolSettings()
{
    gfxrecon::util::Log::Release();
}

bool ToolSettings::GenerateArgumentVector(int32_t                   argc,
                                          const char** const        argv,
                                          std::vector<std::string>& cmd_line_vector)
{
    if (argc > 1 && nullptr != argv)
    {
        cmd_line_vector.resize(argc - 1);
        for (int32_t cur_arg = 1; cur_arg < argc; ++cur_arg)
        {
            cmd_line_vector[cur_arg - 1] = argv[cur_arg];

            // Strip off any quotes surrounding the whole string
            if (cmd_line_vector[cur_arg - 1].front() == '\"')
            {
                cmd_line_vector[cur_arg - 1].erase(cmd_line_vector[cur_arg - 1].begin());
            }
            if (cmd_line_vector[cur_arg - 1].back() == '\"')
            {
                cmd_line_vector[cur_arg - 1].pop_back();
            }
        }
        return true;
    }
    return false;
}

bool ToolSettings::GenerateArgumentVector(std::string command_line_string, std::vector<std::string>& cmd_line_vector)
{
    std::stringstream str_stream(command_line_string);
    std::string       token;

    // Split up string based on whitespace
    while (str_stream >> token)
    {
        // Strip off any quotes surrounding the whole string
        if (token.front() == '\"')
        {
            token.erase(token.begin());
        }
        if (token.back() == '\"')
        {
            token.pop_back();
        }
        cmd_line_vector.push_back(token);
    }
    return true;
}

CmdLineRetType ToolSettings::ProcessCommandLine(int                       argc,
                                                const char**              argv,
                                                uint32_t                  expected_num_of_extra_args,
                                                std::vector<std::string>& extra_args)
{
    std::vector<std::string> command_line_args;
    if (!GenerateArgumentVector(argc, argv, command_line_args))
    {
        return CmdLineRetType_Error;
    }
    return ProcessCommandLine(command_line_args, expected_num_of_extra_args, extra_args);
}

CmdLineRetType ToolSettings::ProcessCommandLine(std::string               command_line_str,
                                                uint32_t                  expected_num_of_extra_args,
                                                std::vector<std::string>& extra_args)
{
    std::vector<std::string> command_line_args;
    if (!GenerateArgumentVector(command_line_str, command_line_args))
    {
        return CmdLineRetType_Error;
    }
    return ProcessCommandLine(command_line_args, expected_num_of_extra_args, extra_args);
}

CmdLineRetType ToolSettings::ProcessCommandLine(const std::vector<std::string>& command_line_args,
                                                uint32_t                        expected_num_of_extra_args,
                                                std::vector<std::string>&       extra_args)
{
    auto& settings_mgr = gfxrecon::util::settings::SettingsManager::GetSingleton();
    if (!settings_mgr.ProcessCommandLine(tool_type_, command_line_args, extra_args) ||
        extra_args.size() != expected_num_of_extra_args)
    {
        return CmdLineRetType_Error;
    }
    const auto settings_struct = settings_mgr.GetSettingsStruct();
    switch (tool_type_)
    {
        case gfxrecon::util::settings::kGfxrToolType_Replay_Tool:
            if (settings_struct->replay_settings.help)
            {
                return CmdLineRetType_PrintUsage;
            }
            if (settings_struct->replay_settings.version)
            {
                return CmdLineRetType_PrintVersion;
            }
            break;
        default:
            break;
    }

    // Update logging with values retrieved from command line arguments
    gfxrecon::util::Log::Settings log_settings;
    GetLogSettings(log_settings);
    gfxrecon::util::Log::UpdateWithSettings(log_settings);

    return CmdLineRetType_Success;
}

void ToolSettings::ProcessDisableDebugPopup()
{
#if defined(WIN32) && defined(_DEBUG)
    const auto settings_struct = gfxrecon::util::settings::SettingsManager::GetSingleton().GetSettingsStruct();
    switch (tool_type_)
    {
        case gfxrecon::util::settings::kGfxrToolType_Replay_Tool:
            if (settings_struct->replay_settings.no_debug_popup)
            {
                _set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);
            }
            break;
        default:
            break;
    }
#endif
}

bool ToolSettings::IsCaptureLayerEnabled()
{
    std::vector<std::string> enable_layers;
    std::vector<std::string> disable_layers;
    GetLayerListFromEnvVar(kOldLoaderLayerEnableEnvVar, enable_layers);

#if !defined(__ANDROID__)
    // Android loader does not support the new layer options
    GetLayerListFromEnvVar(kNewLoaderLayerEnableEnvVar, enable_layers);
    GetLayerListFromEnvVar(kLoaderLayerDisableEnvVar, disable_layers);

    // Remove any layers that were disabled
    for (auto& disabled_layer : disable_layers)
    {
        enable_layers.erase(std::remove(enable_layers.begin(), enable_layers.end(), disabled_layer),
                            enable_layers.end());
    }
#endif

    // Check remaining enabled layers for the capture layer
    if (!enable_layers.empty())
    {
        // Check for the presence of the layer name in the list of active layers.
        if (std::find(enable_layers.begin(), enable_layers.end(), kCaptureLayer) != enable_layers.end())
        {
            return true;
        }
    }
    return false;
}

bool ToolSettings::IsPreloadEnabled()
{
    const auto settings_struct = gfxrecon::util::settings::SettingsManager::GetSingleton().GetSettingsStruct();
    switch (tool_type_)
    {
        case gfxrecon::util::settings::kGfxrToolType_Replay_Tool:
            if (settings_struct->replay_settings.preload_measurement_range)
            {
                return true;
            }
            break;
        default:
            break;
    }
    return false;
}

uint32_t ToolSettings::GetPauseFrame()
{
    uint32_t   pause_frame     = 0;
    const auto settings_struct = gfxrecon::util::settings::SettingsManager::GetSingleton().GetSettingsStruct();
    switch (tool_type_)
    {
        case gfxrecon::util::settings::kGfxrToolType_Replay_Tool:
            if (settings_struct->replay_settings.paused)
            {
                pause_frame = 1;
            }
            else
            {
                pause_frame = settings_struct->replay_settings.pause_frame;
            }
            break;
        default:
            break;
    }
    return pause_frame;
}

WsiPlatform ToolSettings::GetWsiPlatform()
{
    WsiPlatform wsi_platform = WsiPlatform::kAuto;
    std::string wsi_argument;
    const auto  settings_struct = gfxrecon::util::settings::SettingsManager::GetSingleton().GetSettingsStruct();
    switch (tool_type_)
    {
        case gfxrecon::util::settings::kGfxrToolType_Replay_Tool:
            wsi_argument = settings_struct->replay_settings.wsi;
            break;
        default:
            break;
    }

    if (!wsi_argument.empty())
    {
        if (gfxrecon::util::platform::StringCompareNoCase(kWsiPlatformAuto, wsi_argument.c_str()) == 0)
        {
            wsi_platform = WsiPlatform::kAuto;
        }
        else if (gfxrecon::util::platform::StringCompareNoCase(kWsiPlatformWin32, wsi_argument.c_str()) == 0)
        {
#if defined(VK_USE_PLATFORM_WIN32_KHR)
            wsi_platform = WsiPlatform::kWin32;
#else
            GFXRECON_LOG_WARNING("Ignoring wsi option \"%s\", which is not enabled on this system",
                                 wsi_argument.c_str());
#endif
        }
        else if (gfxrecon::util::platform::StringCompareNoCase(kWsiPlatformXlib, wsi_argument.c_str()) == 0)
        {
#if defined(VK_USE_PLATFORM_XLIB_KHR)
            wsi_platform = WsiPlatform::kXlib;
#else
            GFXRECON_LOG_WARNING("Ignoring wsi option %s, which is not enabled on this system", wsi_argument.c_str());
#endif
        }
        else if (gfxrecon::util::platform::StringCompareNoCase(kWsiPlatformXcb, wsi_argument.c_str()) == 0)
        {
#if defined(VK_USE_PLATFORM_XCB_KHR)
            wsi_platform = WsiPlatform::kXcb;
#else
            GFXRECON_LOG_WARNING("Ignoring wsi option \"%s\", which is not enabled on this system",
                                 wsi_argument.c_str());
#endif
        }
        else if (gfxrecon::util::platform::StringCompareNoCase(kWsiPlatformWayland, wsi_argument.c_str()) == 0)
        {
#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
            wsi_platform = WsiPlatform::kWayland;
#else
            GFXRECON_LOG_WARNING("Ignoring wsi option \"%s\", which is not enabled on this system",
                                 wsi_argument.c_str());
#endif
        }
        else if (gfxrecon::util::platform::StringCompareNoCase(kWsiPlatformMetal, wsi_argument.c_str()) == 0)
        {
#if defined(VK_USE_PLATFORM_METAL_EXT)
            wsi_platform = WsiPlatform::kMetal;
#else
            GFXRECON_LOG_WARNING("Ignoring wsi option \"%s\", which is not enabled on this system",
                                 wsi_argument.c_str());
#endif
        }
        else if (gfxrecon::util::platform::StringCompareNoCase(kWsiPlatformDisplay, wsi_argument.c_str()) == 0)
        {
#if defined(VK_USE_PLATFORM_DISPLAY_KHR)
            wsi_platform = WsiPlatform::kDisplay;
#else
            GFXRECON_LOG_WARNING("Ignoring wsi option \"%s\", which is not enabled on this system",
                                 wsi_argument.c_str());
#endif
        }
        else if (gfxrecon::util::platform::StringCompareNoCase(kWsiPlatformHeadless, wsi_argument.c_str()) == 0)
        {
#if defined(VK_USE_PLATFORM_HEADLESS)
            wsi_platform = WsiPlatform::kHeadless;
#else
            GFXRECON_LOG_WARNING("Ignoring wsi option \"%s\", which is not enabled on this system",
                                 wsi_argument.c_str());
#endif
        }
        else
        {
            GFXRECON_LOG_WARNING("Ignoring unrecognized wsi option \"%s\"", wsi_argument.c_str());
        }
    }

    return wsi_platform;
}

std::string ToolSettings::GetFirstWsiExtensionName()
{
    WsiPlatform wsi_platform = GetWsiPlatform();
    return GetFirstWsiExtensionName(wsi_platform);
}

std::string ToolSettings::GetFirstWsiExtensionName(WsiPlatform wsi_platform)
{
    switch (wsi_platform)
    {
        // Return the first available WSI extension name
        case WsiPlatform::kAuto:
#if defined(VK_USE_PLATFORM_WIN32_KHR)
        case WsiPlatform::kWin32:
        {
            return VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
        }
#endif
#if defined(VK_USE_PLATFORM_XLIB_KHR)
        case WsiPlatform::kXlib:
        {
            return VK_KHR_XLIB_SURFACE_EXTENSION_NAME;
        }
#endif
#if defined(VK_USE_PLATFORM_XCB_KHR)
        case WsiPlatform::kXcb:
        {
            return VK_KHR_XCB_SURFACE_EXTENSION_NAME;
        }
#endif
#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
        case WsiPlatform::kWayland:
        {
            return VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME;
        }
#endif
#if defined(VK_USE_PLATFORM_METAL_EXT)
        case WsiPlatform::kMetal:
        {
            return VK_EXT_METAL_SURFACE_EXTENSION_NAME;
        }
#endif
#if defined(VK_USE_PLATFORM_HEADLESS)
        case WsiPlatform::kHeadless:
        {
            return VK_EXT_HEADLESS_SURFACE_EXTENSION_NAME;
        }
#endif
#if defined(VK_USE_PLATFORM_DISPLAY_KHR)
        case WsiPlatform::kDisplay:
        {
            return VK_KHR_DISPLAY_EXTENSION_NAME;
        }
#endif
        default:
        {
            GFXRECON_ASSERT(false && "Failed to get WSI extension name");
            return std::string();
        }
    }
}

std::string ToolSettings::GetWsiExtensionName()
{
    WsiPlatform wsi_platform = GetWsiPlatform();
    switch (wsi_platform)
    {
        case WsiPlatform::kAuto:
        {
            return std::string();
        }
        default:
        {
            return GetFirstWsiExtensionName(wsi_platform);
        }
    }
}

std::string ToolSettings::GetWsiArgString()
{
    std::string wsi_args = kWsiPlatformAuto;
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    wsi_args += ',';
    wsi_args += kWsiPlatformWin32;
#endif
#if defined(VK_USE_PLATFORM_XLIB_KHR)
    wsi_args += ',';
    wsi_args += kWsiPlatformXlib;
#endif
#if defined(VK_USE_PLATFORM_XCB_KHR)
    wsi_args += ',';
    wsi_args += kWsiPlatformXcb;
#endif
#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
    wsi_args += ',';
    wsi_args += kWsiPlatformWayland;
#endif
#if defined(VK_USE_PLATFORM_METAL_EXT)
    wsi_args += ',';
    wsi_args += kWsiPlatformMetal;
#endif
#if defined(VK_USE_PLATFORM_DISPLAY_KHR)
    wsi_args += ',';
    wsi_args += kWsiPlatformDisplay;
#endif
#if defined(VK_USE_PLATFORM_HEADLESS)
    wsi_args += ',';
    wsi_args += kWsiPlatformHeadless;
#endif
    return wsi_args;
}

void ToolSettings::GetMeasurementFilename(std::string& file_name)
{
    const auto settings_struct = gfxrecon::util::settings::SettingsManager::GetSingleton().GetSettingsStruct();
    switch (tool_type_)
    {
        case gfxrecon::util::settings::kGfxrToolType_Replay_Tool:
            file_name = settings_struct->replay_settings.measurement_file;
            break;
        default:
            break;
    }
    if (file_name.empty())
    {
#if defined(__ANDROID__)
        file_name = "/sdcard/gfxrecon-measurements.json";
#else
        file_name = "./gfxrecon-measurements.json";
#endif
    }
}

bool ToolSettings::GetMeasurementFrameRange(uint32_t& start_frame, uint32_t& end_frame)
{
    start_frame = 1;
    end_frame   = std::numeric_limits<uint32_t>::max();

    std::string value;
    const auto  settings_struct = gfxrecon::util::settings::SettingsManager::GetSingleton().GetSettingsStruct();
    switch (tool_type_)
    {
        case gfxrecon::util::settings::kGfxrToolType_Replay_Tool:
            value = settings_struct->replay_settings.measurement_frame_range;
            break;
        default:
            break;
    }

    if (!value.empty())
    {
        std::vector<std::string> values  = gfxrecon::util::strings::SplitString(value, '-');
        bool                     invalid = false;

        if (values.size() != 2)
        {
            GFXRECON_LOG_WARNING(
                "Ignoring invalid measurement frame range \"%s\". Must have format: <start_frame>-<end_frame>",
                value.c_str());
            invalid = true;
        }

        for (std::string& num : values)
        {
            gfxrecon::util::strings::RemoveWhitespace(num);

            // Check that the range string only contains numbers.
            const size_t count = std::count_if(num.begin(), num.end(), ::isdigit);
            if (count != num.length())
            {
                GFXRECON_LOG_WARNING(
                    "Ignoring invalid measurement frame range \"%s\", which contains non-numeric values",
                    value.c_str());
                invalid = true;
                break;
            }
        }

        if (!invalid)
        {
            uint32_t start_frame_arg = std::stoi(values[0]);
            uint32_t end_frame_arg   = std::stoi(values[1]);

            if (start_frame_arg >= end_frame_arg)
            {
                GFXRECON_LOG_WARNING("Ignoring invalid measurement frame range \"%s\", where first frame is "
                                     "greater than or equal to the last frame",
                                     value.c_str());

                return false;
            }

            start_frame = start_frame_arg;
            end_frame   = end_frame_arg;
            return true;
        }
    }

    return false;
}

// Modifies settings parameter with values set via command line
void ToolSettings::GetLogSettings(gfxrecon::util::Log::Settings& log_settings)
{
    gfxrecon::util::LoggingSeverity log_level;
    std::string                     log_level_str;

    const auto settings_struct = gfxrecon::util::settings::SettingsManager::GetSingleton().GetSettingsStruct();
    switch (tool_type_)
    {
        case gfxrecon::util::settings::kGfxrToolType_Replay_Tool:
            log_level_str                          = settings_struct->replay_settings.log_level;
            log_settings.output_timestamps         = settings_struct->replay_settings.log_timestamps;
            log_settings.file_name                 = settings_struct->replay_settings.log_file;
            log_settings.output_to_os_debug_string = settings_struct->replay_settings.log_debugview;
            break;
        default:
            break;
    }

    if (log_level_str.empty() || !gfxrecon::util::Log::StringToSeverity(log_level_str, log_level))
    {
        log_level = gfxrecon::decode::kDefaultLogLevel;
        if (!log_level_str.empty())
        {
            GFXRECON_LOG_WARNING("Ignoring unrecognized log level option value \"%s\"", log_level_str.c_str());
        }
    }
    log_settings.min_severity = log_level;
    if (log_settings.file_name.size() > 0)
    {
        log_settings.write_to_file = true;
    }
}

void ToolSettings::GetLayerListFromEnvVar(const char* env_var, std::vector<std::string>& layer_list)
{
    std::string env_var_value = gfxrecon::util::platform::GetEnv(env_var);

    uint32_t last_index = 0;
    for (uint32_t indx = 0; indx < env_var_value.size(); ++indx)
    {
        if ((indx - last_index > 1) && (env_var_value[indx] == ';' || env_var_value[indx] == ':'))
        {
            layer_list.push_back(env_var_value.substr(last_index, indx - last_index));
            last_index = indx;
        }
    }
}

gfxrecon::util::ScreenshotFormat ToolSettings::GetScreenshotFormat()
{
    gfxrecon::util::ScreenshotFormat format = gfxrecon::util::ScreenshotFormat::kBmp;
    std::string                      value;
    const auto settings_struct = gfxrecon::util::settings::SettingsManager::GetSingleton().GetSettingsStruct();
    switch (tool_type_)
    {
        case gfxrecon::util::settings::kGfxrToolType_Replay_Tool:
            value = settings_struct->replay_settings.screenshot_format;
            break;
        default:
            break;
    }

    if (!value.empty())
    {
        if (gfxrecon::util::platform::StringCompareNoCase(gfxrecon::util::kScreenshotFormatBmp, value.c_str()) == 0)
        {
            format = gfxrecon::util::ScreenshotFormat::kBmp;
        }
        else if (gfxrecon::util::platform::StringCompareNoCase(gfxrecon::util::kScreenshotFormatPng, value.c_str()) ==
                 0)
        {
            format = gfxrecon::util::ScreenshotFormat::kPng;
        }
        else
        {
            GFXRECON_LOG_WARNING("Ignoring unrecognized screenshot format option \"%s\"", value.c_str());
        }
    }

    return format;
}

std::string ToolSettings::GetScreenshotDir()
{
    std::string value;
    const auto  settings_struct = gfxrecon::util::settings::SettingsManager::GetSingleton().GetSettingsStruct();
    switch (tool_type_)
    {
        case gfxrecon::util::settings::kGfxrToolType_Replay_Tool:
            value = settings_struct->replay_settings.screenshot_dir;
            break;
        default:
            break;
    }
    if (!value.empty())
    {
        return value;
    }

    return kDefaultScreenshotDir;
}

void ToolSettings::GetScreenshotSize(uint32_t& width, uint32_t& height)
{
    std::string value;
    const auto  settings_struct = gfxrecon::util::settings::SettingsManager::GetSingleton().GetSettingsStruct();
    switch (tool_type_)
    {
        case gfxrecon::util::settings::kGfxrToolType_Replay_Tool:
            value = settings_struct->replay_settings.screenshot_size;
            break;
        default:
            break;
    }

    if (!value.empty())
    {
        std::size_t x = value.find("x");
        if (x != std::string::npos)
        {
            try
            {
                width  = std::stoul(value.substr(0, x));
                height = std::stoul(value.substr(x + 1));
            }
            catch (std::exception&)
            {
                GFXRECON_LOG_WARNING("Ignoring invalid screenshot width x height option. Expected format is "
                                     "--screenshot-size [width]x[height]");
                width = height = 0;
            }
        }
        else
        {
            width = height = 0;
        }
    }
    else
    {
        width = height = 0;
    }
}

float ToolSettings::GetScreenshotScale()
{
    const auto settings_struct = gfxrecon::util::settings::SettingsManager::GetSingleton().GetSettingsStruct();
    switch (tool_type_)
    {
        case gfxrecon::util::settings::kGfxrToolType_Replay_Tool:
            return settings_struct->replay_settings.screenshot_scale;
        default:
            return 0.0f;
    }
}

std::vector<gfxrecon::decode::ScreenshotRange> ToolSettings::GetScreenshotRanges()
{
    bool                                           screenshot_all = false;
    std::string                                    screenshot_frames;
    std::vector<gfxrecon::decode::ScreenshotRange> ranges;

    const auto settings_struct = gfxrecon::util::settings::SettingsManager::GetSingleton().GetSettingsStruct();
    switch (tool_type_)
    {
        case gfxrecon::util::settings::kGfxrToolType_Replay_Tool:
            screenshot_all    = settings_struct->replay_settings.screenshot_all;
            screenshot_frames = settings_struct->replay_settings.screenshots;
            break;
        default:
            break;
    }

    if (screenshot_all)
    {
        gfxrecon::decode::ScreenshotRange screenshot_range;
        screenshot_range.first = 1;
        screenshot_range.last  = std::numeric_limits<uint32_t>::max();
        ranges.emplace_back(std::move(screenshot_range));
    }
    else if (!screenshot_frames.empty())
    {
        std::vector<gfxrecon::util::UintRange> frame_ranges =
            gfxrecon::util::GetUintRanges(screenshot_frames.c_str(), "screenshot frames");

        for (uint32_t i = 0; i < frame_ranges.size(); ++i)
        {
            gfxrecon::decode::ScreenshotRange range{};
            range.first = frame_ranges[i].first;
            range.last  = frame_ranges[i].last;
            ranges.push_back(range);
        }
    }

    return ranges;
}

static gfxrecon::decode::VulkanResourceAllocator* CreateDefaultAllocator()
{
    return new gfxrecon::decode::VulkanDefaultAllocator(
        "Try replay with the '-m remap' or '-m rebind' options to enable memory translation.");
}

static gfxrecon::decode::VulkanResourceAllocator* CreateRemapAllocator()
{
    return new gfxrecon::decode::VulkanRemapAllocator(
        "Try replay with the '-m rebind' option to enable advanced memory translation.");
}

static gfxrecon::decode::VulkanResourceAllocator* CreateRebindAllocator()
{
    return new gfxrecon::decode::VulkanRebindAllocator();
}

static gfxrecon::decode::CreateResourceAllocator
InitRealignAllocatorCreateFunc(const std::string&                              filename,
                               const gfxrecon::decode::VulkanReplayOptions&    replay_options,
                               gfxrecon::decode::VulkanTrackedObjectInfoTable* tracked_object_info_table)
{
    // Enable first pass of replay to generate resource tracking information.
    GFXRECON_WRITE_CONSOLE("First pass of replay resource tracking for realign memory portability mode. This may take "
                           "some time. Please wait...");

    gfxrecon::decode::FileProcessor file_processor_resource_tracking;
    gfxrecon::decode::VulkanDecoder decoder;

    auto resource_tracking_consumer =
        new gfxrecon::decode::VulkanResourceTrackingConsumer(replay_options, tracked_object_info_table);

    if (file_processor_resource_tracking.Initialize(filename))
    {
        decoder.AddConsumer(resource_tracking_consumer);
        file_processor_resource_tracking.AddDecoder(&decoder);
        file_processor_resource_tracking.ProcessAllFrames();
        file_processor_resource_tracking.RemoveDecoder(&decoder);
        decoder.RemoveConsumer(resource_tracking_consumer);
    }

    // Sort the bound resources according to the binding offsets.
    resource_tracking_consumer->SortMemoriesBoundResourcesByOffset();

    // calculate the replay binding offset of the bound resources and replay memory allocation size
    resource_tracking_consumer->CalculateReplayBindingOffsetAndMemoryAllocationSize();

    GFXRECON_WRITE_CONSOLE("First pass of replay resource tracking done.");

    return [tracked_object_info_table]() -> gfxrecon::decode::VulkanResourceAllocator* {
        return new gfxrecon::decode::VulkanRealignAllocator(
            tracked_object_info_table, "Try replay with the '-m rebind' option to enable advanced memory translation.");
    };
}

gfxrecon::decode::CreateResourceAllocator
ToolSettings::GetCreateResourceAllocatorFunc(const std::string&                              filename,
                                             const gfxrecon::decode::VulkanReplayOptions&    replay_options,
                                             gfxrecon::decode::VulkanTrackedObjectInfoTable* tracked_object_info_table)
{
    gfxrecon::decode::CreateResourceAllocator func = CreateDefaultAllocator;

    std::string value;
    const auto  settings_struct = gfxrecon::util::settings::SettingsManager::GetSingleton().GetSettingsStruct();
    switch (tool_type_)
    {
        case gfxrecon::util::settings::kGfxrToolType_Replay_Tool:
            value = settings_struct->replay_settings.memory_translation;
            break;
        default:
            break;
    }

    if (!value.empty())
    {
        if (gfxrecon::util::platform::StringCompareNoCase(kMemoryTranslationRebind, value.c_str()) == 0)
        {
            func = CreateRebindAllocator;
        }
        else if (gfxrecon::util::platform::StringCompareNoCase(kMemoryTranslationRemap, value.c_str()) == 0)
        {
            func = CreateRemapAllocator;
        }
        else if (gfxrecon::util::platform::StringCompareNoCase(kMemoryTranslationRealign, value.c_str()) == 0)
        {
            func = InitRealignAllocatorCreateFunc(filename, replay_options, tracked_object_info_table);
        }
        else if (gfxrecon::util::platform::StringCompareNoCase(kMemoryTranslationNone, value.c_str()) != 0)
        {
            GFXRECON_LOG_WARNING("Ignoring unrecognized memory translation option \"%s\"", value.c_str());
        }
    }

    return func;
}

void ToolSettings::CheckForceWindowed(gfxrecon::decode::ReplayOptions& options)
{
#if defined(WIN32)
    std::string force_windowed;
    const auto  settings_struct = gfxrecon::util::settings::SettingsManager::GetSingleton().GetSettingsStruct();
    switch (tool_type_)
    {
        case gfxrecon::util::settings::kGfxrToolType_Replay_Tool:
            force_windowed = settings_struct->replay_settings.force_windowed;
            break;
        default:
            break;
    }

    if (!force_windowed.empty())
    {
        options.force_windowed = true;

        std::istringstream value_input;
        value_input.str(force_windowed);
        std::string val;

        std::getline(value_input, val, ',');
        options.windowed_width = std::stoi(val);
        std::getline(value_input, val, ',');
        options.windowed_height = std::stoi(val);
    }
#endif // WIN32
}

void ToolSettings::CheckWindowOrigin(gfxrecon::decode::ReplayOptions& options)
{
#if defined(WIN32)
    std::string force_windowed;
    const auto  settings_struct = gfxrecon::util::settings::SettingsManager::GetSingleton().GetSettingsStruct();
    switch (tool_type_)
    {
        case gfxrecon::util::settings::kGfxrToolType_Replay_Tool:
            force_windowed = settings_struct->replay_settings.force_windowed_origin;
            break;
        default:
            break;
    }

    if (!force_windowed.empty())
    {
        options.force_windowed_origin = true;

        std::istringstream value_input;
        value_input.str(force_windowed);
        std::string val;

        std::getline(value_input, val, ',');
        options.window_topleft_x = std::stoi(val);
        std::getline(value_input, val, ',');
        options.window_topleft_y = std::stoi(val);
    }
#endif // WIN32
}

std::string ToolSettings::GetDumpResourcesDir()
{
    std::string value;
    const auto  settings_struct = gfxrecon::util::settings::SettingsManager::GetSingleton().GetSettingsStruct();
    switch (tool_type_)
    {
        case gfxrecon::util::settings::kGfxrToolType_Replay_Tool:
            value = settings_struct->replay_settings.dump_resources_dir;
            break;
        default:
            break;
    }

    if (!value.empty())
    {
        return value;
    }

    return kDefaultDumpResourcesDir;
}

gfxrecon::util::ScreenshotFormat ToolSettings::GetDumpresourcesImageFormat()
{
    std::string value;
    const auto  settings_struct = gfxrecon::util::settings::SettingsManager::GetSingleton().GetSettingsStruct();
    switch (tool_type_)
    {
        case gfxrecon::util::settings::kGfxrToolType_Replay_Tool:
            value = settings_struct->replay_settings.dump_resources_image_format;
            break;
        default:
            break;
    }

    gfxrecon::util::ScreenshotFormat format = gfxrecon::util::ScreenshotFormat::kBmp;
    if (!value.empty())
    {
        if (gfxrecon::util::platform::StringCompareNoCase(gfxrecon::util::kScreenshotFormatBmp, value.c_str()) == 0)
        {
            format = gfxrecon::util::ScreenshotFormat::kBmp;
        }
        else if (gfxrecon::util::platform::StringCompareNoCase(gfxrecon::util::kScreenshotFormatPng, value.c_str()) ==
                 0)
        {
            format = gfxrecon::util::ScreenshotFormat::kPng;
        }
        else
        {
            GFXRECON_LOG_WARNING("Ignoring unrecognized dump resources image format option \"%s\"", value.c_str());
        }
    }

    return format;
}

gfxrecon::format::CompressionType ToolSettings::GetDumpResourcesCompressionType()
{
    std::string value;
    const auto  settings_struct = gfxrecon::util::settings::SettingsManager::GetSingleton().GetSettingsStruct();
    switch (tool_type_)
    {
        case gfxrecon::util::settings::kGfxrToolType_Replay_Tool:
            value = settings_struct->replay_settings.dump_resources_binary_file_compression_type;
            break;
        default:
            break;
    }

    gfxrecon::format::CompressionType type = gfxrecon::format::CompressionType::kNone;
    if (!value.empty())
    {
        if (gfxrecon::util::platform::StringCompareNoCase(gfxrecon::util::kCompressionTypeNone, value.c_str()) == 0)
        {
            type = gfxrecon::format::CompressionType::kNone;
        }
        else if (gfxrecon::util::platform::StringCompareNoCase(gfxrecon::util::kCompressionTypeLz4, value.c_str()) == 0)
        {
            type = gfxrecon::format::CompressionType::kLz4;
        }
        else if (gfxrecon::util::platform::StringCompareNoCase(gfxrecon::util::kCompressionTypeZlib, value.c_str()) ==
                 0)
        {
            type = gfxrecon::format::CompressionType::kZlib;
        }
        else if (gfxrecon::util::platform::StringCompareNoCase(gfxrecon::util::kCompressionTypeZstd, value.c_str()) ==
                 0)
        {
            type = gfxrecon::format::CompressionType::kZstd;
        }
        else
        {
            GFXRECON_LOG_ERROR("Unrecognized compression method \"%s\"", value.c_str());
        }
    }

    return type;
}

float ToolSettings::GetDumpResourcesScale()
{
    float      scale           = 1.0f;
    const auto settings_struct = gfxrecon::util::settings::SettingsManager::GetSingleton().GetSettingsStruct();
    switch (tool_type_)
    {
        case gfxrecon::util::settings::kGfxrToolType_Replay_Tool:
            scale = settings_struct->replay_settings.dump_resources_scale;
            break;
        default:
            break;
    }

    if (scale <= 0.0f)
    {
        GFXRECON_LOG_WARNING("Ignoring invalid dump resources scale option. Value must > 0.0.");
        scale = 1.0f;
    }
    if (scale >= 10.0f)
    {
        scale = 10.0f;
    }
    return scale;
}

bool ToolSettings::GetQuitAfterFrame(uint32_t& quit_frame)
{
    std::string value;
    const auto  settings_struct = gfxrecon::util::settings::SettingsManager::GetSingleton().GetSettingsStruct();
    switch (tool_type_)
    {
        case gfxrecon::util::settings::kGfxrToolType_Replay_Tool:
            value = settings_struct->replay_settings.quit_after_frame;
            break;
        default:
            break;
    }

    if (!value.empty())
    {
        if (std::count_if(value.begin(), value.end(), ::isdigit) != value.length())
        {
            GFXRECON_LOG_WARNING("Ignoring invalid quit after frame \"%s\", which contains non-numeric values",
                                 value.c_str());
            return false;
        }

        quit_frame = std::stoi(value);
        return true;
    }

    return false;
}
void ToolSettings::GetReplayOptions(gfxrecon::decode::ReplayOptions& options, const std::string& filename)
{
    options.capture_filename = filename;

    std::string frame;
    bool        print_block_all = false;
    std::string print_block_args;
    std::string override_gpu;

    const auto settings_struct = gfxrecon::util::settings::SettingsManager::GetSingleton().GetSettingsStruct();
    switch (tool_type_)
    {
        case gfxrecon::util::settings::kGfxrToolType_Replay_Tool:
            options.create_dummy_allocations = settings_struct->replay_settings.sync;
            options.cpu_mask                 = settings_struct->replay_settings.cpu_mask;
            options.enable_debug_device_lost = settings_struct->replay_settings.debug_device_lost;
            options.enable_validation_layer  = settings_struct->replay_settings.validate;
#if defined(WIN32)
            options.enable_debug_device_lost = settings_struct->replay_settings.create_dummy_allocations;
#endif
            options.flush_measurement_frame_range  = settings_struct->replay_settings.flush_measurement_range;
            options.flush_inside_measurement_range = settings_struct->replay_settings.flush_inside_measurement_range;
            options.num_pipeline_creation_jobs     = settings_struct->replay_settings.pipeline_creation_jobs;
#if defined(__ANDROID__)
            options.omit_null_hardware_buffers = settings_struct->replay_settings.omit_null_hardware_buffers;
#endif
            options.quit_after_measurement_frame_range = settings_struct->replay_settings.quit_after_measurement_range;

            frame            = settings_struct->replay_settings.quit_after_frame;
            override_gpu     = settings_struct->replay_settings.gpu;
            print_block_all  = settings_struct->replay_settings.pbi_all;
            print_block_args = settings_struct->replay_settings.pbis;
            break;
        default:
            break;
    }

    if (!frame.empty())
    {
        options.quit_after_frame = true;
    }

    if (print_block_all)
    {
        options.enable_print_block_info = true;
    }
    else if (!print_block_args.empty())
    {
        options.enable_print_block_info = true;
        std::vector<gfxrecon::util::UintRange> block_ranges =
            gfxrecon::util::GetUintRanges(print_block_args.c_str(), "Print block information");
        options.block_index_from = block_ranges[0].first;
        options.block_index_to   = block_ranges[1].first;
    }

    if (!options.cpu_mask.empty())
    {
        if (gfxrecon::util::platform::SetCpuAffinity(options.cpu_mask))
        {
            GFXRECON_LOG_INFO("CPU mask successfully set: %s", gfxrecon::util::platform::GetCpuAffinity().c_str());
        }
        else
        {
            GFXRECON_LOG_ERROR("Failed to set CPU mask: %s", options.cpu_mask.c_str());
            GFXRECON_LOG_ERROR("Resuming with CPU mask: %s", gfxrecon::util::platform::GetCpuAffinity().c_str());
        }
    }

    if (!override_gpu.empty())
    {
        options.override_gpu_index = std::stoi(override_gpu);
    }

    CheckForceWindowed(options);
    CheckWindowOrigin(options);
}

gfxrecon::decode::VulkanReplayOptions
ToolSettings::GetVulkanReplayOptions(const std::string&                              filename,
                                     gfxrecon::decode::VulkanTrackedObjectInfoTable* tracked_object_info_table)
{
    gfxrecon::decode::VulkanReplayOptions replay_options;
    GetReplayOptions(replay_options, filename);

    bool        enable_captured_swapchain = false;
    std::string gpu_override_group;
    std::string present_mode_option;
    std::string swapchain;
    std::string debug_severity_string;
    int         skip_get_fence_status = 0;
    std::string skip_get_fence_ranges;
    std::string dump_resources;

    const auto settings_struct = gfxrecon::util::settings::SettingsManager::GetSingleton().GetSettingsStruct();
    switch (tool_type_)
    {
        case gfxrecon::util::settings::kGfxrToolType_Replay_Tool:
            replay_options.add_new_pipeline_caches = settings_struct->replay_settings.add_new_pipeline_caches;
            replay_options.capture                 = settings_struct->replay_settings.capture;
            replay_options.do_device_deduplication = settings_struct->replay_settings.deduplicate_device;
            replay_options.dump_resources_before   = settings_struct->replay_settings.dump_resources_before_draw;
            replay_options.dump_resources_color_attachment_index =
                settings_struct->replay_settings.dump_resources_dump_color_attachment_index;
            replay_options.dump_resources_dump_all_image_subresources =
                settings_struct->replay_settings.dump_resources_dump_all_image_subresources;
            replay_options.dump_resources_dump_depth =
                settings_struct->replay_settings.dump_resources_dump_depth_attachment;
            replay_options.dump_resources_dump_raw_images =
                settings_struct->replay_settings.dump_resources_dump_raw_images;
            replay_options.dump_resources_dump_separate_alpha =
                settings_struct->replay_settings.dump_resources_dump_separate_alpha;
            replay_options.dump_resources_dump_unused_vertex_bindings =
                settings_struct->replay_settings.dump_resources_dump_unused_vertex_bindings;
            replay_options.dump_resources_dump_vertex_index_buffer =
                settings_struct->replay_settings.dump_resources_dump_vertex_index_buffers;
            replay_options.dump_resources_json_per_command =
                settings_struct->replay_settings.dump_resources_json_output_per_command;
            replay_options.dump_resources_dump_build_AS_input_buffers =
                settings_struct->replay_settings.dump_resources_dump_build_acceleration_structures_input_buffers;
            replay_options.flush_measurement_frame_range = settings_struct->replay_settings.flush_measurement_range;
            replay_options.load_pipeline_cache_filename  = settings_struct->replay_settings.load_pipeline_cache;
            replay_options.offscreen_swapchain_frame_boundary =
                settings_struct->replay_settings.offscreen_swapchain_frame_boundary;
            replay_options.omit_pipeline_cache_data  = settings_struct->replay_settings.omit_pipeline_cache_data;
            replay_options.preload_measurement_range = settings_struct->replay_settings.preload_measurement_range;
            replay_options.quit_after_measurement_frame_range =
                settings_struct->replay_settings.quit_after_measurement_range;
            replay_options.remove_unsupported_features  = settings_struct->replay_settings.remove_unsupported;
            replay_options.replace_shader_dir           = settings_struct->replay_settings.replace_shaders;
            replay_options.save_pipeline_cache_filename = settings_struct->replay_settings.save_pipeline_cache;
            replay_options.screenshot_interval =
                static_cast<uint32_t>(settings_struct->replay_settings.screenshot_interval);
            replay_options.screenshot_file_prefix = settings_struct->replay_settings.screenshot_prefix;
#if defined(__ANDROID__)
            replay_options.screenshot_ignore_frameBoundaryAndroid =
                settings_struct->replay_settings.screenshot_ignore_FrameBoundaryANDROID;
#endif
            replay_options.screenshot_scale            = settings_struct->replay_settings.screenshot_scale;
            replay_options.surface_index               = settings_struct->replay_settings.surface_index;
            replay_options.skip_failed_allocations     = settings_struct->replay_settings.skip_failed_allocations;
            replay_options.use_colorspace_fallback     = settings_struct->replay_settings.use_colorspace_fallback;
            replay_options.virtual_swapchain_skip_blit = settings_struct->replay_settings.virtual_swapchain_skip_blit;
            replay_options.wait_before_present         = settings_struct->replay_settings.wait_before_present;

            debug_severity_string     = settings_struct->replay_settings.debug_messenger_level;
            dump_resources            = settings_struct->replay_settings.dump_resources;
            enable_captured_swapchain = settings_struct->replay_settings.use_captured_swapchain_indices;
            gpu_override_group        = settings_struct->replay_settings.gpu_group;
            present_mode_option       = settings_struct->replay_settings.present_mode;
            skip_get_fence_ranges     = settings_struct->replay_settings.skip_get_fence_ranges;
            skip_get_fence_status     = settings_struct->replay_settings.skip_get_fence_status;
            swapchain                 = settings_struct->replay_settings.swapchain;
            break;
        default:
            break;
    }

    if (!gpu_override_group.empty())
    {
        replay_options.override_gpu_group_index = std::stoi(gpu_override_group);
    }

    if (swapchain.empty())
    {
        if (enable_captured_swapchain)
        {
            replay_options.swapchain_option = gfxrecon::util::SwapchainOption::kCaptured;
        }
    }
    else
    {
        if (enable_captured_swapchain)
        {
            GFXRECON_LOG_WARNING("Ignoring option: \"%s\" because option: \"%s\" is added",
                                 kEnableUseCapturedSwapchainIndices,
                                 kSwapchainOption);
        }

        if (gfxrecon::util::platform::StringCompareNoCase(kSwapchainCaptured, swapchain.c_str()) == 0)
        {
            replay_options.swapchain_option = gfxrecon::util::SwapchainOption::kCaptured;
        }
        else if (gfxrecon::util::platform::StringCompareNoCase(kSwapchainOffscreen, swapchain.c_str()) == 0)
        {
            replay_options.swapchain_option = gfxrecon::util::SwapchainOption::kOffscreen;
        }
        else if (gfxrecon::util::platform::StringCompareNoCase(kSwapchainVirtual, swapchain.c_str()) != 0)
        {
            GFXRECON_LOG_WARNING("Ignoring unrecognized \"--swapchain\" option: %s", swapchain.c_str());
        }
    }

    if (gfxrecon::util::platform::StringCompareNoCase(kPresentModeCapture, present_mode_option.c_str()) == 0)
    {
        replay_options.present_mode_option = gfxrecon::util::PresentModeOption::kCapture;
    }
    else if (gfxrecon::util::platform::StringCompareNoCase(kPresentModeImmediate, present_mode_option.c_str()) == 0)
    {
        replay_options.present_mode_option = gfxrecon::util::PresentModeOption::kImmediate;
    }
    else if (gfxrecon::util::platform::StringCompareNoCase(kPresentModeMailbox, present_mode_option.c_str()) == 0)
    {
        replay_options.present_mode_option = gfxrecon::util::PresentModeOption::kMailbox;
    }
    else if (gfxrecon::util::platform::StringCompareNoCase(kPresentModeFifo, present_mode_option.c_str()) == 0)
    {
        replay_options.present_mode_option = gfxrecon::util::PresentModeOption::kFifo;
    }
    else if (gfxrecon::util::platform::StringCompareNoCase(kPresentModeFifoRelaxed, present_mode_option.c_str()) == 0)
    {
        replay_options.present_mode_option = gfxrecon::util::PresentModeOption::kFifoRelaxed;
    }
    else if (!present_mode_option.empty())
    {
        GFXRECON_LOG_WARNING("Ignoring unrecognized \"--present-mode\" option: %s", present_mode_option.c_str());
    }

    if (!debug_severity_string.empty())
    {
        if (!gfxrecon::util::platform::StringCompareNoCase("debug", debug_severity_string.c_str()))
        {
            replay_options.debug_message_severity =
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        }
        else if (!gfxrecon::util::platform::StringCompareNoCase("info", debug_severity_string.c_str()))
        {
            replay_options.debug_message_severity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
                                                    VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                                    VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        }
        else if (!gfxrecon::util::platform::StringCompareNoCase("warning", debug_severity_string.c_str()))
        {
            replay_options.debug_message_severity =
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        }
        else if (!gfxrecon::util::platform::StringCompareNoCase("error", debug_severity_string.c_str()))
        {
            replay_options.debug_message_severity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        }
        else
        {
            GFXRECON_LOG_WARNING("Ignoring unrecognized debug messenger severity option value \"%s\"",
                                 debug_severity_string.c_str());
        }
    }

    replay_options.create_resource_allocator =
        GetCreateResourceAllocatorFunc(filename, replay_options, tracked_object_info_table);

    if (replay_options.screenshot_interval == 0)
    {
        GFXRECON_LOG_WARNING("A screenshot interval of 0 is invalid. Using default value of 1.");
        replay_options.screenshot_interval = 1;
    }

    replay_options.screenshot_format = GetScreenshotFormat();
    replay_options.screenshot_dir    = GetScreenshotDir();
    replay_options.screenshot_ranges = GetScreenshotRanges();
    GetScreenshotSize(replay_options.screenshot_width, replay_options.screenshot_height);

    if (skip_get_fence_status < static_cast<int>(gfxrecon::decode::SkipGetFenceStatus::COUNT))
    {
        replay_options.skip_get_fence_status = static_cast<gfxrecon::decode::SkipGetFenceStatus>(skip_get_fence_status);
    }
    else
    {
        GFXRECON_LOG_FATAL("Unexpected value after '--skip-get-fence-status' : '%d'. Closing the program.",
                           skip_get_fence_status);
        abort();
    }

    if (skip_get_fence_ranges.empty())
    {
        gfxrecon::util::UintRange range;
        range.first = 1;
        range.last  = std::numeric_limits<uint32_t>::max();
        replay_options.skip_get_fence_ranges.push_back(range);
    }
    else
    {
        replay_options.skip_get_fence_ranges =
            gfxrecon::util::GetUintRanges(skip_get_fence_ranges.c_str(), "skip-get-fence-ranges");
    }

    if (!dump_resources.empty() && dump_resources.find_first_not_of("0123456789,") == std::string::npos)
    {
        replay_options.enable_dump_resources = true;
        if (dump_resources.find_first_not_of("0123456789,") == std::string::npos)
        {
            std::vector<std::string> values = gfxrecon::util::strings::SplitString(dump_resources, ',');
            if (values.size() == 3)
            {
                replay_options.dump_resources_target.submit_index    = std::stoi(values[0]);
                replay_options.dump_resources_target.command_index   = std::stoi(values[1]);
                replay_options.dump_resources_target.draw_call_index = std::stoi(values[2]);
                replay_options.using_dump_resources_target           = true;
            }
        }
        else
        {
            replay_options.dump_resources_block_indices = dump_resources;
        }
    }

    replay_options.dump_resources_image_format = GetDumpresourcesImageFormat();
    replay_options.dump_resources_scale        = GetDumpResourcesScale();
    replay_options.dump_resources_output_dir   = GetDumpResourcesDir();
    replay_options.dumping_resources           = !replay_options.dump_resources_block_indices.empty();

    replay_options.dump_resources_binary_file_compression_type = GetDumpResourcesCompressionType();

    return replay_options;
}

#if defined(D3D12_SUPPORT)

std::vector<int32_t> ToolSettings::GetFilteredMsgs(const std::string& messages)
{
    std::vector<int32_t> msgs;
    if (!messages.empty())
    {
        std::vector<std::string> values;
        std::istringstream       value_input;
        value_input.str(messages);

        for (std::string val; std::getline(value_input, val, ',');)
        {
            size_t count = std::count_if(val.begin(), val.end(), ::isdigit);
            if (count == val.length())
            {
                msgs.push_back(std::stoi(val));
            }
            else
            {
                GFXRECON_LOG_WARNING("Ignoring invalid filter messages\"%s\", which contains non-numeric values",
                                     val.c_str());
                break;
            }
        }
    }
    return msgs;
}
gfxrecon::decode::DxReplayOptions ToolSettings::GetDxReplayOptions(const std::string& filename)
{
    gfxrecon::decode::DxReplayOptions replay_options;
    GetReplayOptions(replay_options, filename);

    bool        ags_inj_marker_enabled = false;
    std::string dump_resources;
    std::string memory_usage;
    std::string messages;
    bool        psos_set = false;

    const auto settings_struct = gfxrecon::util::settings::SettingsManager::GetSingleton().GetSettingsStruct();
    switch (tool_type_)
    {
        case gfxrecon::util::settings::kGfxrToolType_Replay_Tool:
            replay_options.dump_resources_before = settings_struct->replay_settings.dump_resources_before_draw;
            replay_options.dump_resources_modifiable_state_only =
                settings_struct->replay_settings.dump_resources_modifiable_state_only;
            replay_options.enable_d3d12_two_pass_replay = settings_struct->replay_settings.dx12_two_pass_replay;
            replay_options.override_object_names        = settings_struct->replay_settings.dx12_override_object_names;
            replay_options.screenshot_interval =
                static_cast<uint32_t>(settings_struct->replay_settings.screenshot_interval);
            replay_options.screenshot_file_prefix = settings_struct->replay_settings.screenshot_prefix;
            replay_options.screenshot_scale       = settings_struct->replay_settings.screenshot_scale;
            replay_options.use_cached_psos        = settings_struct->replay_settings.use_cached_psos;

            ags_inj_marker_enabled             = settings_struct->replay_settings.discard_cached_psos;
            dump_resources                     = settings_struct->replay_settings.dump_resources;
            messages                           = settings_struct->replay_settings.denied_messages;
            replay_options.DeniedDebugMessages = GetFilteredMsgs(messages);
            messages.clear();
            messages                            = settings_struct->replay_settings.allowed_messages;
            replay_options.AllowedDebugMessages = GetFilteredMsgs(messages);
            psos_set                            = settings_struct->replay_settings.discard_cached_psos;
            break;
        default:
            break;
    }

    if (psos_set)
    {
        GFXRECON_LOG_WARNING("The parameters --dcp and --discard-cached-psos have been deprecated in favor for "
                             "--use-cached-psos");
    }

    if (ags_inj_marker_enabled)
    {
#ifdef GFXRECON_AGS_SUPPORT
        replay_options.ags_inject_markers = true;
#else
        GFXRECON_LOG_ERROR("Unsupported option --dx12-ags-inject-markers");
#endif
    }

    if (!dump_resources.empty() && dump_resources.find_first_not_of("0123456789,") == std::string::npos)
    {
        std::vector<std::string> values = gfxrecon::util::strings::SplitString(dump_resources, ',');
        if (values.size() == 3)
        {
            replay_options.dump_resources_target.submit_index    = std::stoi(values[0]);
            replay_options.dump_resources_target.command_index   = std::stoi(values[1]);
            replay_options.dump_resources_target.draw_call_index = std::stoi(values[2]);
            replay_options.enable_dump_resources                 = true;
            replay_options.using_dump_resources_target           = true;
        }
    }

    replay_options.dump_resources_output_dir = GetDumpResourcesDir();

    if (!memory_usage.empty())
    {
        int memory_usage_int = std::stoi(memory_usage);
        if (memory_usage_int >= 0 && memory_usage_int <= 100)
        {
            replay_options.memory_usage = static_cast<uint32_t>(memory_usage_int);
        }
        else
        {
            GFXRECON_LOG_WARNING(
                "The parameter to --batching-memory-usage is out of range [0, 100], will use 80 as default value.");
        }
    }

    if (replay_options.screenshot_interval == 0)
    {
        GFXRECON_LOG_WARNING("A screenshot interval of 0 is invalid. Using default value of 1.");
        replay_options.screenshot_interval = 1;
    }

    replay_options.screenshot_format = GetScreenshotFormat();
    replay_options.screenshot_dir    = GetScreenshotDir();
    replay_options.screenshot_ranges = GetScreenshotRanges();
    GetScreenshotSize(replay_options.screenshot_width, replay_options.screenshot_height);

    return replay_options;
}
#endif

GFXRECON_END_NAMESPACE(tools)
GFXRECON_END_NAMESPACE(gfxrecon)
