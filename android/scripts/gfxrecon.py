#!/usr/bin/env python3
#
# Copyright (c) 2018-2025 LunarG, Inc.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to
# deal in the Software without restriction, including without limitation the
# rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
# sell copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
# IN THE SOFTWARE.

import argparse
import os
import sys
import shlex
import subprocess

argv = sys.argv
argc = len(sys.argv)

# Supported commands
valid_commands = [
    'install-apk',
    'multiwin-replay',
    'replay'
]

# Arguments
# gfxrecon install-apk <file>
# gfxrecon replay [-p | --push-file <file-on-desktop>] <file-on-device>
# gfxrecon multiwin-replay [-p | --push-file <file-on-desktop>] <file-on-device>

# Application info
app_name = 'com.lunarg.gfxreconstruct.replay'
app_activity = '"com.lunarg.gfxreconstruct.replay/android.app.NativeActivity"'
multiwin_app_activity = '"com.lunarg.gfxreconstruct.replay/.ReplayActivity"'
app_action = 'android.intent.action.MAIN'
app_category = 'android.intent.category.LAUNCHER'

# ADB commands
adb_install = 'adb install -g -t -r'
adb_sdk_version = 'adb shell getprop ro.build.version.sdk'
adb_stop = 'adb shell am force-stop {}'.format(app_name)
adb_push = 'adb push'
adb_devices = 'adb devices'

# Environment variable for android serial number
android_serial = 'ANDROID_SERIAL'

class DeviceSelectionException(Exception):
    pass

def QueryAvailableDevices():
    result = subprocess.run(shlex.split(adb_devices, posix='win' not in sys.platform), capture_output=True, check=True)
    devices = result.stdout.decode().strip().splitlines()[1:]
    return [device.split('\t')[0] for device in devices]

def CheckDeviceSelection():
    devices = QueryAvailableDevices()
    if len(devices) <= 1:
        return

    selection = os.getenv(android_serial)
    if selection is None or selection == '':
        raise DeviceSelectionException('Multiple devices detected - you must specify which one to use by setting ANDROID_SERIAL environment variable.')
    if selection not in devices:
        raise DeviceSelectionException(f'Selected ({selection}) device not present. Available devices: {devices}')

def CreateCommandParser():
    parser = argparse.ArgumentParser(description='GFXReconstruct utility launcher for Android.')
    parser.add_argument('command', choices=valid_commands, metavar='command', help='Command to execute. Valid options are [{}]'.format(', '.join(valid_commands)))
    parser.add_argument('args', nargs=argparse.REMAINDER, help='Command-specific argument list. Specify -h after command name for more.')
    return parser

def CreateInstallApkParser():
    parser = argparse.ArgumentParser(prog=os.path.basename(sys.argv[0]) + ' install-apk', description='Install the replay tool.')
    parser.add_argument('file', help='APK file to install')
    parser.add_argument('-s', '--select', metavar='DEVICE_ID', help='Specify the destination device id. Needed if multiple devices are attached.')
    return parser

# Create the replay parser
def CreateReplayParser():
    parser = argparse.ArgumentParser(prog=os.path.basename(sys.argv[0]) + ' replay', description='Launch the replay tool.')
    parser.add_argument('-p', '--push-file', metavar='LOCAL_FILE', help='Local file to push to the location on device specified by <file>')

    # START REPLAY CODEGEN INSERT
    # **WARNING** Do not manually edit the following code.
    # *********** This code was added by the generate_settings.py script using the
    # *********** settings.json input file.  Please add new settings there.
    parser.add_argument('--log-file', metavar='LOG_FILE', help='When set, log messages will be written to a file at the specified location. (forwarded to gfxrecon-replay tool)')
    parser.add_argument('--log-level', metavar='LEVEL', choices='debug,info,warning,error,fatal', help='Specify the highest level message to log.  The specified level and all levels listed after it will be enabled for logging.  For example, choosing the `warning` level will also enable the `error` and `fatal` levels. (forwarded to gfxrecon-replay tool)')
    parser.add_argument('--log-timestamps', action='store_true', default=False, help='Output a timestamp in front of each log message. (forwarded to gfxrecon-replay tool)')
    parser.add_argument('--screenshot-dir', metavar='DIR', help='Directory used where screenshots are saved to.  Default is the current working directory. (forwarded to gfxrecon-replay tool)')
    parser.add_argument('--screenshot-format', metavar='FORMAT', choices='bmp,png', help='Image file format to use when saving screenshots. (forwarded to gfxrecon-replay tool)')
    parser.add_argument('--screenshots', metavar='RANGE', help='Generate screenshots for the specified frames. Target frames are specified as a comma separated list of frame ranges.  A frame range can be specified as a single value, to specify a single frame, or as two hyphenated values, to specify the first and last frames to process.  Frame ranges should be specified in ascending order and cannot overlap.  Note that frame numbering is 1-based (i.e. the first frame is frame 1). Example: 200,301-305 will generate six screenshots. (forwarded to gfxrecon-replay tool)')
    parser.add_argument('--screenshot-interval', metavar='N', help='Sets the number between every screenshot taken.  1 means every frame is screenshot, 2 means every other, etc. Example: If screenshot range is 10-15 and interval is 2, screenshot will be generated for frames 10, 12 and 14. (forwarded to gfxrecon-replay tool)')
    parser.add_argument('--add-new-pipeline-caches', action='store_true', default=False, help='If set, allows gfxreconstruct to create new vkPipelineCache objects when it encounters a pipeline created without cache. This option can be used in coordination with `--save-pipeline-cache` and `--load-pipeline-cache`. (forwarded to gfxrecon-replay tool)')
    parser.add_argument('--capture', action='store_true', default=False, help='Capture the replaying GFXR file. Capture uses the same log options as replay. All other capture option behavior and usage is the same as when capturing with the GFXR layer. The capture functionality is included in the `gfxrecon-replay` executable -- no GFXR capture layer is added to the Vulkan layer chain. (forwarded to gfxrecon-replay tool)')
    parser.add_argument('--cpu-mask', metavar='BINARY-MASK', help='Set of CPU cores used by the replayer. `binary-mask` is a succession of "0" and "1" read from left to right that specifies used/unused cores. For example "10010" activates the first and fourth cores and deactivate all other cores. If the option is not set, all cores can be used. If the option is set only for some cores, the other cores are not used. (forwarded to gfxrecon-replay tool)')
    parser.add_argument('--debug-messenger-level', metavar='LEVEL', choices='debug,info,warning,error,fatal', help='Specify highest debug messenger severity level. Options are: debug, info, warning, and error. (forwarded to gfxrecon-replay tool)')
    parser.add_argument('--deduplicate-device', action='store_true', default=False, help='If set, at most one VkDevice will be created for each VkPhysicalDevice for RenderDoc and DXVK case. (forwarded to gfxrecon-replay tool)')
    parser.add_argument('--dump-resources', metavar='ARGS', help='Dump out resources during replay (forwarded to gfxrecon-replay tool)')
    parser.add_argument('--dump-resources-before-draw', action='store_true', default=False, help='In addition to dumping GPU resources after the draw calls specified by the `--dump-resources` argument, also dump resources before the draw calls. (forwarded to gfxrecon-replay tool)')
    parser.add_argument('--dump-resources-binary-file-compression-type', metavar='COMPRESSION-FORMAT', choices='lz4,zlib,zstd,none', help='[Vulkan Only] Compress files that are dumped as binary.  (forwarded to gfxrecon-replay tool)')
    parser.add_argument('--dump-resources-dir', metavar='DIR', help='Directory to write dump resources output files. Default is the current working directory. (forwarded to gfxrecon-replay tool)')
    parser.add_argument('--dump-resources-dump-all-image-subresources', action='store_true', default=False, help='Dump all available mip levels and layers when dumping images. (forwarded to gfxrecon-replay tool)')
    parser.add_argument('--dump-resources-dump-build-acceleration-structures-input-buffers', action='store_true', default=False, help='Dump all input buffers used in vkCmdBuildAccelerationStructures. This includes vertex, index, transformation matrix, AABB and instance buffers. Default is off. (forwarded to gfxrecon-replay tool)')
    parser.add_argument('--dump-resources-dump-color-attachment-index', metavar='INDEX', help='Configures which color attachment to dump when dumping draw calls.  Default is all attachments. (forwarded to gfxrecon-replay tool)')
    parser.add_argument('--dump-resources-dump-depth-attachment', action='store_true', default=False, help='Configures whether to dump the depth attachment of draw calls. Default is false. (forwarded to gfxrecon-replay tool)')
    parser.add_argument('--dump-resources-dump-raw-images', action='store_true', default=False, help='Dump the images in raw format (without conversion), otherwise they will be transformed into a VK_FORMAT_B8G8R8A8_* format, more suitable for dumping in an image file. (forwarded to gfxrecon-replay tool)')
    parser.add_argument('--dump-resources-dump-separate-alpha', action='store_true', default=False, help='Dump the image alpha as a separate file from the main image. (forwarded to gfxrecon-replay tool)')
    parser.add_argument('--dump-resources-dump-unused-vertex-bindings', action='store_true', default=False, help='Dump a vertex binding even if no vertex attributes references it. (forwarded to gfxrecon-replay tool)')
    parser.add_argument('--dump-resources-dump-vertex-index-buffers', action='store_true', default=False, help='Enable dumping of vertex and index buffers while dumping draw call resources. Default is disabled. (forwarded to gfxrecon-replay tool)')
    parser.add_argument('--dump-resources-image-format', metavar='FORMAT', choices='bmp,png', help='Image file format to use when dumping image resources. Available formats are: bmp, png. (forwarded to gfxrecon-replay tool)')
    parser.add_argument('--dump-resources-json-output-per-command', action='store_true', default=False, help='Enable storing a json output file for each dumped command.  Overrides default behavior which is generating one output json file that contains the information for all dumped commands. (forwarded to gfxrecon-replay tool)')
    parser.add_argument('--dump-resources-scale', metavar='SCALE', help='Scale images generated by dump resources by the given scale factor. The scale factor must be a floating point number greater than 0. Values greater than 10 are capped at 10. Default value is 1.0. (forwarded to gfxrecon-replay tool)')
    parser.add_argument('--flush-inside-measurement-range', action='store_true', default=False, help='If this is specified the replayer will flush and wait for all current GPU work to finish at the end of each frame inside the measurement range. (forwarded to gfxrecon-replay tool)')
    parser.add_argument('--fmr', '--flush-measurement-range', action='store_true', default=False, help='If this is specified the replayer will flush and wait for all current GPU work to finish at the start and end of the measurement range. (forwarded to gfxrecon-replay tool)')
    parser.add_argument('--gpu', metavar='INDEX', help='Use the specified device for replay, where index is the zero-based index to the array of physical devices returned by vkEnumeratePhysicalDevices or IDXGIFactory1::EnumAdapters1. Replay may fail if the specified device is not compatible with the original capture devices. (forwarded to gfxrecon-replay tool)')
    parser.add_argument('--gpu-group', metavar='INDEX', help='Use the specified device group for replay, where index is the zero-based index to the array of physical device group returned by vkEnumeratePhysicalDeviceGroups.  Replay may fail if the specified device group is not compatible with the original capture device group. (forwarded to gfxrecon-replay tool)')
    parser.add_argument('-h', '--help', action='store_true', default=False, help='Print usage information and exit (forwarded to gfxrecon-replay tool)')
    parser.add_argument('--input-capture', metavar='INPUT_FILE', help='The name of the capture file to use for processing.  This overrides the non-tagged input filename that used to appear after all optional arguments (forwarded to gfxrecon-replay tool)')
    parser.add_argument('--load-pipeline-cache', metavar='CACHE-FILE', help='If set, loads data created by the `--save-pipeline-cache` option in <cache-file> and uses it to create the pipelines instead of the pipeline caches saved at capture time. (forwarded to gfxrecon-replay tool)')
    parser.add_argument('--log-debugview', action='store_true', default=False, help='Log messages with OutputDebugStringA. (forwarded to gfxrecon-replay tool)')
    parser.add_argument('--measurement-file', metavar='FILE', help='Write measurements to a file at the specified path. Default is: "/sdcard/gfxrecon-measurements.json" on Android and"./gfxrecon-measurements.json" on desktop. (forwarded to gfxrecon-replay tool)')
    parser.add_argument('--mfr', '--measurement-frame-range', metavar='START_FRAME-END_FRAME', help='Custom framerange to measure FPS for. This range will include the start frame but not the end frame. The measurement frame range defaults to all frames except the loading frame but can be configured for any range. If the end frame is past the last frame in the trace it will be clamped to the frame after the last (so in that case the results would include the last frame). (forwarded to gfxrecon-replay tool)')
    parser.add_argument('-m', '--memory-translation', metavar='MODE', choices='none,remap,realign,rebind', help='Enable memory translation for replay on GPUs with memory types that are not compatible with the capture GPU"s memory types (forwarded to gfxrecon-replay tool)')
    parser.add_argument('--offscreen-swapchain-frame-boundary', action='store_true', default=False, help='Should only be used with offscreen swapchain. Activates the extension VK_EXT_frame_boundary (always supported if trimming, checks for driver support otherwise) and inserts command buffer submission with VkFrameBoundaryEXT where vkQueuePresentKHR was called in the original capture.  This allows preserving frames when capturing a replay that uses offscreen swapchain. (forwarded to gfxrecon-replay tool)')
    parser.add_argument('--onhb', '--omit-null-hardware-buffers', action='store_true', default=False, help='[Android Only] Omit calls that use a NULL AHardwareBuffer (forwarded to gfxrecon-replay tool)')
    parser.add_argument('--opcd', '--omit-pipeline-cache-data', action='store_true', default=False, help='Omit pipeline cache data from calls to vkCreatePipelineCache and skip calls to vkGetPipelineCacheData (forwarded to gfxrecon-replay tool)')
    parser.add_argument('--pause-frame', metavar='FRAME', help='Pause after replay frame number <frame> (forwarded to gfxrecon-replay tool)')
    parser.add_argument('--paused', action='store_true', default=False, help='Pause after replaying the first frame (same as `--pause-frame 1`). (forwarded to gfxrecon-replay tool)')
    parser.add_argument('--pbi-all', action='store_true', default=False, help='Print all block information. (forwarded to gfxrecon-replay tool)')
    parser.add_argument('--pbis', metavar='INDEX1,INDEX2', help='Print block information between block index1 and block index2. (forwarded to gfxrecon-replay tool)')
    parser.add_argument('--pcj', '--pipeline-creation-jobs', metavar='NUM_JOBS', help='Specify the number of asynchronous pipeline-creation jobs as integer. If <num_jobs> is negative it will be added to the number of cpu-cores Default: 0 (do not use asynchronous operations). (forwarded to gfxrecon-replay tool)')
    parser.add_argument('--preload-measurement-range', action='store_true', default=False, help='Preload the provided measurement range of frames. (forwarded to gfxrecon-replay tool)')
    parser.add_argument('--present-mode', metavar='MODE', choices='capture,immediate,mailbox,fifo,fifo_relaxed', help='[Vulkan Only] Override the VkPresentModeKHR of the swapchain value during replay (forwarded to gfxrecon-replay tool)')
    parser.add_argument('--quit-after-frame', metavar='FRAME', help='Quit replay after frame <frame> has completed. (forwarded to gfxrecon-replay tool)')
    parser.add_argument('--qamr', '--quit-after-measurement-range', action='store_true', default=False, help='If this is specified the replayer will abort when it reaches the <end_frame> specified in the `--measurement-frame-range` argument. (forwarded to gfxrecon-replay tool)')
    parser.add_argument('--remove-unsupported', action='store_true', default=False, help='Remove unsupported extensions and features from instance and device creation parameters. (forwarded to gfxrecon-replay tool)')
    parser.add_argument('--replace-shaders', metavar='DIR', help='Replace the shader code in each CreateShaderModule with the contents of the file <dir>/sh<handle_id> if found, where <handle_id> is the handle id of the CreateShaderModule call. See gfxrecon-extract. (forwarded to gfxrecon-replay tool)')
    parser.add_argument('--save-pipeline-cache', metavar='CACHE-FILE', help='If set, produces pipeline caches at replay time instead of using the one saved at capture time and save those caches in <cache-file> (forwarded to gfxrecon-replay tool)')
    parser.add_argument('--screenshot-all', action='store_true', default=False, help='Generate screenshots for all frames.  When this option is specified, other screenshot frame-related options are ignored. (forwarded to gfxrecon-replay tool)')
    parser.add_argument('--screenshot-ignore-FrameBoundaryANDROID', action='store_true', default=False, help='If set, frames switched with vkFrameBoundANDROID will be ignored from the screenshot handler. (forwarded to gfxrecon-replay tool)')
    parser.add_argument('--screenshot-prefix', metavar='FILE-PREFIX', help='Prefix to apply to the screenshot file name.  Default is "screenshot", producing file names similar to "screenshot_frame_8049.bmp" (forwarded to gfxrecon-replay tool)')
    parser.add_argument('--screenshot-scale', metavar='SCALE', help='Specify a decimal factor which will determine screenshot sizes. The factor will be multiplied with the swapchain images dimension to determine the screenshot dimensions. Default value is 1.0. (forwarded to gfxrecon-replay tool)')
    parser.add_argument('--screenshot-size', metavar='WIDTHXHEIGHT', help='Specify desired screenshot dimensions. Leaving this unspecified screenshots will use the swapchain images dimensions. If `--screenshot-scale` is also specified then this option is ignored. (forwarded to gfxrecon-replay tool)')
    parser.add_argument('--sfa', '--skip-failed-allocations', action='store_true', default=False, help='Skip vkAllocateMemory, vkAllocateCommandBuffers, and vkAllocateDescriptorSets calls that failed during capture. (forwarded to gfxrecon-replay tool)')
    parser.add_argument('--sgfr', '--skip-get-fence-ranges', metavar='FRAME-RANGES', help='[Vulkan Only] Frame ranges where `--sgfs` applies. The format is: <frame-start-1>-<frame-end-1>[,<frame-start-1>-<frame-end-1>]* (forwarded to gfxrecon-replay tool)')
    parser.add_argument('--sgfs', '--skip-get-fence-status', metavar='STATUS', help='[Vulkan Only] Specify behaviour to skip calls to vkWaitForFences and vkGetFenceStatus. <status> = 0: Don"t skip, <status> = 1: Skip unsuccessful calls, <status> = 2: Always Skip. If no skip frame range is specified (through `--sgfr`), the status applies to all frames. Skip vkGetFenceStatus calls that return a provided status (forwarded to gfxrecon-replay tool)')
    parser.add_argument('--surface-index', metavar='N', help='[Vulkan Only] Restrict rendering to the Nth surface object created. Used with captures that include multiple surfaces.  Default is -1 (render to all surfaces). (forwarded to gfxrecon-replay tool)')
    parser.add_argument('--swapchain', metavar='MODE', choices='captured,offscreen,virtual', help='Choose a swapchain mode to replay. (forwarded to gfxrecon-replay tool)')
    parser.add_argument('--sync', action='store_true', default=False, help='Synchronize after each queue submission with vkQueueWaitIdle. (forwarded to gfxrecon-replay tool)')
    parser.add_argument('--use-captured-swapchain-indices', action='store_true', default=False, help='Same as `--swapchain captured`.  Ignored if the `--swapchain` option is used. (forwarded to gfxrecon-replay tool)')
    parser.add_argument('--use-colorspace-fallback', action='store_true', default=False, help='Swap the swapchain color space if unsupported by replay device. Check if color space is not supported by replay device and fallback to VK_COLOR_SPACE_SRGB_NONLINEAR_KHR. (forwarded to gfxrecon-replay tool)')
    parser.add_argument('--validate', action='store_true', default=False, help='Enable the Khronos Vulkan validation layer when replaying a Vulkan capture or the Direct3D debug layer when replaying a Direct3D 12 capture. (forwarded to gfxrecon-replay tool)')
    parser.add_argument('--version', action='store_true', default=False, help='Print version information and exit. (forwarded to gfxrecon-replay tool)')
    parser.add_argument('--vssb', '--virtual-swapchain-skip-blit', action='store_true', default=False, help='If using Virtual Swapchain, skip the actual BLTs to the actual HW surface to gain performance during replay. (forwarded to gfxrecon-replay tool)')
    parser.add_argument('--wait-before-present', action='store_true', default=False, help='Force wait on completion of queue operations for all queues before calling Present. This is needed for accurate acquisition of instrumentation data on some platforms. (forwarded to gfxrecon-replay tool)')
    parser.add_argument('--wsi', metavar='PLATFORM', choices='auto,win32,xlib,xcb,wayland,display,headless', help='Force replay to use the specified wsi platform. (forwarded to gfxrecon-replay tool)')

    return parser

# Handle replay parser
def MakeExtrasString(args):
    arg_list = []

    if args.log_file:
        arg_list.append('--log-file')
        arg_list.append('{}'.format(args.log_file))

    if args.log_level:
        arg_list.append('--log-level')
        arg_list.append('{}'.format(args.log_level))

    if args.log_timestamps:
        arg_list.append('--log-timestamps')

    if args.screenshot_dir:
        arg_list.append('--screenshot-dir')
        arg_list.append('{}'.format(args.screenshot_dir))

    if args.screenshot_format:
        arg_list.append('--screenshot-format')
        arg_list.append('{}'.format(args.screenshot_format))

    if args.screenshots:
        arg_list.append('--screenshots')
        arg_list.append('{}'.format(args.screenshots))

    if args.screenshot_interval:
        arg_list.append('--screenshot-interval')
        arg_list.append('{}'.format(args.screenshot_interval))

    if args.add_new_pipeline_caches:
        arg_list.append('--add-new-pipeline-caches')

    if args.capture:
        arg_list.append('--capture')

    if args.cpu_mask:
        arg_list.append('--cpu-mask')
        arg_list.append('{}'.format(args.cpu_mask))

    if args.debug_messenger_level:
        arg_list.append('--debug-messenger-level')
        arg_list.append('{}'.format(args.debug_messenger_level))

    if args.deduplicate_device:
        arg_list.append('--deduplicate-device')

    if args.dump_resources:
        arg_list.append('--dump-resources')
        arg_list.append('{}'.format(args.dump_resources))

    if args.dump_resources_before_draw:
        arg_list.append('--dump-resources-before-draw')

    if args.dump_resources_binary_file_compression_type:
        arg_list.append('--dump-resources-binary-file-compression-type')
        arg_list.append('{}'.format(args.dump_resources_binary_file_compression_type))

    if args.dump_resources_dir:
        arg_list.append('--dump-resources-dir')
        arg_list.append('{}'.format(args.dump_resources_dir))

    if args.dump_resources_dump_all_image_subresources:
        arg_list.append('--dump-resources-dump-all-image-subresources')

    if args.dump_resources_dump_build_acceleration_structures_input_buffers:
        arg_list.append('--dump-resources-dump-build-acceleration-structures-input-buffers')

    if args.dump_resources_dump_color_attachment_index:
        arg_list.append('--dump-resources-dump-color-attachment-index')
        arg_list.append('{}'.format(args.dump_resources_dump_color_attachment_index))

    if args.dump_resources_dump_depth_attachment:
        arg_list.append('--dump-resources-dump-depth-attachment')

    if args.dump_resources_dump_raw_images:
        arg_list.append('--dump-resources-dump-raw-images')

    if args.dump_resources_dump_separate_alpha:
        arg_list.append('--dump-resources-dump-separate-alpha')

    if args.dump_resources_dump_unused_vertex_bindings:
        arg_list.append('--dump-resources-dump-unused-vertex-bindings')

    if args.dump_resources_dump_vertex_index_buffers:
        arg_list.append('--dump-resources-dump-vertex-index-buffers')

    if args.dump_resources_image_format:
        arg_list.append('--dump-resources-image-format')
        arg_list.append('{}'.format(args.dump_resources_image_format))

    if args.dump_resources_json_output_per_command:
        arg_list.append('--dump-resources-json-output-per-command')

    if args.dump_resources_scale:
        arg_list.append('--dump-resources-scale')
        arg_list.append('{}'.format(args.dump_resources_scale))

    if args.flush_inside_measurement_range:
        arg_list.append('--flush-inside-measurement-range')

    if args.flush_measurement_range:
        arg_list.append('--fmr')

    if args.gpu:
        arg_list.append('--gpu')
        arg_list.append('{}'.format(args.gpu))

    if args.gpu_group:
        arg_list.append('--gpu-group')
        arg_list.append('{}'.format(args.gpu_group))

    if args.help:
        arg_list.append('-h')

    if args.input_capture:
        arg_list.append('--input-capture')
        arg_list.append('{}'.format(args.input_capture))

    if args.load_pipeline_cache:
        arg_list.append('--load-pipeline-cache')
        arg_list.append('{}'.format(args.load_pipeline_cache))

    if args.log_debugview:
        arg_list.append('--log-debugview')

    if args.measurement_file:
        arg_list.append('--measurement-file')
        arg_list.append('{}'.format(args.measurement_file))

    if args.measurement_frame_range:
        arg_list.append('--mfr')
        arg_list.append('{}'.format(args.measurement_frame_range))

    if args.memory_translation:
        arg_list.append('-m')
        arg_list.append('{}'.format(args.memory_translation))

    if args.offscreen_swapchain_frame_boundary:
        arg_list.append('--offscreen-swapchain-frame-boundary')

    if args.omit_null_hardware_buffers:
        arg_list.append('--onhb')

    if args.omit_pipeline_cache_data:
        arg_list.append('--opcd')

    if args.pause_frame:
        arg_list.append('--pause-frame')
        arg_list.append('{}'.format(args.pause_frame))

    if args.paused:
        arg_list.append('--paused')

    if args.pbi_all:
        arg_list.append('--pbi-all')

    if args.pbis:
        arg_list.append('--pbis')
        arg_list.append('{}'.format(args.pbis))

    if args.pipeline_creation_jobs:
        arg_list.append('--pcj')
        arg_list.append('{}'.format(args.pipeline_creation_jobs))

    if args.preload_measurement_range:
        arg_list.append('--preload-measurement-range')

    if args.present_mode:
        arg_list.append('--present-mode')
        arg_list.append('{}'.format(args.present_mode))

    if args.quit_after_frame:
        arg_list.append('--quit-after-frame')
        arg_list.append('{}'.format(args.quit_after_frame))

    if args.quit_after_measurement_range:
        arg_list.append('--qamr')

    if args.remove_unsupported:
        arg_list.append('--remove-unsupported')

    if args.replace_shaders:
        arg_list.append('--replace-shaders')
        arg_list.append('{}'.format(args.replace_shaders))

    if args.save_pipeline_cache:
        arg_list.append('--save-pipeline-cache')
        arg_list.append('{}'.format(args.save_pipeline_cache))

    if args.screenshot_all:
        arg_list.append('--screenshot-all')

    if args.screenshot_ignore_FrameBoundaryANDROID:
        arg_list.append('--screenshot-ignore-FrameBoundaryANDROID')

    if args.screenshot_prefix:
        arg_list.append('--screenshot-prefix')
        arg_list.append('{}'.format(args.screenshot_prefix))

    if args.screenshot_scale:
        arg_list.append('--screenshot-scale')
        arg_list.append('{}'.format(args.screenshot_scale))

    if args.screenshot_size:
        arg_list.append('--screenshot-size')
        arg_list.append('{}'.format(args.screenshot_size))

    if args.skip_failed_allocations:
        arg_list.append('--sfa')

    if args.skip_get_fence_ranges:
        arg_list.append('--sgfr')
        arg_list.append('{}'.format(args.skip_get_fence_ranges))

    if args.skip_get_fence_status:
        arg_list.append('--sgfs')
        arg_list.append('{}'.format(args.skip_get_fence_status))

    if args.surface_index:
        arg_list.append('--surface-index')
        arg_list.append('{}'.format(args.surface_index))

    if args.swapchain:
        arg_list.append('--swapchain')
        arg_list.append('{}'.format(args.swapchain))

    if args.sync:
        arg_list.append('--sync')

    if args.use_captured_swapchain_indices:
        arg_list.append('--use-captured-swapchain-indices')

    if args.use_colorspace_fallback:
        arg_list.append('--use-colorspace-fallback')

    if args.validate:
        arg_list.append('--validate')

    if args.version:
        arg_list.append('--version')

    if args.virtual_swapchain_skip_blit:
        arg_list.append('--vssb')

    if args.wait_before_present:
        arg_list.append('--wait-before-present')

    if args.wsi:
        arg_list.append('--wsi')
        arg_list.append('{}'.format(args.wsi))

    # **WARNING** Do not manually edit the above code.
    # *********** This code was added by the generate_settings.py script using the
    # *********** settings.json input file.  Please add new settings there.
    # END REPLAY CODEGEN INSERT

    # Add the additional required arg flag
    if args.file:
        arg_list.append(args.file)
    elif not args.version:
        print('gfxrecon.py release: error: the following arguments are required: file')
        return None

    return ' '.join(arg_list)

def InstallApk(install_args):
    install_parser = CreateInstallApkParser()
    args = install_parser.parse_args(install_args)
    sdk = int(subprocess.check_output(shlex.split(adb_sdk_version)).decode())
    force_queryable = ' --force-queryable' if sdk >= 30 else ''
    cmd = adb_install + force_queryable + ' ' + args.file

    CheckDeviceSelection()

    print('Executing:', cmd)
    subprocess.check_call(shlex.split(cmd, posix='win' not in sys.platform))

def ReplayCommon(replay_args, activity):
    replay_parser = CreateReplayParser()
    args = replay_parser.parse_args(replay_args)

    extras = MakeExtrasString(args)

    CheckDeviceSelection()

    if extras:
        if args.push_file:
            cmd = ' '.join([adb_push, args.push_file, args.file])
            print('Executing:', cmd)
            subprocess.check_call(shlex.split(cmd, posix='win' not in sys.platform))

        print('Executing:', adb_stop)
        subprocess.check_call(shlex.split(adb_stop, posix='win' not in sys.platform))

        adb_start = 'adb shell am start -n {} -a {} -c {}'.format(activity, app_action, app_category)

        print(f'Executing: {adb_start} --es args \'"{extras}"\'')
        cmd = ' '.join([adb_start, '--es', 'args', '"{}"'.format(extras)])

        # Specify posix=False to prevent removal of quotes from adb extras.
        subprocess.check_call(shlex.split(cmd, posix=False))

def Replay(replay_args):
    ReplayCommon(replay_args, app_activity)

def MultiWinReplay(replay_args):
    ReplayCommon(replay_args, multiwin_app_activity)

if __name__ == '__main__':
    devices = QueryAvailableDevices()

    command_parser = CreateCommandParser()
    command = command_parser.parse_args()

    if command.command == 'install-apk':
        InstallApk(command.args)
    elif command.command == 'replay':
        Replay(command.args)
    elif command.command == 'multiwin-replay':
        MultiWinReplay(command.args)