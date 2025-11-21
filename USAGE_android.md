<!-- markdownlint-disable MD033 -->
<!-- markdownlint-disable MD041 -->
<p align="left"><img src="https://vulkan.lunarg.com/img/NewLunarGLogoBlack.png" alt="LunarG" width=263 height=113 /></p>

[![Creative Commons][1]][2]

[1]: https://i.creativecommons.org/l/by-nd/4.0/88x31.png "Creative Commons License"
[2]: https://creativecommons.org/licenses/by-nd/4.0/

Copyright &copy; 2018-2023 LunarG, Inc.

# GFXReconstruct API Capture and Replay - Android

***This document describes the GFXReconstruct software for capturing and
replaying Vulkan API calls on Android devices.***

If you are looking for capturing/replaying on a different platform, please refer
to one of these other documents:
 * [GFXReconstruct for Desktop Vulkan](./USAGE_desktop_Vulkan.md)
 * [GFXReconstruct for Desktop D3D12](./USAGE_desktop_D3D12.md)

Additional instructions for debugging the GFXReconstruct capture layer on Android can be found here:
 * [GFXReconstruct Capture layer debugging - Android](./docs/DEBUG_android.md)

## Index

1. [Capturing API Calls](#capturing-api-calls)
    1. [Before Use](#before-use)
    2. [Enabling the Capture Layer](#enabling-the-capture-layer)
    3. [Capture Options](#capture-options)
    4. [Capture Files](#capture-files)
    6. [Capture Limitations](#capture-limitations)
    7. [Troubleshooting Capturing of Applications](#troubleshooting-capturing-of-applications)
2. [Replaying API Calls](#replaying-api-calls)
    1. [Launch Script](#launch-script)
    2. [Install APK Command](#install-apk-command)
    3. [Replay Command](#replay-command)
    4. [Touch Controls](#touch-controls)
    5. [Key Controls](#key-controls)
    6. [Limitations of Replay On Android](#limitations-of-replay-on-android)
    7. [Troubleshooting Replay of Applications](#troubleshooting-replay-of-applications)
    8. [Dumping resources](#dumping-resources)
3. [Android Detailed Examples](#android-detailed-examples)


## Behavior on Android

The purpose of this section is to describe some of the software changes made to
the GFXReconstruct software to add Android support.
This section will not provide a comprehensive list of changes, but will instead
highlight some of the primary adjustments required to adapt the GFXReconstruct
software to the Android ecosystem.

### Android Writable Locations

The contents of the traces should be written to external storage on the
Android device.
The final "external storage" result varies based on Android version but some
locations that can be tried are:

 - `/sdcard/Download`
 - `/storage/emulated/0/Download`
 - `/sdcard/Android/data/${Application Full Name}`
 - `/sdcard`
 - `/mnt/shell/emulated/0`

Where `${Application Full Name}` is the full name of the application, such
as `com.khronos.vulkan_samples`.

Some devices won't allow access to those folders for certain applications.
In those cases, the following folders can be used, but will require `adb` root
access to retrieve the files:

 - `/data/data/${Application Full Name}/`
 - `/data/user/0/${Application Full Name}/`

**NOTE:** These directories may not be visible to other applications (including
gfxrecon-replay, adb pull), so any capture files will need to be copied to a
readable location with adb shell before they can be replayed.

## Capturing API Calls

The GFXReconstruct capture layer is a Vulkan layer that intercepts Vulkan API
calls made by the capture target application and logs them to a GFXReconstruct capture file.

> [!Note]
> This document uses `${Package Name}` as a placeholder for the full package name
> of a capture target application, such as com.khronos.vulkan_samples.

### Before Use

#### Android Writable Locations

The GFXReconstruct layer, and therefore the capture target application, *must*
have access to the path provided by the `debug.gfxrecon.capture_file` property.
This is typically a location within external storage. The best external storage
path varies based on application and Android version, but some recommended
locations include:

* `/sdcard/Android/data/${Package Name}` (should not require additional permissions)
* `/sdcard/Download`
* `/sdcard`

External locations outside of the app-specific directory (first list item)
require that the target capture application have additional storage permissions
as described in the [Android Data Storage](https://developer.android.com/training/data-storage/manage-all-files)
documentation.

If external storage cannot be used, internal storage may be used instead. While
these locations require no additional permissions, they may require `adb` root
access to retrieve the resulting capture files and/or copy them to a location
that the replay tool can read.

* `/data/data/${Package Name}/`
* `/data/user/0/${Package Name}/`

> [!Warning]
>Failure to provide a suitable location for the capture file will cause
>the layer to return `VK_ERROR_INITIALIZATION_FAILED` from its
>`vkCreateInstance` function and produce the following error in logcat:
>
>```text
> E gfxrecon: fopen(/sdcard/gfxrecon_capture.gfxr, wb) failed (errno = 1)
> F gfxrecon: Failed to initialize CaptureManager
>```

#### Understanding GFXReconstruct Layer Memory Capture

The Vulkan API allows Vulkan memory objects to be mapped by an application
for direct modification.
To successfully capture an application, the GFXReconstruct layer must be able to
detect if the application modifies the mapped memory in order to dump the changes
in the capture file so that they can be re-applied while replaying.
To achieve this GFXR utilizes four different modes:

##### 1. `assisted`
This mode expects the application to call `vkFlushMappedMemoryRanges`
after memory is modified; the memory ranges specified to the
`vkFlushMappedMemoryRanges` call will be written to the capture file
during the call.

##### 2. `unassisted`
This mode writes the full content of mapped memory to the capture file
on calls to `vkUnmapMemory` and `vkQueueSubmit`. It is very inefficient
for performance and it will bloat capture file sizes. May be unusable
with real-world applications that map large amounts of memory.

##### 3. `page_guard`
`page_guard` tracks modifications to individual memory pages, which are
written to the capture file on calls to `vkFlushMappedMemoryRanges`,
`vkUnmapMemory`, and `vkQueueSubmit`. This method requires allocating
shadow memory for all mapped memory. The way the changes are being tracked
varies depending on the operating system.
- On Windows `Vectored Exception Handling` mechanism is used on the shadow
memories that correspond to the mapped device memory regions.
- On Linux and Android the shadow memory regions are similarly trapped by
changing its access protection to `PROT_NONE`. Every access from the
application will generate a `SIGSEGV` which is handled by the appropriate
signal handler installed by the page guard manager.

Because a shadow memory is allocated and returned to the application instead
of the actual mapped memory returned by the driver, both reads and writes need
to be tracked.

- Writes need to be dumped to the capture file.
- Reads must trigger a memory copy from the actual mapped memory into the shadow
memory so that the application will read the correct/updated data each time.

`page_guard` is the most efficient, both performance and capture file size
wise, mechanism. However, as described in
[Conflicts With Crash Detection Libraries](#conflicts-with-crash-detection-libraries),
it has some limitation when capturing applications that install their own
signal handler for handling the `SIGSEGV` signal. This limitation exists
only on Linux and Android applications. To work around this
limitation there is the `userfaultfd` mechanism.

##### 4. `userfaultfd`
This mode utilizes the userfaultfd mechanism provided by the Linux kernel which
allows user space applications to detect and handle page faults.
Under the hood `userfaultfd` is the same mechanism as `page_guard` but instead of trapping
the shadow memory regions with the `PROT_NONE` + `SIGSEGV` trick, it
registers those memory regions for tracking to the userfaultfd mechanism.

Shadow memory regions are registered using the
`UFFDIO_REGISTER_MODE_WP | UFFDIO_REGISTER_MODE_MISSING` flags with the
userfaultfd mechanism and a handler thread is started and polls for faults
to trigger. The combination of those flags will trigger a fault in two cases:
- When an unallocated page is accessed with either a write or a read.
- When a page is written.

This imposes a limitation: When the shadow memory is freshly allocated all
pages will be unallocated, making tracking both reads and writes simple as
both will trigger a fault. However, after the first time the accesses are
tracked and dumped to the capture file, the reads cannot be tracked any longer
as the pages will be already allocated and won't trigger a fault.
To workaround this each time the memory is examined, the dirty regions are
being "reset". This involves unregistering those subregions from userfaultfd,
requesting new pages from the OS to be provided at the same virtual addresses
and then the subregions are registered again for tracking.
This has a performance penalty as in this case both reads and writes need
to be copied from the actual mapped memory into the shadow memory when
detected, while the `page_guard` method requires this only for reads.

Also there is another limitation. The way the new pages are requested each
time and the regions are unregistered and registered again, makes this
mechanism prone to race conditions when there are multiple threads. If a
thread is accessing a specific page within a region and at the same time
that region is being reset, then the access is not trapped and undefined
behavior occurs.

In order to work around this a list of the thread ids that access each
region is kept. When that specific region is being reset a signal is
sent to each thread which will force them to enter a signal handler that
GFXR registers for that signal. The signal handler essentially performs a
form of synchronization between the thread that is triggering the reset and
the rest of the threads that potentially are touching pages that are being
reset. The signal used one of the real time signals, the first in the range
[`SIGRTMIN`, `SIGRTMAX`] that has no handler already installed.

`userfaultfd` is less efficient performance wise than `page_guard` but
should be fast enough for real-world applications and games.

##### Disabling Debug Breaks Triggered by the GFXReconstruct Layer

When running an application in a debugger with the layer enabled, the
access violations triggered by the layer's memory tracking behavior may
cause the debugger to break. These debug breaks may be disabled for LLDB with
the following command:

```text
process handle SIGSEGV -n true -p true -s false
```

This command may be entered manually through the LLDB tab on Android Studio's
Debug panel.

It may also be set as a post attach command in the project configuration:

* Click on "Run" in the menu
* Choose "Edit Configurations..."
* In the dialog box, select the "Debugger" tab
* In the "Debugger" tab, select the "LLDB Post Attach Commands" tab
* Click the `+` to add the command to the command list
* Enter the `process handle SIGSEGV -n true -p true -s false` command
* Click "Apply"

### Enabling the Capture Layer

#### Install the Replay APK

The GFXReconstruct capture layer is included in the replay tool APK. Follow the [Install APK Command](#install-apk-command) instructions to make it available on the target device.

#### Enable the Layer for a Specific Application

There are 5 steps to enable the GFXReconstruct capture layer for a capture target application:

 1. Make sure the target application is debuggable
 2. Enable GPU Debug layers
 3. Indicate what app you are enabling GPU Debug on
 4. Identify the layers used to debug (in this case the layer is `VK_LAYER_LUNARG_gfxreconstruct`)
 5. Indicate what package to load debug layers from (in this case the package is `com.lunarg.gfxreconstruct.replay`)

Together, the last 4 commands look like the following:

```bash
adb shell settings put global enable_gpu_debug_layers 1
adb shell settings put global gpu_debug_app ${Package Name}
adb shell settings put global gpu_debug_layers VK_LAYER_LUNARG_gfxreconstruct
adb shell settings put global gpu_debug_layer_app com.lunarg.gfxreconstruct.replay
```

You can also restrict the layer to a specific application using these three steps:
1. Push the GFXReconstruct capture layer to `/data/local/debug/vulkan` directory.
2. Enable the global layer.
3. Set the `capture_process_name` capture option.

For example like this:
```
adb shell setprop debug.vulkan.layer.1 VK_LAYER_LUNARG_gfxreconstruct
adb shell setprop debug.gfxrecon.capture_process_name ${Package name}
```


If you attempt to capture and nothing is happening, check the `logcat` output.
A successful run of GFXReconstruct should show a message like the following:

```text
I gfxrecon: Initializing GFXReconstruct capture layer
```

### Capture Options

The GFXReconstruct layer supports the following options, which may be enabled
through Android system properties or a layer settings file.  Each Android
system property begins with the prefix `debug.gfxrecon`, and can be set
through ADB with the following command syntax:

```bash
adb shell "setprop <option> '<value>'"
```

For example, to set the log_level to "warning", specify:

```bash
adb shell "setprop debug.gfxrecon.log_level 'warning'"
```

#### Supported Options

Options with the BOOL type accept the following values:

* A case-insensitive string value 'true' or a non-zero integer value indicate true.
* A case-insensitive string value 'false' or a zero integer value indicate false.

The capture layer will generate a warning message for unrecognized or invalid
option values.

<!-- CAPTURE_SETTINGS_OPTIONS TABLE CODEGEN BEGIN -->
| Setting/Options | Property | Type | Default | Description |
| ------- | -------- | ---- | ------- | ----------- |
| Acceleration Struct Size Padding | debug.gfxrecon.accel_struct_padding | UINT | 0 | [D3D12 Only] Increase the required acceleration structure size that is reported to the application by calls to `GetRaytracingAccelerationStructurePrebuildInfo`. This can enable replay in environments with increased acceleration structure size requirements. The value should be specified as a percent of size increase. For example, a value of `5` would increase the reported acceleration structure sizes by `5%`. |
| Allow Application Compiled Pipeline Behavior | debug.gfxrecon.allow_pipeline_compile_required | BOOL | false | [Vulkan Only] The default behavior forces VK_PIPELINE_COMPILE_REQUIRED to be returned from Create*Pipelines calls which have VK_PIPELINE_CREATE_FAIL_ON_PIPELINE_COMPILE_REQUIRED_BIT set, and skips dispatching and recording the calls. This forces applications to fallback to recompiling pipelines without caching, the Vulkan calls for which will be captured. Enabling this option causes capture to record the application's calls and implementation's return values unmodified, but the resulting captures are fragile to changes in Vulkan implementations if they use pipeline caching. |
|  Capture Dump Android Assets | debug.gfxrecon.capture_android_dump_assets | BOOL | false | [Android Only] Setting this triggers a dump of all assets into the asset file. Since Android options cannot be set by the layer, dumping is done whenever this option switches between from `false` to `true` or from `true` to `false`. [Deprecated, please use 'capture_dynamic_trigger_dump_assets'] |
|  Capture Trigger For Android | debug.gfxrecon.capture_android_trigger | BOOL | false | [Android Only] Set during runtime to `true` to start capturing and to `false` to stop. If not set at all then it is disabled (non-trimmed capture). [Deprecated, please use 'capture_dynamic_trigger'] |
| Capture File Compression Format | debug.gfxrecon.capture_compression_type | ENUM | lz4 | Compression format to use with the capture file. Valid values are: LZ4, ZLIB, ZSTD, and NONE. |
| Capture Specific Draw Calls | debug.gfxrecon.capture_draw_calls | STRING | <empty> | [D3D12 Only] Specify one index or a range indices drawcalls(include dispatch) based on a ExecuteCommandList index and a CommandList index to capture. The index is 0-based. The args are one submit index, one command index, one or a range indices of draw calls, one or a range indices of bundle draw calls(option), like `0,0,0` or `0,0,0-2` or `0,0,0-2,0`. The forth arg is an option for bundle case. If the the 3rd arg is a bundle commandlist, but it doesn't set the 4th arg, it will set 0 as default. If not defined, all drawcalls are captured. |
| Capture File Name | debug.gfxrecon.capture_file | SAVE_FILE | gfxrecon_capture.gfxr | Path to use when creating the capture file. Supports variable patterns for dynamic file paths, such as `${AppName}` (the application or executable name). |
| Capture File Flush After Write | debug.gfxrecon.capture_file_flush | BOOL | false | Flush output stream after each packet is written to the capture file. |
| Capture File Name with Timestamp | debug.gfxrecon.capture_file_timestamp | BOOL | true | Add a timestamp (yyyymmddThhmmss) postfix to the capture file name. |
| Capture Specific Frames | debug.gfxrecon.capture_frames | STRING | <empty> | Specify one or more comma-separated frame ranges to capture. Each range will be written to its own file. A frame range can be specified as a single value, to specify a single frame to capture, or as two hyphenated values, to specify the first and last frame to capture. Frame ranges should be specified in ascending order and cannot overlap. Note that frame numbering is 1-based (i.e. the first frame is frame 1). Example: 200,301-305 will create two capture files, one containing a single frame and one containing five frames. If not defined, all frames are captured. |
| Capture IID_IUnknown Wrapping | debug.gfxrecon.capture_iunknown_wrapping | BOOL | false | [D3D12 Only] If GFXRECON_CAPTURE_IUNKNOWN_WRAPPING is enabled detect when applications call QueryInterface(IID_IUnknown) and switch IID_IUnknown to original IID, so that GFXR returns original interface wrapper. |
| Capture Specific Process Name | debug.gfxrecon.capture_process_name | STRING | <empty> | The name of the process to capture content from, all other processes will be ignored.  All valid API processes are captured otherwise. |
| Capture Specific GPU Queue Submits | debug.gfxrecon.capture_queue_submits | STRING | <empty> | Specify one or more comma-separated GPU queue submit call ranges to capture.  Queue submit calls are `vkQueueSubmit` for Vulkan and `ID3D12CommandQueue::ExecuteCommandLists` for DX12. Queue submit ranges work as described for `capture_frames` but on GPU queue submit calls instead of frames. The index is 0-based. Default is all queue submits are captured. |
|  Dynamic Capture Trigger | debug.gfxrecon.capture_dynamic_trigger | BOOL | false | Set during runtime to `true` to start capturing and to `false` to stop. If not set at all then it is disabled (non-trimmed capture). [Replaces 'capture_android_trigger'] |
|  Dynamic Capture Trigger Dump Assets | debug.gfxrecon.capture_dynamic_trigger_dump_assets | BOOL | false | Setting this triggers a dump of all assets into the asset file. Dumping is done whenever this option switches between from `false` to `true` or from `true` to `false`. [Replaces 'capture_android_dump_assets'] |
| Hotkey Capture Frame Limit | debug.gfxrecon.capture_trigger_frames | STRING | <empty> | Specify a limit on the number of frames to be captured via hotkey. Example: `1` will capture exactly one frame when the trigger key is pressed. If not defined, then there is no limit |
| Use Asset File | debug.gfxrecon.capture_use_asset_file | BOOL | false | When set to `true` assets (images, buffers and descriptors) will be stored separately into an asset file instead of the capture file. |
| Debug Device Lost | debug.gfxrecon.debug_device_lost | BOOL | false | [D3D12 Only] Enables automatic injection of breadcrumbs into command buffers and page fault reporting. Used to debug device removed problems. |
| Enable Debug Layer | debug.gfxrecon.debug_layer | BOOL | false | [D3D12 Only] Enable the Direct3D debug layer for Direct3D 12 application captures. |
| Disable DXR Support | debug.gfxrecon.disable_dxr | BOOL | false | [D3D12 Only] Override the result of `CheckFeatureSupport` to report the `RaytracingTier` as `D3D12_RAYTRACING_TIER_NOT_SUPPORTED`. |
| Disable MetaCommmand Support | debug.gfxrecon.disable_meta_command | BOOL | false | [D3D12 Only] Override the result of `CheckFeatureSupport` to report the D3D12_FEATURE_QUERY_META_COMMAND is invalid. Some games might use DirectStorage to improve performance, GFXR could not fully support the feature. By using this option, could bypass the feature and force game to use regular I/O to read from harddisk. |
| Force Command Serialization | debug.gfxrecon.force_command_serialization | BOOL | false | Sets exclusive locks (unique_lock) for every ApiCall. It can avoid external multi-thread to cause captured issue. |
| Force FIFO Present Mode | debug.gfxrecon.force_fifo_present_mode | BOOL | true | [Vulkan Only] When the `force_fifo_present_mode` is enabled, force all present modes in vkGetPhysicalDeviceSurfacePresentModesKHR to VK_PRESENT_MODE_FIFO_KHR, app present mode is set in vkCreateSwapchain to VK_PRESENT_MODE_FIFO_KHR. Otherwise the original present mode will be used. |
| Ignore vkFrameBoundaryANDROID | debug.gfxrecon.ignore_frame_boundary_android | BOOL | false | [Android Vulkan Only] Ignore any calls to vkFrameBoundaryANDROID when determining frame boundaries. |
| Log Apply Additional Indentation | debug.gfxrecon.log_allow_indents | BOOL | false | Apply additional indentation formatting to log messages. |
| Log Break on Error | debug.gfxrecon.log_break_on_error | BOOL | false | Trigger a debug break when logging an error. |
| Log Name and Line Number | debug.gfxrecon.log_detailed | BOOL | false | Include name and line number from the file responsible for the log message. |
| Log Errors to StdError | debug.gfxrecon.log_errors_to_stderr | BOOL | true | Log any errors that occur to stderr. |
| Log File | debug.gfxrecon.log_file | SAVE_FILE | <empty> | When set, log messages will be written to a file at the specified location. |
| Log File Overwrite | debug.gfxrecon.log_file_create_new | BOOL | true | Specifies that log file initialization should overwrite an existing file when `true`, or append to an existing file when `false`. |
| Log File Flush After Write | debug.gfxrecon.log_file_flush_after_write | BOOL | false | Flush the log file to disk after each write when true. |
| Log File Keep Open | debug.gfxrecon.log_file_keep_open | BOOL | true | Keep the log file open between log messages when true, or close and reopen the log file for each message when false. |
| Log Level | debug.gfxrecon.log_level | ENUM | info | Specify the highest level message to log.  The specified level and all levels listed after it will be enabled for logging.  For example, choosing the `warning` level will also enable the `error` and `fatal` levels. |
| Log Output to Console / stdout | debug.gfxrecon.log_output_to_console | BOOL | true | Log messages will be written to stdout. |
| Log Timestamps | debug.gfxrecon.log_timestamps | BOOL | false | Output a timestamp in front of each log message. |
| Memory Tracking Mode | debug.gfxrecon.memory_tracking_mode | ENUM | page_guard | Specifies the memory tracking mode to use for detecting modifications to mapped Vulkan memory objects. Available options are: page_guard, assisted, and unassisted. |
| Page Guard Align Buffer Sizes | debug.gfxrecon.page_guard_align_buffer_sizes | BOOL | true | [Vulkan Only] When the `page_guard` memory tracking mode is enabled, this option overrides the Vulkan API calls that report buffer memory properties to report that buffer sizes and alignments must be a multiple of the system page size. This option is intended to be used with applications that perform CPU writes and GPU writes/copies to different buffers that are bound to the same page of mapped memory, which may result in data being lost when copying pages from the page_guard shadow allocation to the real allocation. This data loss can result in visible corruption during capture. Forcing buffer sizes and alignments to a multiple of the system page size prevents multiple buffers from being bound to the same page, avoiding data loss from simultaneous CPU writes to the shadow allocation and GPU writes to the real allocation for different buffers bound to the same page. |
| Page Guard Copy on Map | debug.gfxrecon.page_guard_copy_on_map | BOOL | true | When the `page_guard` memory tracking mode is enabled, copies the content of the mapped memory to the shadow memory immediately after the memory is mapped. |
| Page Guard Persistent Memory | debug.gfxrecon.page_guard_persistent_memory | BOOL | false | When the `page_guard` memory tracking mode is enabled, this option changes the way that the shadow memory used to detect modifications to mapped memory is allocated. The default behavior is to allocate and copy the mapped memory range on map and free the allocation on unmap. When this option is enabled, an allocation with a size equal to that of the object being mapped is made once on the first map and is not freed until the object is destroyed. This option is intended to be used with applications that frequently map and unmap large memory ranges, to avoid frequent allocation and copy operations that can have a negative impact on performance. This option is ignored when `page_guard_external_memory` is enabled. |
| Page Guard Separate Read Tracking | debug.gfxrecon.page_guard_separate_read | BOOL | true | When the `page_guard` memory tracking mode is enabled, copies the content of pages accessed for read from mapped memory to shadow memory on each read. Can overwrite unprocessed shadow memory content when an application is reading from and writing to the same page. |
| Page Guard Signal Handler Watcher | debug.gfxrecon.page_guard_signal_handler_watcher | BOOL | false | When the `page_guard` memory tracking mode is enabled, setting this to `true` will spawn a thread which will will periodically reinstall the `SIGSEGV` handler if it has been replaced by the application being traced. |
| Page Guard Signal Handler Watcher Max Restores | debug.gfxrecon.page_guard_signal_handler_watcher_max_restores | INT | 1 | Sets the number of times the watcher will attempt to restore the signal handler. Setting it to a negative will make the watcher thread run indefinitely. |
| Page Guard Track AHB Memory | debug.gfxrecon.page_guard_track_ahb_memory | BOOL | false | [Android Only] When the `page_guard` memory tracking mode is enabled, eliminate the need for shadow memory allocations for Android Hardware Buffers. |
| Page Guard Unblock SIGSEGV | debug.gfxrecon.page_guard_unblock_sigsegv | BOOL | false | When the `page_guard` memory tracking mode is enabled and in the case that SIGSEGV has been marked as blocked in thread's signal mask, setting this to `true` will forcibly re-enable the signal in the thread's signal mask. |
| Queue Zero Only | debug.gfxrecon.queue_zero_only | BOOL | false | Forces to using only QueueFamilyIndex: 0 and queueCount: 1 on capturing to avoid replay error for unavailable VkQueue. |
| Quit after capturing frame ranges | debug.gfxrecon.quit_after_capture_frames | BOOL | false | Specify one or more comma-separated frame ranges to capture. Each range will be written to its own file. A frame range can be specified as a single value, to specify a single frame to capture, or as two hyphenated values, to specify the first and last frame to capture. Frame ranges should be specified in ascending order and cannot overlap. Note that frame numbering is 1-based (i.e. the first frame is frame 1). Example: 200,301-305 will create two capture files, one containing a single frame and one containing five frames. If not defined, all frames are captured. |
| Specify GPU Descriptor Handle RV Annotation | debug.gfxrecon.rv_annotation_descriptor | STRING | <empty> | [D3D12 Only] By default, the 2-byte identifier values are hard-coded. This option forces a specific identifier value to be used for annotating GPU descriptors. The value should be specified as a 2-byte hexadecimal string, e.g., `0xAB12` or `ab12`. |
| Enable Experimental RV Search | debug.gfxrecon.rv_annotation_experimental | BOOL | false | [D3D12 Only] Experimental feature to help enable replay of certain DXR/ExecuteIndirect workloads. RV annotation is a capture mode which attempts to detect when applications write Resource Values (RVs) to memory. Conceptually, RVs represent different types of GPU pointers that applications write as resource data. This capture mode writes GFXR-specific identifier values into unoccupied bits of application-facing RVs and then searches for the identifier values when the application performs a memory write. This allows GFXR to better track RV locations and eventually produce an RV-optimized capture file that may be replayed. Enabling this feature introduces performance overhead during capture, and may result in unstable capture and/or replay. |
| Specify GPU VA RV Annotation | debug.gfxrecon.rv_annotation_gpuva | STRING | <empty> | [D3D12 Only] By default, the 2-byte identifier values are hard-coded. This option forces a specific identifier value to be used for annotating GPUVAs. The value should be specified as a 2-byte hexadecimal string, e.g., `0xAB12` or `ab12`. |
| Use Random RV Annotation | debug.gfxrecon.rv_annotation_rand | BOOL | false | [D3D12 Only] By default, the 2-byte identifier values are hard-coded. This option generates random identifier values used for annotating GPUVAs and GPU descriptors. Use this if capture-time crashes are observed. |
| Screenshot Directory | debug.gfxrecon.screenshot_dir | SAVE_FOLDER | <empty> | Directory used where screenshots are saved to.  Default is the current working directory. |
| Screenshot Format | debug.gfxrecon.screenshot_format | ENUM | bmp | Image file format to use when saving screenshots. |
| Screenshot Specific Frames | debug.gfxrecon.screenshot_frames | STRING | <empty> | Specify one or more comma-separated frame ranges to screenshot. Each range will be written to its own file. A frame range can be specified as a single value, to specify a single frame to capture, or as two hyphenated values, to specify the first and last frame to capture. Frame ranges should be specified in ascending order and cannot overlap. Note that frame numbering is 1-based (i.e. the first frame is frame 1). |
| Screenshot Interval | debug.gfxrecon.screenshot_interval | INT | 1 | Sets the number between every screenshot taken.  1 means every frame is screenshot, 2 means every other, etc. Example: If screenshot range is 10-15 and interval is 2, screenshot will be generated for frames 10, 12 and 14. |
| Skip Threads With Invalid Data | debug.gfxrecon.skip_threads_with_invalid_data | BOOL | false | [OpenXR Only] When a thread is encountered which contains data that is unexpected, skip the data and mark the thread as skippable.  This is important especially in OpenXR where other API commands (such as Vulkan) may be generated inside of the OpenXR commands, but may still be referenced in some fashion outside of the OpenXR commands.  This results in issues during replay.  So, this option prevents those commands, and the threads containing those commands from being recorded to the capture file. |
<!-- CAPTURE_SETTINGS_OPTIONS TABLE CODEGEN END -->

#### Settings File

Capture options may also be specified through a layer settings file.  The layer
settings file will be loaded before the Android system properties are
processed, allowing system properties to override individual settings file
entries.

The `debug.gfxrecon.settings_path` Android system property is used to enable
a settings file:

```bash
adb shell "setprop debug.gfxrecon.settings_path /sdcard/vk_layer_settings.txt"
```

The system property may be set as either the path to the folder containing a
file named `vk_layer_settings.txt` or the full path to a file with a custom
name. When set to a folder, the capture layer will try to open a file in that
folder named `vk_layer_settings.txt`.  When set to a file, the capture layer
will try to open a file with the specified name.

The settings file may be combined with settings files for other layers. The
capture layer will ignore entries that do not start with the
'lunarg_gfxreconstruct.' prefix.

A sample layer settings file, documenting each available setting, can be found
in the GFXReconstruct GitHub repository at `layer/vk_layer_settings.txt`. Most
binary distributions of the GFXReconstruct software will also include a sample
settings file.

#### Layer Settings via VK_EXT_layer_settings

An alternative way to configure the GFXReconstruct Vulkan capture layer is via the Vulkan
`VK_EXT_layer_settings` extension, which allows settings to be passed directly through the
Vulkan API at instance creation time. This is especially useful in environments where
environment variables and settings files are not available or convenient (such as some
launchers or embedded systems).

GFXReconstruct supports reading capture options from `VkLayerSettingEXT` structures
provided in the `pNext` chain of `VkInstanceCreateInfo` when creating a Vulkan instance.
This allows you to specify settings programmatically, without relying on environment
variables or external files.

To use this feature, add a `VkLayerSettingsCreateInfoEXT` structure to the `pNext` chain
of your `VkInstanceCreateInfo`, and include settings for the
`VK_LAYER_LUNARG_gfxreconstruct` layer. For example, to set the capture file name:

```c
const char* capture_file_value[] = { "my_capture.gfxr" };

VkLayerSettingEXT capture_file_setting = {
    .pLayerName = "VK_LAYER_LUNARG_gfxreconstruct",
    .pSettingName = "capture_file",
    .type = VK_LAYER_SETTING_TYPE_STRING_EXT,
    .valueCount = 1,
    .pValues = capture_file_value,
};

VkLayerSettingsCreateInfoEXT layer_settings_info = {
    .sType = VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT,
    .pNext = NULL,
    .settingCount = 1,
    .pSettings = &capture_file_setting
};

VkInstanceCreateInfo instance_info = {
    .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
    .pNext = &layer_settings_info,
    // ... other fields ...
};
```

Supported settings include:

- `capture_file` (string): Path to use when creating the capture file (same as `GFXRECON_CAPTURE_FILE`).

#### Selecting Settings for the page_guard Memory Tracking Mode

The default settings selected for the `page_guard` memory tracking mode are the
settings that are most likely to work on a given platform, but may not provide
the best performance for all cases.

If capture performs poorly with the the default settings, try setting
`debug.gfxrecon.page_guard_persistent_memory` to `true`.

If corruption is observed during capture, try setting
`debug.gfxrecon.page_guard_align_buffer_sizes` to `true`.
If this does not help, try setting `debug.gfxrecon.page_guard_separate_read` to
`false`.

### Capture Files

Capture files are created on the first call to `vkCreateInstance`, when the
Vulkan loader loads the capture layer, and are closed on `vkDestroyInstance`,
when the last active instance is destroyed and the layer is unloaded.

If multiple instances are active concurrently, only one capture file will be
created. If multiple instances are active consecutively (i.e. an instance is
created and destroyed before the next instance is created), the creation of
each instance will generate a new file. For applications that create multiple
instances consecutively, it will be necessary to enable capture file timestamps
to prevent each new instance from overwriting the file created by the previous
instance.

If the layer fails to open the capture file, it will make the call to
`vkCreateInstance` fail, returning `VK_ERROR_INITIALIZATION_FAILED`.

#### Specifying Capture File Location

The capture file's save location can be specified by setting the
`debug.gfxrecon.capture_file` system property, described above in
the [Capture Options](#capture-options) section.

Please note that only some directories are writable.
See [Android Writable Locations](#android-writable-locations) for more info.


#### Timestamps

When capture file timestamps are enabled, a timestamp with an
[ISO 8601-based](https://en.wikipedia.org/wiki/ISO_8601)
format will be added to the name of every file created by the layer. The
timestamp is generated when the capture file is created by the layer's
`vkCreateInstance` function and is added to the base filename specified
through the `debug.gfxrecon.capture_file` system property. Timestamps have
the form:

```text
_yyyymmddThhmmss
```

where the lower-case letters stand for: Year, Month, Day, Hours, Minutes,
Seconds.  The `T` is a designator that separates the date and time components.
Time is reported for the local timezone and is specified with the 24-hour
format.

The following example shows a timestamp that was added to a file that was
originally named `gfxrecon_capture.gfxr` and was created at 2:35 PM
on November 25, 2018:
  `gfxrecon_capture_20181125T143527.gfxr`


#### Retrieving a Trace

First check the directory to make sure a trace exists:

```bash
adb shell ls ${location}
```

For example:

```bash
adb shell ls /sdcard/Download
```

If the trace exists there, for example as "gfxrecon_trace.gfxr", it can be
retrieved from the Android device using the `adb pull` command:

```bash
adb pull ${location}/${filename}
```

Such as:

```bash
adb pull /sdcard/Download/gfxrecon_trace.gfxr
```

This will download the file to the current directory.

### Trimmed Captures

Trimmed captures are created when GFXR is configured to start capturing at some later
time in execution.

To create a trimmed capture one of the trimming options can be used.
For example on android there is the `debug.gfxrecon.capture_frames` property,
which specifies the frame ranges to capture, each region generating a separate
trimmed capture file. There's also the `debug.gfxrecon.capture_android_trigger` property.
Each time the option is set accordingly, a new trimmed capture is started/stopped.

An existing capture file can be trimmed by replaying the capture with the capture layer
enabled and a trimming frame range or capture trigger enabled. (However, replay for
some content may be fast enough using the trigger property may be difficult.)

### Asset files

When doing a trimmed capture, `debug.gfxrecon.capture_use_asset_file` gives the
option to dump all assets (images, buffers and descriptors) separately in a
different capture file called the asset file. When this option is enabled
assets are tracked and only those that are changed during a tracking period
(outside of a trim range) are dumped into the asset file. This first time a
trim range is encountered (or the hotkey is pressed) all assets will be dumped,
but the next time this happens only the assets that have been changed will be
dumped. This should speed up the dumping process.

### Capture Limitations

#### Conflicts With Crash Detection Libraries

As described in
[Understanding GFXReconstruct Layer Memory Capture](#understanding-gfxreconstruct-layer-memory-capture),
the capture layer, when utilizing the `page_guard` mechanism, it uses a signal
handler to detect modifications to mapped memory.
Only one signal handler for that signal can be registered at a time, which can
lead to a potential conflict with crash detection libraries that will also
register a signal handler.

Conflict between the `page_guard` mechanism  and crash detection libraries depends on the
order with which each component registers its signal handler.
The capture layer will not register its signal handler until the first call to
`vkMapMemory`.
As long as the application initializes the crash detection library before
calling `vkMapMemory`, there should be no conflict.

The conflict occurs when the application initializes its Vulkan component and
its crash detection library concurrently.
Applications have been observed to initialize Vulkan and begin uploading
resources with one or more threads, while at the same time initializing a crash
detection library from another thread.
For this scenario, the crash detection library sets its signal handler after the
first call to `vkMapMemory`, while a resource upload thread is actively writing
to the mapped memory.
After the crash detection library sets its signal handler, it immediately
receives a SIGSEGV event generated by the concurrent write to mapped memory,
which it detects as a crash and terminates the application.

`userfaultfd` mechanism was introduced in order to work around such conflicts.

#### Memory Tracking Limitations

There is a limitation with the `page_guard` memory tracking method used by the
GFXReconstruct capture layer.
The logic behind that method is to apply a memory protection to the
guarded/shadowed regions so that accesses made by the user to trigger a
segmentation fault which is handled by GFXReconstruct.
If the access is made by a system call (like `fread()`) then there won't be a
segmentation fault generated and the function will fail.
As a result the mapped region will not be updated.

#### Android-Specific Limitations

The current version of the Android capture layer has the following limitations:

 * The capture layer must be deployed and enabled manually.
   Future work may provide a tool to automate the process of launching a
   debuggable app, injecting the trace layer with the Java Debug Wire Protocol
   (JDWP), and enabling the layer.
 * Writing files to external storage requires the external storage permission.
   If the app to be captured does not already have this permission, it can be
   granted manually through the Applications panel from device Settings or
   granted through `adb` using the `pm` command as listed later.
 * Adding a timestamp to the filename does not guarantee a unique filename.
   Some applications will query for Vulkan support by creating an instance,
   retrieving instance properties, and then destroying the instance.
   After the initial support query, the application will create a second
   instance to do its real work.
   Each of these Vulkan sessions will generate a capture file.
   If the time elapsed between these two sessions is less than a second, both
   sessions will use the same file name, with the second session overwriting the
   file produced by the first session.

### Troubleshooting Capturing of Applications

There are several steps to take to determine why a capture may not be working.
Attempt each of these and re-run after the suggested modifications to see if
the problem is resolved.

#### Enable Logcat

First, run logcat showing only messages for GFXReconstruct:

```bash
adb logcat -s gfxrecon
```

This should report messages that may be useful in determining the source of the
capture error.

An extended option would be to look for both `gfxrecon` and `vulkan` messages
using this command:

```bash
adb logcat -s gfxrecon,vulkan
```

#### Stop Your App Before Capture

Sometimes, the application could still be running in the background prior to
enabling the capture layer.
In this case, you must force stop the application and restart it in order for
the capture to work properly.

```bash
adb shell am force-stop ${Package Name}
```

##### Forcing Aligned Pages

In some cases, the application may write to buffers that are bound in the same
mapped page of memory resulting in data being lost when copying pages from the
page_guard shadow allocation to the real allocation.
This data loss can result in visible corruption during capture.

To avoid this issue when using `page_guard`, enable page aligned memory:

```bash
adb shell "setprop debug.gfxrecon.page_guard_align_buffer_sizes 'true'"
```

##### Application Write Access Issues

`debug.gfxrecon.capture_file` is the most important as the capture layer will
fail if the location isn’t writable by the target app.

See locations list in [Android Writable Locations](#android-writable-locations)
above for ideas of where to store your capture file.

If logcat output shows a line like the following:

```text
E gfxrecon: fopen(/storage/emulated/0/Download/gfxrecon_capture_20220607T132930.gfxr, wb) failed (errno = 13)
```

Then you may need to set the “Files and Media” permission on the application.
That may be possible through the application’s settings in the Android
interface, or you can try “granting” the permission through the `pm` command:

```bash
adb shell pm grant ${Package Name} android.permission.WRITE_EXTERNAL_STORAGE
```

Refer to the other settings in [Capture Options](#capture-options).


## Replaying API Calls

### Launch Script

The `gfxrecon.py` script, located in android/scripts directory of the
[gfxreconstruct git repository](https://github.com/LunarG/gfxreconstruct)
is provided as a convenient method for deploying and launching the Android replay tool. The script
currently supports the following commands:

| Command     | Description                                                                       |
| ----------- | --------------------------------------------------------------------------------- |
| install-apk | Install the specified APK file with the `-g -t -r --force-queryable` options.     |
| replay      | Launch the replay tool with the specified command line options.                   |

### Install APK Command

The `gfxrecon.py install-apk` command has the following usage:

```text
usage: gfxrecon.py install-apk [-h] <file>

positional arguments:
  file        APK file to install

optional arguments:
  -h, --help  show this help message and exit
```

The command is equivalent to:

```bash
adb install -g -t -r --force-queryable <file>
```

The install-apk option of the `gfxrecon.py` script with the install-apk option is
is a convenient way to install the gfxrecon replay tool.

For example, starting at the root directory of the GFXReconstruct repo after
building, install the replay tool using the following commands:

```bash
pushd android
python scripts/gfxrecon.py install-apk tools/replay/build/outputs/apk/debug/replay-debug.apk
popd
```

#### Additional Permissions

##### Android 10

For replay devices running Android 10, the replay tool now requires the enabling
of legacy storage access:

```bash
adb shell appops set com.lunarg.gfxreconstruct.replay android:legacy_storage allow
```

##### Android 11 and Newer

For replay devices running Android 11 and newer, the replay tool requires that
the Android permission for `MANAGE_EXTERNAL_STORAGE` be granted either through
the following `adb` command or by clicking on the permission dialog when it
opens up:

```bash
adb shell appops set com.lunarg.gfxreconstruct.replay MANAGE_EXTERNAL_STORAGE allow
```

In addition to the storage permissions, Android 11 requires the app be installed
with the `--force-queryable` flag to make it visible to other applications when
using the the `gpu_debug_layer_app` Android system setting. This should be
handled automatically by the `gfxrecon.py install-apk` command above. On some
devices it may be necessary to run the install command _twice_ in order for the
queryable permission to apply.

### Replay Command

The `gfxrecon.py replay` command is used to perform a replay of a capture file.

The `gfxrecon.py replay` command has the following usage:

```text
usage: gfxrecon.py replay <options> <file>
```

With `<file>` being the name of the required input capture file.

`<options>` can be any of the following:

<!-- REPLAY_SETTINGS_OPTIONS TABLE CODEGEN BEGIN -->
| Command-Line Argument | Required | Description | Default | Valid for APIs |
| --------------------- | -------- | ----------- | ------- | -------------- |
| --log-file &lt;log_file&gt; | Optional | When set, log messages will be written to a file at the specified location. | <empty> | ALL |
| --log-level &lt;level&gt; | Optional | Specify the highest level message to log.  The specified level and all levels listed after it will be enabled for logging.  For example, choosing the `warning` level will also enable the `error` and `fatal` levels. <level> may be the following: <ul>    <li>`debug`  :  Messages  with debug-severity and  higher </li>    <li>`info`  :  Messages  with info-severity and  higher </li>    <li>`warning`  :  Messages  with warning-severity and  higher </li>    <li>`error`  :  Only  Error/Fatal messages </li>    <li>`fatal`  :  Only Fatal  messages </li> </ul> | info | ALL |
| --log-timestamps | Optional | Output a timestamp in front of each log message. | false | ALL |
| --screenshot-dir &lt;dir&gt; | Optional | Directory used where screenshots are saved to.  Default is the current working directory. | <empty> | ALL |
| --screenshot-format &lt;format&gt; | Optional | Image file format to use when saving screenshots. <format> may be the following: <ul>    <li>`bmp`  :  Bitmap File  Format. </li>    <li>`png`  :  Portable  Network Graphics Format. </li> </ul> | bmp | ALL |
| --screenshots &lt;range&gt; | Optional | Generate screenshots for the specified frames. Target frames are specified as a comma separated list of frame ranges.  A frame range can be specified as a single value, to specify a single frame, or as two hyphenated values, to specify the first and last frames to process.  Frame ranges should be specified in ascending order and cannot overlap.  Note that frame numbering is 1-based (i.e. the first frame is frame 1). Example: 200,301-305 will generate six screenshots. | <empty> | ALL |
| --screenshot-interval &lt;N&gt; | Optional | Sets the number between every screenshot taken.  1 means every frame is screenshot, 2 means every other, etc. Example: If screenshot range is 10-15 and interval is 2, screenshot will be generated for frames 10, 12 and 14. | 1 | ALL |
| --add-new-pipeline-caches | Optional | If set, allows gfxreconstruct to create new vkPipelineCache objects when it encounters a pipeline created without cache. This option can be used in coordination with `--save-pipeline-cache` and `--load-pipeline-cache`. | false | ALL |
| --capture | Optional | Capture the replaying GFXR file. Capture uses the same log options as replay. All other capture option behavior and usage is the same as when capturing with the GFXR layer. The capture functionality is included in the `gfxrecon-replay` executable -- no GFXR capture layer is added to the Vulkan layer chain. | false | ALL |
| --cpu-mask &lt;binary-mask&gt; | Optional | Set of CPU cores used by the replayer. `binary-mask` is a succession of '0' and '1' read from left to right that specifies used/unused cores. For example '10010' activates the first and fourth cores and deactivate all other cores. If the option is not set, all cores can be used. If the option is set only for some cores, the other cores are not used. | <empty> | ALL |
| --debug-messenger-level &lt;level&gt; | Optional | Specify highest debug messenger severity level. Options are: debug, info, warning, and error. <level> may be the following: <ul>    <li>`debug`  :  Messages  with debug-severity and  higher </li>    <li>`info`  :  Messages  with info-severity and  higher </li>    <li>`warning`  :  Messages  with warning-severity and  higher </li>    <li>`error`  :  Only  Error/Fatal messages </li>    <li>`fatal`  :  Only Fatal  messages </li> </ul> | warning | ALL |
| --deduplicate-device | Optional | If set, at most one VkDevice will be created for each VkPhysicalDevice for RenderDoc and DXVK case. | false | ALL |
| --dump-resources &lt;args&gt; | Optional | Dump out resources during replay <args> may be the following: <ul>    <li>`<submit-index,command-index,draw-call-index>`  :   The capture file will be   examined, and   &lt;submit-index,command-index,draw-call-index&gt;   will be used to dump   resources for the given   submit, command and draw   indices. </li>    <li>`<filename>.json`  :   Extract `--dump-resources`   block indices args from the   specified json file. The   format for the json file is   documented in detail in the   gfxreconstruct   documentation. </li> </ul> | <empty> | ALL |
| --dump-resources-before-draw | Optional | In addition to dumping GPU resources after the draw calls specified by the `--dump-resources` argument, also dump resources before the draw calls. | false | ALL |
| --dump-resources-binary-file-compression-type &lt;compression-format&gt; | Optional | [Vulkan Only] Compress files that are dumped as binary.  <compression-format> may be the following: <ul>    <li>`lz4`  :  Fastest  compression algorithm that  favors performance over  compression ratio. </li>    <li>`zlib`  :  Widely used  compression algorithm with  better compression ratio  than LZ4 but worse  performance. </li>    <li>`zstd`  :  Newer  compression algorithm with  better compression ratio and  performance than ZLIB,  though not as fast as LZ4. </li>    <li>`none`  :  No  compression (Default) </li> </ul> | none | VULKAN |
| --dump-resources-dir &lt;dir&gt; | Optional | Directory to write dump resources output files. Default is the current working directory. | <empty> | ALL |
| --dump-resources-dump-all-image-subresources | Optional | Dump all available mip levels and layers when dumping images. | false | ALL |
| --dump-resources-dump-build-acceleration-structures-input-buffers | Optional | Dump all input buffers used in vkCmdBuildAccelerationStructures. This includes vertex, index, transformation matrix, AABB and instance buffers. Default is off. | false | ALL |
| --dump-resources-dump-color-attachment-index &lt;index&gt; | Optional | Configures which color attachment to dump when dumping draw calls.  Default is all attachments. | -1 | ALL |
| --dump-resources-dump-depth-attachment | Optional | Configures whether to dump the depth attachment of draw calls. Default is false. | false | ALL |
| --dump-resources-dump-raw-images | Optional | Dump the images in raw format (without conversion), otherwise they will be transformed into a VK_FORMAT_B8G8R8A8_* format, more suitable for dumping in an image file. | false | ALL |
| --dump-resources-dump-separate-alpha | Optional | Dump the image alpha as a separate file from the main image. | false | ALL |
| --dump-resources-dump-unused-vertex-bindings | Optional | Dump a vertex binding even if no vertex attributes references it. | false | ALL |
| --dump-resources-dump-vertex-index-buffers | Optional | Enable dumping of vertex and index buffers while dumping draw call resources. Default is disabled. | false | ALL |
| --dump-resources-image-format &lt;format&gt; | Optional | Image file format to use when dumping image resources. Available formats are: bmp, png. <format> may be the following: <ul>    <li>`bmp`  :  Bitmap File  Format. </li>    <li>`png`  :  Portable  Network Graphics Format. </li> </ul> | bmp | ALL |
| --dump-resources-json-output-per-command | Optional | Enable storing a json output file for each dumped command. Overrides default behavior which is generating one output json file that contains the information for all dumped commands. | false | ALL |
| --dump-resources-scale &lt;scale&gt; | Optional | Scale images generated by dump resources by the given scale factor. The scale factor must be a floating point number greater than 0. Values greater than 10 are capped at 10. Default value is 1.0. | 1.0 | ALL |
| --flush-inside-measurement-range | Optional | If this is specified the replayer will flush and wait for all current GPU work to finish at the end of each frame inside the measurement range. | false | ALL |
| --fmr\|--flush-measurement-range | Optional | If this is specified the replayer will flush and wait for all current GPU work to finish at the start and end of the measurement range. | false | ALL |
| --gpu &lt;index&gt; | Optional | Use the specified device for replay, where index is the zero-based index to the array of physical devices returned by vkEnumeratePhysicalDevices or IDXGIFactory1::EnumAdapters1. Replay may fail if the specified device is not compatible with the original capture devices. | <empty> | ALL |
| --gpu-group &lt;index&gt; | Optional | Use the specified device group for replay, where index is the zero-based index to the array of physical device group returned by vkEnumeratePhysicalDeviceGroups. Replay may fail if the specified device group is not compatible with the original capture device group. | <empty> | ALL |
| -h\|--help | Optional | Print usage information and exit | false | ALL |
| --input-capture &lt;input_file&gt; | Optional | The name of the capture file to use for processing.  This overrides the non-tagged input filename that used to appear after all optional arguments | <empty> | ALL |
| --load-pipeline-cache &lt;cache-file&gt; | Optional | If set, loads data created by the `--save-pipeline-cache` option in <cache-file> and uses it to create the pipelines instead of the pipeline caches saved at capture time. | <empty> | ALL |
| --log-debugview | Optional | Log messages with OutputDebugStringA. | false | ALL |
| --measurement-file &lt;file&gt; | Optional | Write measurements to a file at the specified path. Default is: '/sdcard/gfxrecon-measurements.json' on Android and'./gfxrecon-measurements.json' on desktop. | gfxrecon-measurement.gfxr | ALL |
| --mfr\|--measurement-frame-range &lt;start_frame&gt;-&lt;end_frame&gt; | Optional | Custom framerange to measure FPS for. This range will include the start frame but not the end frame. The measurement frame range defaults to all frames except the loading frame but can be configured for any range. If the end frame is past the last frame in the trace it will be clamped to the frame after the last (so in that case the results would include the last frame). | <empty> | ALL |
| -m\|--memory-translation &lt;mode&gt; | Optional | Enable memory translation for replay on GPUs with memory types that are not compatible with the capture GPU's memory types <mode> may be the following: <ul>    <li>`none`  :  No memory  translation is performed  (Default). </li>    <li>`remap`  :  Attempt to  mapcapture memory types to  compatible replay memory  types, without altering  memory allocation behavior. </li>    <li>`realign`  :  Adjust  memory allocation sizes and  resource binding offsets  based on replay memory  properties. </li>    <li>`rebind`  :  Change  memory allocation behavior  based on resource usage and  replay memory properties.   Resources may be bound to  different allocations with  different offsets.  Uses VMA  tomanage allocations and  suballocations. </li> </ul> | none | ALL |
| --offscreen-swapchain-frame-boundary | Optional | Should only be used with offscreen swapchain. Activates the extension VK_EXT_frame_boundary (always supported if trimming, checks for driver support otherwise) and inserts command buffer submission with VkFrameBoundaryEXT where vkQueuePresentKHR was called in the original capture.  This allows preserving frames when capturing a replay that uses offscreen swapchain. | false | ALL |
| --onhb\|--omit-null-hardware-buffers | Optional | [Android Only] Omit calls that use a NULL AHardwareBuffer | false | ALL |
| --opcd\|--omit-pipeline-cache-data | Optional | Omit pipeline cache data from calls to vkCreatePipelineCache and skip calls to vkGetPipelineCacheData | false | ALL |
| --pause-frame &lt;frame&gt; | Optional | Pause after replay frame number <frame> | -1 | ALL |
| --paused | Optional | Pause after replaying the first frame (same as `--pause-frame 1`). | false | ALL |
| --pbi-all | Optional | Print all block information. | false | ALL |
| --pbis &lt;index1,index2&gt; | Optional | Print block information between block index1 and block index2. | <empty> | ALL |
| --pcj\|--pipeline-creation-jobs &lt;num_jobs&gt; | Optional | Specify the number of asynchronous pipeline-creation jobs as integer. If <num_jobs> is negative it will be added to the number of cpu-cores Default: 0 (do not use asynchronous operations). | 0 | ALL |
| --preload-measurement-range | Optional | Preload the provided measurement range of frames. | false | ALL |
| --present-mode &lt;mode&gt; | Optional | [Vulkan Only] Override the VkPresentModeKHR of the swapchain value during replay <mode> may be the following: <ul>    <li>`capture`  :  Use  whatever present mode the  original application used. </li>    <li>`immediate`  :  Force  VK_PRESENT_MODE_IMMEDIATE_KHR </li>    <li>`mailbox`  :  Force  VK_PRESENT_MODE_MAILBOX_KHR </li>    <li>`fifo`  :  Force  VK_PRESENT_MODE_FIFO_KHR </li>    <li>`fifo_relaxed`  :   Force  VK_PRESENT_MODE_FIFO_RELAXED_KHR </li> </ul> | capture | VULKAN |
| --quit-after-frame &lt;frame&gt; | Optional | Quit replay after frame <frame> has completed. | <empty> | ALL |
| --qamr\|--quit-after-measurement-range | Optional | If this is specified the replayer will abort when it reaches the <end_frame> specified in the `--measurement-frame-range` argument. | false | ALL |
| --remove-unsupported | Optional | Remove unsupported extensions and features from instance and device creation parameters. | false | ALL |
| --replace-shaders &lt;dir&gt; | Optional | Replace the shader code in each CreateShaderModule with the contents of the file <dir>/sh<handle_id> if found, where <handle_id> is the handle id of the CreateShaderModule call. See gfxrecon-extract. | <empty> | ALL |
| --save-pipeline-cache &lt;cache-file&gt; | Optional | If set, produces pipeline caches at replay time instead of using the one saved at capture time and save those caches in <cache-file> | <empty> | ALL |
| --screenshot-all | Optional | Generate screenshots for all frames.  When this option is specified, other screenshot frame-related options are ignored. | false | ALL |
| --screenshot-ignore-FrameBoundaryANDROID | Optional | If set, frames switched with vkFrameBoundANDROID will be ignored from the screenshot handler. | false | ALL |
| --screenshot-prefix &lt;file-prefix&gt; | Optional | Prefix to apply to the screenshot file name.  Default is 'screenshot', producing file names similar to 'screenshot_frame_8049.bmp' | screenshot | ALL |
| --screenshot-scale &lt;scale&gt; | Optional | Specify a decimal factor which will determine screenshot sizes. The factor will be multiplied with the swapchain images dimension to determine the screenshot dimensions. Default value is 1.0. | 1.0 | ALL |
| --screenshot-size &lt;width&gt;x&lt;height&gt; | Optional | Specify desired screenshot dimensions. Leaving this unspecified screenshots will use the swapchain images dimensions. If `--screenshot-scale` is also specified then this option is ignored. | <empty> | ALL |
| --sfa\|--skip-failed-allocations | Optional | Skip vkAllocateMemory, vkAllocateCommandBuffers, and vkAllocateDescriptorSets calls that failed during capture. | false | ALL |
| --sgfr\|--skip-get-fence-ranges &lt;frame-ranges&gt; | Optional | [Vulkan Only] Frame ranges where `--sgfs` applies. The format is: <frame-start-1>-<frame-end-1>[,<frame-start-1>-<frame-end-1>]* | <empty> | VULKAN |
| --sgfs\|--skip-get-fence-status &lt;status&gt; | Optional | [Vulkan Only] Specify behaviour to skip calls to vkWaitForFences and vkGetFenceStatus. <status> = 0: Don't skip, <status> = 1: Skip unsuccessful calls, <status> = 2: Always Skip. If no skip frame range is specified (through `--sgfr`), the status applies to all frames. Skip vkGetFenceStatus calls that return a provided status | 0 | VULKAN |
| --surface-index &lt;N&gt; | Optional | [Vulkan Only] Restrict rendering to the Nth surface object created. Used with captures that include multiple surfaces.  Default is -1 (render to all surfaces). | -1 | VULKAN |
| --swapchain &lt;mode&gt; | Optional | Choose a swapchain mode to replay. <mode> may be the following: <ul>    <li>`captured`  :  Use the  swapchain indices stored in  thecapture directly on the  swapchain setup for replay. </li>    <li>`offscreen`  :   Disable creating swapchains,  surfaces and windows. To see  rendering, add the  --screenshots option. </li>    <li>`virtual`  :  Virtual  Swapchain of images which  match the swapchain in  effect at capture time and  which are copied to the  underlying swapchain of the  implementation being  replayed on (Default). </li> </ul> | virtual | ALL |
| --sync | Optional | Synchronize after each queue submission with vkQueueWaitIdle. | false | ALL |
| --use-captured-swapchain-indices | Optional | Same as `--swapchain captured`.  Ignored if the `--swapchain` option is used. | false | ALL |
| --use-colorspace-fallback | Optional | Swap the swapchain color space if unsupported by replay device. Check if color space is not supported by replay device and fallback to VK_COLOR_SPACE_SRGB_NONLINEAR_KHR. | false | ALL |
| --validate | Optional | Enable the Khronos Vulkan validation layer when replaying a Vulkan capture or the Direct3D debug layer when replaying a Direct3D 12 capture. | false | ALL |
| --version | Optional | Print version information and exit. | false | ALL |
| --vssb\|--virtual-swapchain-skip-blit | Optional | If using Virtual Swapchain, skip the actual BLTs to the actual HW surface to gain performance during replay. | false | ALL |
| --wait-before-present | Optional | Force wait on completion of queue operations for all queues before calling Present. This is needed for accurate acquisition of instrumentation data on some platforms. | false | ALL |
| --wsi &lt;platform&gt; | Optional | Force replay to use the specified wsi platform. <platform> may be the following: <ul>    <li>`auto`  :   Automatically attempt to  determine WSI used based on  platform (Default). </li>    <li>`win32`  :  Force  WIN32 WSI type WSI (Valid on  Windows). </li>    <li>`xlib`  :  Force using  XLIB WSI (Valid on Linux). </li>    <li>`xcb`  :  Force using  XCBWSI (Valid on Linux). </li>    <li>`wayland`  :  Force  using Wayland WSI (Valid on  Linux). </li>    <li>`display`  :  Force  using Display WSI. </li>    <li>`headless`  :  Force  using headless WSI. </li> </ul> | auto | ALL |
<!-- REPLAY_SETTINGS_OPTIONS TABLE CODEGEN END -->

The command will force-stop an active replay process before starting the replay
activity with the following:

```bash
adb shell am force-stop com.lunarg.gfxreconstruct.replay
adb shell am start -n "com.lunarg.gfxreconstruct.replay/android.app.NativeActivity" \
                   -a android.intent.action.MAIN \
                   -c android.intent.category.LAUNCHER \
                   --es args \
                   '"<arg-list>"'
```

If `gfxrecon-replay` was built with Vulkan Validation Layer support,
`VK_LAYER_KHRONOS_validation` can be enabled and disabled in the same manner as
`VK_LAYER_LUNARG_gfxreconstruct`


#### Using the Replay Command

To replay a captured trace, find the location of the trace and replay it using
the built `gfxrecon-replay` tool:

```bash
./android/scripts/gfxrecon.py replay ${capture file path}
```

For example, if the capture was saved to the `/sdcard/Download` folder as
"gfxrecon_capture_20221210T120558.gfxr", the command would look like:

```bash
./android/scripts/gfxrecon.py replay /sdcard/Download/gfxrecon_capture_20221210T120558.gfxr
```

##### Pushing a Replay File from the Host

The Android GFXReconstruct replay script allows pushing a file from the host
to the Android device using the `-p` or `--push-file` option.

If a capture was pulled down onto the host, it could be pushed up to the Android
device using the following:

```bash
./android/scripts/gfxrecon.py replay --push-file ${HOST_FILE} ${RESULTING_ANDROID_FILE}
```

For example, if the capture file was found at
`/home/user/test/android_capture.gfxr` on the local host, and needed to be
pushed up to the Android device into the `/sdcard/Download` folder, then the
command would be:

```bash
./android/scripts/gfxrecon.py replay --push-file /home/user/test/android_capture.gfxr /sdcard/Download/android_capture.gfxr
```

This would result in the `gfxrecon.py` Python script first pushing up the file
to the requested location and then starting the replay.


### Touch Controls

The `gfxrecon-replay` tool for Android supports the following touch controls:

| Key(s)     | Action                                 |
| ---------- | -------------------------------------- |
| Tap        | Toggle pause/play.                     |
| Swipe left | Advance to the next frame when paused. |

### Key Controls

The `gfxrecon-replay` tool for Android supports the following key controls. Key
events can be simulated through `adb` with the
`adb shell input keyevent <key-code>` command:

| Key(s)         | Key code(s)              | Action                                 |
| -------------- | ------------------------ | -------------------------------------- |
| Space, p       | Space = 62, p = 44       | Toggle pause/play.                     |
| D-pad right, n | D-pad right = 22, n = 42 | Advance to the next frame when paused. |


### Limitations of Replay On Android

The current version of the Android replay layer has the following limitations:

  * Creation of more than one swapchain at a time is not guaranteed to work.
    The replay tool has a single `ANativeActivity` with a single
    `ANativeWindow`.
    Android may enforce a limitation of one Vulkan surface per-window.
     * **NOTE**:The `--surface-index` option may improve the chances of
       GFXReconstruct properly capturing the necessary content from the correct
       surface.
  * Permission to access external storage must be granted manually.

### Troubleshooting Replay of Applications

There are several steps to take to determine why a replay of a capture may not
be working.
Attempt each of these and re-run after the suggested modifications to see if
the problem is resolved.

#### Removing Unsupported Extensions

When capturing an application, the GFXReconstruct layer exposes the
`VK_EXT_tooling_info` extension.
If the application or OS uses this extension, it will be stored in the capture.
However, when replaying the capture, the extension may not be exposed by other
Vulkan components on that system and may fail in`vkCreateDevice` with
`VK_ERROR_EXTENSION_NOT_PRESENT`.
This is just one example of a scenario where an extension is present during
the capture process, but not during the replay later on.

To remedy this problem, use the `--remove-unsupported` argument to the
`gfxrecon.py replay` command.

For example (modifying the example above):

```bash
./android/scripts/gfxrecon.py replay --remove-unsupported /sdcard/Download/gfxrecon_capture_20221210T120558.gfxr
```

#### Virtual Swapchain

During replay, swapchain indices for present can be different from captured
indices.
Causes for this can include the swapchain image count differing between capture
and replay, and `vkAcquireNextImageKHR` returning a different `pImageIndex` at
replay to the one that was captured.
These issues can cause unexpected rendering or even crashes.

Virtual Swapchain insulates higher layers in the Vulkan stack from these
problems by creating a set of images, exactly matching the swapchain
configuration at capture time, which it exposes for them to render into.
Before a present, it copies the virtual image to a target swapchain image for
display.
Since this issue can happen in many situations, virtual swapchain is the default
setup.
If the user wants to bypass the feature and use the captured indices to present
directly on the swapchain of the replay implementation, they should add the
`--use-captured-swapchain-indices` option when invoking `gfxrecon-replay`.

#### Dumping resources

GFXReconstruct offers the capability to dump resources when replaying a capture file. Detailed documentation of that feature can be found in [vulkan_dump_resources.md](./vulkan_dump_resources.md)

## Android Detailed Examples

For more information and detailed examples on using GFXReconstruct on Android
can be found in the [HOWTO_android.md](./HOWTO_android.md) document.