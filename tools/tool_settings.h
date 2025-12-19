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

#ifndef GFXRECON_PLATFORM_SETTINGS_H
#define GFXRECON_PLATFORM_SETTINGS_H

#include PROJECT_VERSION_HEADER_FILE

#if defined(D3D12_SUPPORT)
#include "decode/dx_replay_options.h"
#include <initguid.h>
#include "generated/generated_dx12_decoder.h"
#endif
#include "decode/file_processor.h"

#include "decode/vulkan_default_allocator.h"
#include "decode/vulkan_realign_allocator.h"
#include "decode/vulkan_rebind_allocator.h"
#include "decode/vulkan_remap_allocator.h"
#include "decode/vulkan_replay_options.h"
#include "decode/vulkan_resource_tracking_consumer.h"
#include "decode/vulkan_tracked_object_info_table.h"
#include "generated/generated_vulkan_decoder.h"
#include "format/format.h"

#if ENABLE_OPENXR_SUPPORT
#include "generated/generated_openxr_decoder.h"
#endif

#include "util/argument_parser.h"
#include "util/logging.h"
#include "util/platform.h"
#include "util/options.h"
#include "util/strings.h"
#include "util/settings_manager.h"

#if ENABLE_OPENXR_SUPPORT
#include "openxr/openxr.h"
#endif

#include "vulkan/vulkan_core.h"

#include <cstdlib>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

const char kApplicationName[] = "GFXReconstruct Replay";
const char kCaptureLayer[]    = "VK_LAYER_LUNARG_gfxreconstruct";

const char kHelpShortOption[]                    = "-h";
const char kHelpLongOption[]                     = "--help";
const char kVersionOption[]                      = "--version";
const char kLogLevelArgument[]                   = "--log-level";
const char kLogTimestampsOption[]                = "--log-timestamps";
const char kDebugMessageSeverityArgument[]       = "--debug-messenger-level";
const char kLogFileArgument[]                    = "--log-file";
const char kLogDebugView[]                       = "--log-debugview";
const char kNoDebugPopup[]                       = "--no-debug-popup";
const char kCpuMaskArgument[]                    = "--cpu-mask";
const char kOverrideGpuArgument[]                = "--gpu";
const char kOverrideGpuGroupArgument[]           = "--gpu-group";
const char kPausedOption[]                       = "--paused";
const char kPauseFrameArgument[]                 = "--pause-frame";
const char kCaptureOption[]                      = "--capture";
const char kSkipFailedAllocationShortOption[]    = "--sfa";
const char kSkipFailedAllocationLongOption[]     = "--skip-failed-allocations";
const char kDiscardCachedPsosShortOption[]       = "--dcp";
const char kDiscardCachedPsosLongOption[]        = "--discard-cached-psos";
const char kUseCachedPsosOption[]                = "--use-cached-psos";
const char kOmitPipelineCacheDataShortOption[]   = "--opcd";
const char kOmitPipelineCacheDataLongOption[]    = "--omit-pipeline-cache-data";
const char kWsiArgument[]                        = "--wsi";
const char kSurfaceIndexArgument[]               = "--surface-index";
const char kMemoryPortabilityShortOption[]       = "-m";
const char kMemoryPortabilityLongOption[]        = "--memory-translation";
const char kSyncOption[]                         = "--sync";
const char kRemoveUnsupportedOption[]            = "--remove-unsupported";
const char kValidateOption[]                     = "--validate";
const char kDebugDeviceLostOption[]              = "--debug-device-lost";
const char kCreateDummyAllocationsOption[]       = "--create-dummy-allocations";
const char kOmitNullHardwareBuffersLongOption[]  = "--omit-null-hardware-buffers";
const char kOmitNullHardwareBuffersShortOption[] = "--onhb";
const char kDeniedMessages[]                     = "--denied-messages";
const char kAllowedMessages[]                    = "--allowed-messages";
const char kShaderReplaceArgument[]              = "--replace-shaders";
const char kScreenshotAllOption[]                = "--screenshot-all";
const char kScreenshotRangeArgument[]            = "--screenshots";
const char kScreenshotIntervalArgument[]         = "--screenshot-interval";
const char kScreenshotFormatArgument[]           = "--screenshot-format";
const char kScreenshotDirArgument[]              = "--screenshot-dir";
const char kScreenshotFilePrefixArgument[]       = "--screenshot-prefix";
const char kScreenshotSizeArgument[]             = "--screenshot-size";
const char kScreenshotScaleArgument[]            = "--screenshot-scale";
const char kForceWindowedShortArgument[]         = "--fw";
const char kForceWindowedLongArgument[]          = "--force-windowed";
const char kForceWindowWithOriginShortArgument[] = "--fwo";
const char kForceWindowWithOriginLongArgument[]  = "--force-windowed-origin";
const char kOutput[]                             = "--output";
const char kMeasurementRangeArgument[]           = "--measurement-frame-range";
const char kMeasurementFileArgument[]            = "--measurement-file";
const char kQuitAfterMeasurementRangeOption[]    = "--quit-after-measurement-range";
const char kQuitAfterFrameArgument[]             = "--quit-after-frame";
const char kFlushMeasurementRangeOption[]        = "--flush-measurement-range";
const char kFlushInsideMeasurementRangeOption[]  = "--flush-inside-measurement-range";
const char kSwapchainOption[]                    = "--swapchain";
const char kPresentModeOption[]                  = "--present-mode";
const char kEnableUseCapturedSwapchainIndices[] =
    "--use-captured-swapchain-indices"; // The same: util::SwapchainOption::kCaptured
const char kVirtualSwapchainSkipBlitShortOption[] = "--vssb";
const char kVirtualSwapchainSkipBlitLongOption[]  = "--virtual-swapchain-skip-blit";
const char kColorspaceFallback[]                  = "--use-colorspace-fallback";
const char kOffscreenSwapchainFrameBoundary[]     = "--offscreen-swapchain-frame-boundary";
const char kFormatArgument[]                      = "--format";
const char kIncludeBinariesOption[]               = "--include-binaries";
const char kExpandFlagsOption[]                   = "--expand-flags";
const char kFilePerFrameOption[]                  = "--file-per-frame";
const char kFrameRange[]                          = "--frame-range";
const char kSkipGetFenceStatus[]                  = "--skip-get-fence-status";
const char kSkipGetFenceRanges[]                  = "--skip-get-fence-ranges";
const char kWaitBeforePresent[]                   = "--wait-before-present";
const char kPrintBlockInfoAllOption[]             = "--pbi-all";
const char kPrintBlockInfosArgument[]             = "--pbis";
const char kNumPipelineCreationJobs[]             = "--pipeline-creation-jobs";
const char kPreloadMeasurementRangeOption[]       = "--preload-measurement-range";
const char kSavePipelineCacheArgument[]           = "--save-pipeline-cache";
const char kLoadPipelineCacheArgument[]           = "--load-pipeline-cache";
const char kCreateNewPipelineCacheOption[]        = "--add-new-pipeline-caches";
const char kDeduplicateDevice[]                   = "--deduplicate-device";
const char kWaitBeforeFirstSubmit[]               = "--wait-before-first-submit";

const char kScreenshotIgnoreFrameBoundaryArgument[] = "--screenshot-ignore-FrameBoundaryANDROID";

#if defined(WIN32)
const char kDxTwoPassReplay[]                  = "--dx12-two-pass-replay";
const char kDxOverrideObjectNames[]            = "--dx12-override-object-names";
const char kDxAgsMarkRenderPasses[]            = "--dx12-ags-inject-markers";
const char kBatchingMemoryUsageArgument[]      = "--batching-memory-usage";
const char kDumpResourcesModifiableStateOnly[] = "--dump-resources-modifiable-state-only";
const char kDumpResourcesBeforeDrawOption[]    = "--dump-resources-before-draw";
#endif

const char kDumpResourcesArgument[]    = "--dump-resources";
const char kDumpResourcesDirArgument[] = "--dump-resources-dir";

enum class WsiPlatform
{
    kAuto,
    kWin32,
    kXlib,
    kXcb,
    kWayland,
    kMetal,
    kDisplay,
    kHeadless
};

const char kWsiPlatformAuto[]     = "auto";
const char kWsiPlatformWin32[]    = "win32";
const char kWsiPlatformXlib[]     = "xlib";
const char kWsiPlatformXcb[]      = "xcb";
const char kWsiPlatformWayland[]  = "wayland";
const char kWsiPlatformMetal[]    = "metal";
const char kWsiPlatformDisplay[]  = "display";
const char kWsiPlatformHeadless[] = "headless";

const char kMemoryTranslationNone[]    = "none";
const char kMemoryTranslationRemap[]   = "remap";
const char kMemoryTranslationRealign[] = "realign";
const char kMemoryTranslationRebind[]  = "rebind";

const char kSwapchainVirtual[]   = "virtual";
const char kSwapchainCaptured[]  = "captured";
const char kSwapchainOffscreen[] = "offscreen";

const char kPresentModeCapture[]     = "capture";
const char kPresentModeImmediate[]   = "immediate";
const char kPresentModeMailbox[]     = "mailbox";
const char kPresentModeFifo[]        = "fifo";
const char kPresentModeFifoRelaxed[] = "fifo_relaxed";

#if defined(__ANDROID__)
const char kDefaultScreenshotDir[]    = "/sdcard";
const char kDefaultDumpResourcesDir[] = "/sdcard";
#else
const char kDefaultScreenshotDir[]                  = "";
const char kDefaultDumpResourcesDir[]               = "";
const bool kDefaultDumpResourcesModifiableStateOnly = false;
#endif

GFXRECON_BEGIN_NAMESPACE(gfxrecon)
GFXRECON_BEGIN_NAMESPACE(tools)

enum CmdLineRetType
{
    CmdLineRetType_Success = 0,
    CmdLineRetType_PrintUsage,
    CmdLineRetType_PrintVersion,
    CmdLineRetType_Error
};

class ToolSettings
{
  public:
    ToolSettings(gfxrecon::util::settings::GfxrToolType tool_type);
    ~ToolSettings();

    CmdLineRetType ProcessCommandLine(int                       argc,
                                      const char**              argv,
                                      uint32_t                  expected_num_of_extra_args,
                                      std::vector<std::string>& extra_args);
    CmdLineRetType ProcessCommandLine(std::string               command_line_args,
                                      uint32_t                  expected_num_of_extra_args,
                                      std::vector<std::string>& extra_args);

    // Disable the debug pop-up
    void ProcessDisableDebugPopup();

    // Is the GFXR capture layer enabled
    bool IsCaptureLayerEnabled();

    // Get any pause settings (function return is if we should pause true/false)
    uint32_t GetPauseFrame(void);

    bool IsPreloadEnabled();

    // API-specific replay options
    gfxrecon::decode::VulkanReplayOptions
    GetVulkanReplayOptions(const std::string&                              filename,
                           gfxrecon::decode::VulkanTrackedObjectInfoTable* tracked_object_info_table);

    // WSI funcs
    // ====================
    std::string GetWsiArgString();

    /// @brief Selects the WSI extension name based on the WSI platform.
    /// @return If WsiPlatform::kAuto, returns the first available WSI extension name.
    ///         Otherwise, returns the WSI extension name for the specified platform.
    std::string GetFirstWsiExtensionName();

    /// @brief Selects the WSI extension name based on the WSI platform.
    /// @return If WsiPlatform::kAuto, returns an empty string.
    ///         Otherwise, returns the WSI extension name for the specified platform.
    std::string GetWsiExtensionName();

    bool GetQuitAfterFrame(uint32_t& quit_frame);

    void GetMeasurementFilename(std::string& file_name);
    bool GetMeasurementFrameRange(uint32_t& start_frame, uint32_t& end_frame);

  private:
    bool GenerateArgumentVector(int32_t argc, const char** const argv, std::vector<std::string>& cmd_line_vector);
    bool GenerateArgumentVector(std::string, std::vector<std::string>& cmd_line_vector);
    CmdLineRetType ProcessCommandLine(const std::vector<std::string>& command_line_args,
                                      uint32_t                        expected_num_of_extra_args,
                                      std::vector<std::string>&       extra_args);

    void GetLayerListFromEnvVar(const char* env_var, std::vector<std::string>& layer_list);

    WsiPlatform GetWsiPlatform();
    std::string GetFirstWsiExtensionName(WsiPlatform platform);

    // Replay options private methods
    void GetReplayOptions(gfxrecon::decode::ReplayOptions& options, const std::string& filename);
    void CheckForceWindowed(gfxrecon::decode::ReplayOptions& options);
    void CheckWindowOrigin(gfxrecon::decode::ReplayOptions& options);

    gfxrecon::decode::CreateResourceAllocator
    GetCreateResourceAllocatorFunc(const std::string&                              filename,
                                   const gfxrecon::decode::VulkanReplayOptions&    replay_options,
                                   gfxrecon::decode::VulkanTrackedObjectInfoTable* tracked_object_info_table);

    std::vector<int32_t> GetFilteredMsgs(const std::string& messages);

    // Modifies settings parameter with values set via command line
    void GetLogSettings(gfxrecon::util::Log::Settings& log_settings);

    // Private screenshot methods
    std::string                                    GetScreenshotDir();
    gfxrecon::util::ScreenshotFormat               GetScreenshotFormat();
    void                                           GetScreenshotSize(uint32_t& width, uint32_t& height);
    float                                          GetScreenshotScale();
    std::vector<gfxrecon::decode::ScreenshotRange> GetScreenshotRanges();

    // Private dump-resources methods
    std::string                       GetDumpResourcesDir();
    gfxrecon::util::ScreenshotFormat  GetDumpresourcesImageFormat();
    gfxrecon::format::CompressionType GetDumpResourcesCompressionType();
    float                             GetDumpResourcesScale();

    // Members
    gfxrecon::util::settings::GfxrToolType tool_type_;
};

static bool PrintVersion(const char* exe_name)
{
    std::string app_name     = exe_name;
    size_t      dir_location = app_name.find_last_of("/\\");

    if (dir_location >= 0)
    {
        app_name.replace(0, dir_location + 1, "");
    }

    GFXRECON_WRITE_CONSOLE("%s version info:", app_name.c_str());
    GFXRECON_WRITE_CONSOLE("  GFXReconstruct Version %s", GetProjectVersionString());
    GFXRECON_WRITE_CONSOLE("  Vulkan Header Version %u.%u.%u",
                           VK_VERSION_MAJOR(VK_HEADER_VERSION_COMPLETE),
                           VK_VERSION_MINOR(VK_HEADER_VERSION_COMPLETE),
                           VK_VERSION_PATCH(VK_HEADER_VERSION_COMPLETE));

#if ENABLE_OPENXR_SUPPORT
    GFXRECON_WRITE_CONSOLE("  OpenXR Header Version %u.%u.%u",
                           XR_VERSION_MAJOR(XR_CURRENT_API_VERSION),
                           XR_VERSION_MINOR(XR_CURRENT_API_VERSION),
                           XR_VERSION_PATCH(XR_CURRENT_API_VERSION));
#endif

    return true;
}

GFXRECON_END_NAMESPACE(tools)
GFXRECON_END_NAMESPACE(gfxrecon)

// TODO: Remove these settings functions after the transition to the new settings
//       manager is complete for all tools

// Modifies settings parameter with values set via command line
static void GetLogSettings(const gfxrecon::util::ArgumentParser& arg_parser,
                           gfxrecon::util::Log::Settings&        log_settings)
{
    // Parse log level
    gfxrecon::util::LoggingSeverity log_level;
    const std::string&            value_string = arg_parser.GetArgumentValue(kLogLevelArgument);
    if (value_string.empty() || !gfxrecon::util::Log::StringToSeverity(value_string, log_level))
    {
        log_level = gfxrecon::decode::kDefaultLogLevel;
        if (!value_string.empty())
        {
            GFXRECON_LOG_WARNING("Ignoring unrecognized log level option value \"%s\"", value_string.c_str());
        }
    }

    // Update settings
    log_settings.min_severity              = log_level;
    log_settings.output_timestamps         = arg_parser.IsOptionSet(kLogTimestampsOption);
    log_settings.file_name                 = arg_parser.GetArgumentValue(kLogFileArgument);
    log_settings.output_to_os_debug_string = arg_parser.IsOptionSet(kLogDebugView);
}

static WsiPlatform GetWsiPlatform(const gfxrecon::util::ArgumentParser& arg_parser)
{
    WsiPlatform wsi_platform = WsiPlatform::kAuto;
    const auto& value        = arg_parser.GetArgumentValue(kWsiArgument);

    if (!value.empty())
    {
        if (gfxrecon::util::platform::StringCompareNoCase(kWsiPlatformAuto, value.c_str()) == 0)
        {
            wsi_platform = WsiPlatform::kAuto;
        }
        else if (gfxrecon::util::platform::StringCompareNoCase(kWsiPlatformWin32, value.c_str()) == 0)
        {
#if defined(VK_USE_PLATFORM_WIN32_KHR)
            wsi_platform = WsiPlatform::kWin32;
#else
            GFXRECON_LOG_WARNING("Ignoring wsi option \"%s\", which is not enabled on this system", value.c_str());
#endif
        }
        else if (gfxrecon::util::platform::StringCompareNoCase(kWsiPlatformXlib, value.c_str()) == 0)
        {
#if defined(VK_USE_PLATFORM_XLIB_KHR)
            wsi_platform = WsiPlatform::kXlib;
#else
            GFXRECON_LOG_WARNING("Ignoring wsi option %s, which is not enabled on this system", value.c_str());
#endif
        }
        else if (gfxrecon::util::platform::StringCompareNoCase(kWsiPlatformXcb, value.c_str()) == 0)
        {
#if defined(VK_USE_PLATFORM_XCB_KHR)
            wsi_platform = WsiPlatform::kXcb;
#else
            GFXRECON_LOG_WARNING("Ignoring wsi option \"%s\", which is not enabled on this system", value.c_str());
#endif
        }
        else if (gfxrecon::util::platform::StringCompareNoCase(kWsiPlatformWayland, value.c_str()) == 0)
        {
#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
            wsi_platform = WsiPlatform::kWayland;
#else
            GFXRECON_LOG_WARNING("Ignoring wsi option \"%s\", which is not enabled on this system", value.c_str());
#endif
        }
        else if (gfxrecon::util::platform::StringCompareNoCase(kWsiPlatformMetal, value.c_str()) == 0)
        {
#if defined(VK_USE_PLATFORM_METAL_EXT)
            wsi_platform = WsiPlatform::kMetal;
#else
            GFXRECON_LOG_WARNING("Ignoring wsi option \"%s\", which is not enabled on this system", value.c_str());
#endif
        }
        else if (gfxrecon::util::platform::StringCompareNoCase(kWsiPlatformDisplay, value.c_str()) == 0)
        {
#if defined(VK_USE_PLATFORM_DISPLAY_KHR)
            wsi_platform = WsiPlatform::kDisplay;
#else
            GFXRECON_LOG_WARNING("Ignoring wsi option \"%s\", which is not enabled on this system", value.c_str());
#endif
        }
        else if (gfxrecon::util::platform::StringCompareNoCase(kWsiPlatformHeadless, value.c_str()) == 0)
        {
#if defined(VK_USE_PLATFORM_HEADLESS)
            wsi_platform = WsiPlatform::kHeadless;
#else
            GFXRECON_LOG_WARNING("Ignoring wsi option \"%s\", which is not enabled on this system", value.c_str());
#endif
        }
        else
        {
            GFXRECON_LOG_WARNING("Ignoring unrecognized wsi option \"%s\"", value.c_str());
        }
    }

    return wsi_platform;
}

/// @brief Selects the WSI extension name based on the WSI platform.
/// @param wsi_platform The WSI platform to select the extension name for.
/// @return If WsiPlatform::kAuto, returns the first available WSI extension name.
///         Otherwise, returns the WSI extension name for the specified platform.
static std::string GetFirstWsiExtensionName(WsiPlatform wsi_platform)
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

/// @brief Selects the WSI extension name based on the WSI platform.
/// @param wsi_platform The WSI platform to select the extension name for.
/// @return If WsiPlatform::kAuto, returns an empty string.
///         Otherwise, returns the WSI extension name for the specified platform.
static std::string GetWsiExtensionName(WsiPlatform wsi_platform)
{
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

static std::string GetWsiArgString()
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

static bool CheckOptionPrintVersion(const char* exe_name, const gfxrecon::util::ArgumentParser& arg_parser)
{
    if (arg_parser.IsOptionSet(kVersionOption))
    {
        std::string app_name     = exe_name;
        size_t      dir_location = app_name.find_last_of("/\\");

        if (dir_location >= 0)
        {
            app_name.replace(0, dir_location + 1, "");
        }

        GFXRECON_WRITE_CONSOLE("%s version info:", app_name.c_str());
        GFXRECON_WRITE_CONSOLE("  GFXReconstruct Version %s", GetProjectVersionString());
        GFXRECON_WRITE_CONSOLE("  Vulkan Header Version %u.%u.%u",
                               VK_VERSION_MAJOR(VK_HEADER_VERSION_COMPLETE),
                               VK_VERSION_MINOR(VK_HEADER_VERSION_COMPLETE),
                               VK_VERSION_PATCH(VK_HEADER_VERSION_COMPLETE));

#if ENABLE_OPENXR_SUPPORT
        GFXRECON_WRITE_CONSOLE("  OpenXR Header Version %u.%u.%u",
                               XR_VERSION_MAJOR(XR_CURRENT_API_VERSION),
                               XR_VERSION_MINOR(XR_CURRENT_API_VERSION),
                               XR_VERSION_PATCH(XR_CURRENT_API_VERSION));
#endif

        return true;
    }

    return false;
}

typedef void (*fnPrintUsage)(const char* exe_name);

static bool
CheckOptionPrintUsage(const char* exe_name, const gfxrecon::util::ArgumentParser& arg_parser, fnPrintUsage print_usage)
{
    if (arg_parser.IsOptionSet(kHelpShortOption) || arg_parser.IsOptionSet(kHelpLongOption))
    {
        print_usage(exe_name);
        return true;
    }

    return false;
}

#endif // GFXRECON_PLATFORM_SETTINGS_H
