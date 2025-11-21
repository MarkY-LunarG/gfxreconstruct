<!-- markdownlint-disable MD033 -->
<!-- markdownlint-disable MD041 -->

<p align="left"><img src="https://vulkan.lunarg.com/img/NewLunarGLogoBlack.png" alt="LunarG" width=263 height=113 /></p>

[![Creative Commons][1]][2]

[1]: https://i.creativecommons.org/l/by-nd/4.0/88x31.png "Creative Commons License"
[2]: https://creativecommons.org/licenses/by-nd/4.0/

Copyright &copy; 2022-2023 LunarG, Inc.

Copyright &copy; 2022-2023 Advanced Micro Devices, Inc. All rights reserved.

# GFXReconstruct API Capture and Replay - D3D12

***This document describes the GFXReconstruct software for capturing and
replaying D3D12 API calls on Windows Desktop systems.***

If you are looking for capturing/replaying on a different platform, please refer
to one of these other documents:
 * [GfxReconstruct for Android Vulkan](./USAGE_android.md)
 * [GfxReconstruct for Desktop Vulkan](./USAGE_desktop_Vulkan.md)


## Index

1. [Capturing API calls](#capturing-api-calls)
    1. [Enabling D3D12 Capture](#enabling-d3d12-capture)
    2. [Capture Options](#capture-options)
    3. [Capture Files](#capture-files)
2. [Replaying API Calls](#replaying-api-calls)
    1. [Command Line Arguments](#command-line-arguments)
    2. [Keyboard Controls](#keyboard-controls)
    3. [Capturing Replay (Recapture)](#capturing-replay-recapture)
3. [Other Capture File Processing Tools](#other-capture-file-processing-tools)
    1. [Capture File Info](#capture-file-info)
    2. [Capture File Compression](#capture-file-compression)
    3. [Capture File Optimizer](#capture-file-optimizer)
    4. [Renaming Scripts](#renaming-scripts)
4. [AMD GPU Services Support](#ags-support)
    1. [How to Capture AGS](#how-to-capture-ags)
    2. [How to Process AGS Capture Files](#how-to-process-ags-capture-files)




## Capturing API calls

The GFXRconstruct capture process leans on a DLL substitution mechanism to intercept D3D12 API calls. When an application loads the D3D12 or DXGI runtimes, it loads the GFXReconstruct capture libraries instead, which provides a means of API call interception. From that point on, GFXReconstruct is able record D3D12 API calls to a capture file and also forward them to the real runtimes.

### Enabling D3D12 Capture

First, these components must be identified:

- The path to the application executable
- The GFXReconstruct capture libraries:
  - `d3d12.dll`
  - `dxgi.dll`
  - `d3d12_capture.dll`

These capture libraries must be placed beside the application executable. For example, if we have the following application:

```bash
C:\AppPath\d3d12_app.exe
```

We must populate its folder as follows:

```bash
C:\AppPath\d3d12_app.exe
C:\AppPath\d3d12.dll
C:\AppPath\dxgi.dll
C:\AppPath\d3d12_capture.dll
```

It is important to remove these capture libraries once capture is complete. Otherwise, subsequent runs of the application will also be captured.

If there is a specialized need to have the GFXReconstruct capture libraries call into a custom D3D12/DXGI runtime, then the custom runtime must be named either `d3d12_ms.dll` or `dxgi_ms.dll`. It must also be placed beside the application executable.

### Capture Options

Capture-time settings may be changed by setting `GFXRECON_*` environment variables.

If no options are set, GFXReconstruct will apply default settings and always record from the first frame up until application termination.

The following example demonstrates how to use the Windows Command Prompt to set the log level to "warning" and then launch the application. Running these commands will produce a .gfxr capture file in the current working directory.

```bat
set GFXRECON_LOG_LEVEL=warning
C:\AppPath\d3d12_app.exe
```

Note that some applications are started using a launcher (i.e. gaming services/platforms). In this case, GFXReconstruct's capture options should also be set in a Windows Command Prompt, and the launcher needs to be started from that **same** prompt. Then any child applications that are started by that launcher will inherit GFXReconstruct's capture options which were set in the prompt.

##### Supported Options

Options with the BOOL type accept the following values:

* A case-insensitive string value 'true' or a non-zero integer value indicate true.
* A case-insensitive string value 'false' or a zero integer value indicate false.

The capture layer will generate a warning message for unrecognized or invalid option values.

<!-- CAPTURE_SETTINGS_OPTIONS TABLE CODEGEN BEGIN -->
| Setting/Options | Environment Variable | Type | Default | Description |
| ------- | -------- | ---- | ------- | ----------- |
| Acceleration Struct Size Padding | GFXRECON_ACCEL_STRUCT_PADDING | UINT | 0 | [D3D12 Only] Increase the required acceleration structure size that is reported to the application by calls to `GetRaytracingAccelerationStructurePrebuildInfo`. This can enable replay in environments with increased acceleration structure size requirements. The value should be specified as a percent of size increase. For example, a value of `5` would increase the reported acceleration structure sizes by `5%`. |
| Capture File Compression Format | GFXRECON_CAPTURE_COMPRESSION_TYPE | STRING with possible values of: "LZ4", "ZLIB", "ZSTD", "NONE" | LZ4 | Compression format to use with the capture file. Valid values are: LZ4, ZLIB, ZSTD, and NONE.  may be the following: <ul>    <li>`lz4`  :  Fastest  compression algorithm that  favors performance over  compression ratio. </li>    <li>`zlib`  :  Widely used  compression algorithm with  better compression ratio  than LZ4 but worse  performance. </li>    <li>`zstd`  :  Newer  compression algorithm with  better compression ratio and  performance than ZLIB,  though not as fast as LZ4. </li>    <li>`none`  :  No  compression </li> </ul> |
| Capture Environment | GFXRECON_CAPTURE_ENVIRONMENT | LIST | <empty> | Comma delimited list of environment variables to store in the capture file. These can optionally be restored during replay to their capture-time values with the `gfxrecon-replay-renamed.py` utility. |
| Capture Specific Draw Calls | GFXRECON_CAPTURE_DRAW_CALLS | STRING | <empty> | [D3D12 Only] Specify one index or a range indices drawcalls(include dispatch) based on a ExecuteCommandList index and a CommandList index to capture. The index is 0-based. The args are one submit index, one command index, one or a range indices of draw calls, one or a range indices of bundle draw calls(option), like `0,0,0` or `0,0,0-2` or `0,0,0-2,0`. The forth arg is an option for bundle case. If the the 3rd arg is a bundle commandlist, but it doesn't set the 4th arg, it will set 0 as default. If not defined, all drawcalls are captured. |
| Capture File Name | GFXRECON_CAPTURE_FILE | STRING (filter = *.gfxr) | gfxrecon_capture.gfxr | Path to use when creating the capture file. Supports variable patterns for dynamic file paths, such as `${AppName}` (the application or executable name). |
| Capture File Flush After Write | GFXRECON_CAPTURE_FILE_FLUSH | BOOL | false | Flush output stream after each packet is written to the capture file. |
| Capture File Name with Timestamp | GFXRECON_CAPTURE_FILE_TIMESTAMP | BOOL | true | Add a timestamp (yyyymmddThhmmss) postfix to the capture file name. |
| Capture Specific Frames | GFXRECON_CAPTURE_FRAMES | STRING | <empty> | Specify one or more comma-separated frame ranges to capture. Each range will be written to its own file. A frame range can be specified as a single value, to specify a single frame to capture, or as two hyphenated values, to specify the first and last frame to capture. Frame ranges should be specified in ascending order and cannot overlap. Note that frame numbering is 1-based (i.e. the first frame is frame 1). Example: 200,301-305 will create two capture files, one containing a single frame and one containing five frames. If not defined, all frames are captured. |
| Capture IID_IUnknown Wrapping | GFXRECON_CAPTURE_IUNKNOWN_WRAPPING | BOOL | false | [D3D12 Only] If GFXRECON_CAPTURE_IUNKNOWN_WRAPPING is enabled detect when applications call QueryInterface(IID_IUnknown) and switch IID_IUnknown to original IID, so that GFXR returns original interface wrapper. |
| Capture Specific Process Name | GFXRECON_CAPTURE_PROCESS_NAME | STRING | <empty> | The name of the process to capture content from, all other processes will be ignored.  All valid API processes are captured otherwise. |
| Capture Specific GPU Queue Submits | GFXRECON_CAPTURE_QUEUE_SUBMITS | STRING | <empty> | Specify one or more comma-separated GPU queue submit call ranges to capture.  Queue submit calls are `vkQueueSubmit` for Vulkan and `ID3D12CommandQueue::ExecuteCommandLists` for DX12. Queue submit ranges work as described for `capture_frames` but on GPU queue submit calls instead of frames. The index is 0-based. Default is all queue submits are captured. |
| Hotkey Capture Trigger | GFXRECON_CAPTURE_TRIGGER | STRING with possible values of: "", "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12", "TAB", "CONTROL" | <empty> | Specify a hotkey (any one of F1-F12, TAB, CONTROL) that will be used to start/stop capture. Example: F3 will set the capture trigger to F3 hotkey. One capture file will be generated for each pair of start/stop hotkey presses.   This, in effect, toggles `capture_dynamic_trigger`.  may be the following: <ul>    <li>""  :  None (Default) </li>    <li>`f1`  :  F1 Key </li>    <li>`f2`  :  F2 Key </li>    <li>`f3`  :  F3 Key </li>    <li>`f4`  :  F4 Key </li>    <li>`f5`  :  F5 Key </li>    <li>`f6`  :  F6 Key </li>    <li>`f7`  :  F7 Key </li>    <li>`f8`  :  F8 Key </li>    <li>`f9`  :  F9 Key </li>    <li>`f10`  :  F10 Key </li>    <li>`f11`  :  F11 Key </li>    <li>`f12`  :  F12 Key </li>    <li>`tab`  :  TAB Key </li>    <li>`control`  :  CONTROL  Key </li> </ul> |
|  Dynamic Capture Trigger | GFXRECON_CAPTURE_DYNAMIC_TRIGGER | BOOL | false | Set during runtime to `true` to start capturing and to `false` to stop. If not set at all then it is disabled (non-trimmed capture). [Replaces 'capture_android_trigger'] |
|  Dynamic Capture Trigger Dump Assets | GFXRECON_CAPTURE_DYNAMIC_TRIGGER_DUMP_ASSETS | BOOL | false | Setting this triggers a dump of all assets into the asset file. Dumping is done whenever this option switches between from `false` to `true` or from `true` to `false`. [Replaces 'capture_android_dump_assets'] |
| Hotkey Capture Frame Limit | GFXRECON_CAPTURE_TRIGGER_FRAMES | STRING | <empty> | Specify a limit on the number of frames to be captured via hotkey. Example: `1` will capture exactly one frame when the trigger key is pressed. If not defined, then there is no limit |
| Use Asset File | GFXRECON_CAPTURE_USE_ASSET_FILE | BOOL | false | When set to `true` assets (images, buffers and descriptors) will be stored separately into an asset file instead of the capture file. |
| Debug Device Lost | GFXRECON_DEBUG_DEVICE_LOST | BOOL | false | [D3D12 Only] Enables automatic injection of breadcrumbs into command buffers and page fault reporting. Used to debug device removed problems. |
| Enable Debug Layer | GFXRECON_DEBUG_LAYER | BOOL | false | [D3D12 Only] Enable the Direct3D debug layer for Direct3D 12 application captures. |
| Disable DXR Support | GFXRECON_DISABLE_DXR | BOOL | false | [D3D12 Only] Override the result of `CheckFeatureSupport` to report the `RaytracingTier` as `D3D12_RAYTRACING_TIER_NOT_SUPPORTED`. |
| Disable MetaCommmand Support | GFXRECON_DISABLE_META_COMMAND | BOOL | false | [D3D12 Only] Override the result of `CheckFeatureSupport` to report the D3D12_FEATURE_QUERY_META_COMMAND is invalid. Some games might use DirectStorage to improve performance, GFXR could not fully support the feature. By using this option, could bypass the feature and force game to use regular I/O to read from harddisk. |
| Force Command Serialization | GFXRECON_FORCE_COMMAND_SERIALIZATION | BOOL | false | Sets exclusive locks (unique_lock) for every ApiCall. It can avoid external multi-thread to cause captured issue. |
| Log Apply Additional Indentation | GFXRECON_LOG_ALLOW_INDENTS | BOOL | false | Apply additional indentation formatting to log messages. |
| Log Break on Error | GFXRECON_LOG_BREAK_ON_ERROR | BOOL | false | Trigger a debug break when logging an error. |
| Log Name and Line Number | GFXRECON_LOG_DETAILED | BOOL | false | Include name and line number from the file responsible for the log message. |
| Log Errors to StdError | GFXRECON_LOG_ERRORS_TO_STDERR | BOOL | true | Log any errors that occur to stderr. |
| Log File | GFXRECON_LOG_FILE | STRING (filter = *.txt) | <empty> | When set, log messages will be written to a file at the specified location. |
| Log File Overwrite | GFXRECON_LOG_FILE_CREATE_NEW | BOOL | true | Specifies that log file initialization should overwrite an existing file when `true`, or append to an existing file when `false`. |
| Log File Flush After Write | GFXRECON_LOG_FILE_FLUSH_AFTER_WRITE | BOOL | false | Flush the log file to disk after each write when true. |
| Log File Keep Open | GFXRECON_LOG_FILE_KEEP_OPEN | BOOL | true | Keep the log file open between log messages when true, or close and reopen the log file for each message when false. |
| Log Level | GFXRECON_LOG_LEVEL | STRING with possible values of: "debug", "info", "warning", "error", "fatal" | info | Specify the highest level message to log.  The specified level and all levels listed after it will be enabled for logging.  For example, choosing the `warning` level will also enable the `error` and `fatal` levels. <level> may be the following: <ul>    <li>`debug`  :  Messages  with debug-severity and  higher </li>    <li>`info`  :  Messages  with info-severity and  higher </li>    <li>`warning`  :  Messages  with warning-severity and  higher </li>    <li>`error`  :  Only  Error/Fatal messages </li>    <li>`fatal`  :  Only Fatal  messages </li> </ul> |
| Log Output to Console / stdout | GFXRECON_LOG_OUTPUT_TO_CONSOLE | BOOL | true | Log messages will be written to stdout. |
| Log Output to Debug Console | GFXRECON_LOG_OUTPUT_TO_OS_DEBUG_STRING | BOOL | false | [Windows Only] Log messages will be written to the Debug Console with OutputDebugStringA |
| Log Timestamps | GFXRECON_LOG_TIMESTAMPS | BOOL | false | Output a timestamp in front of each log message. |
| Memory Tracking Mode | GFXRECON_MEMORY_TRACKING_MODE | STRING with possible values of: "page_guard", "assisted", "unassisted" | page_guard | Specifies the memory tracking mode to use for detecting modifications to mapped Vulkan memory objects. Available options are: page_guard, assisted, and unassisted.  may be the following: <ul>    <li>`page_guard`  :   Tracks modifications to  individual memory pages,  which are written to the  capture file on calls to  vkFlushMappedMemoryRanges,  vkUnmapMemory, and  vkQueueSubmit. Tracking  modifications requires  allocating shadow memory for  allmapped memory. </li>    <li>`assisted`  :  Expects  theapplication to call  vkFlushMappedMemoryRanges  after memory is modified;  thememory ranges specified  tothe  vkFlushMappedMemoryRanges  call will be written to the  capture file during the  call. </li>    <li>`unassisted`  :   Writes the full content of  mapped memory to the capture  file on calls to  vkUnmapMemory and  vkQueueSubmit. It is very  inefficient and may be  unusable with real-world  applications that map large  amounts of memory. </li> </ul> |
| Page Guard Copy on Map | GFXRECON_PAGE_GUARD_COPY_ON_MAP | BOOL | true | When the `page_guard` memory tracking mode is enabled, copies the content of the mapped memory to the shadow memory immediately after the memory is mapped. |
| Page Guard External Memory | GFXRECON_PAGE_GUARD_EXTERNAL_MEMORY | BOOL | false | [Windows Only] When the `page_guard` memory tracking mode is enabled, use the VK_EXT_external_memory_host extension to eliminate the need for shadow memory allocations. For each memory allocation from a host visible memory type, the capture layer will create an allocation from system memory, which it can monitor for write access, and provide that allocation to vkAllocateMemory as external memory. |
| Page Guard Persistent Memory | GFXRECON_PAGE_GUARD_PERSISTENT_MEMORY | BOOL | false | When the `page_guard` memory tracking mode is enabled, this option changes the way that the shadow memory used to detect modifications to mapped memory is allocated. The default behavior is to allocate and copy the mapped memory range on map and free the allocation on unmap. When this option is enabled, an allocation with a size equal to that of the object being mapped is made once on the first map and is not freed until the object is destroyed. This option is intended to be used with applications that frequently map and unmap large memory ranges, to avoid frequent allocation and copy operations that can have a negative impact on performance. This option is ignored when `page_guard_external_memory` is enabled. |
| Page Guard Separate Read Tracking | GFXRECON_PAGE_GUARD_SEPARATE_READ | BOOL | true | When the `page_guard` memory tracking mode is enabled, copies the content of pages accessed for read from mapped memory to shadow memory on each read. Can overwrite unprocessed shadow memory content when an application is reading from and writing to the same page. |
| Page Guard Signal Handler Watcher | GFXRECON_PAGE_GUARD_SIGNAL_HANDLER_WATCHER | BOOL | false | When the `page_guard` memory tracking mode is enabled, setting this to `true` will spawn a thread which will will periodically reinstall the `SIGSEGV` handler if it has been replaced by the application being traced. |
| Page Guard Signal Handler Watcher Max Restores | GFXRECON_PAGE_GUARD_SIGNAL_HANDLER_WATCHER_MAX_RESTORES | INT | 1 | Sets the number of times the watcher will attempt to restore the signal handler. Setting it to a negative will make the watcher thread run indefinitely. |
| Page Guard Unblock SIGSEGV | GFXRECON_PAGE_GUARD_UNBLOCK_SIGSEGV | BOOL | false | When the `page_guard` memory tracking mode is enabled and in the case that SIGSEGV has been marked as blocked in thread's signal mask, setting this to `true` will forcibly re-enable the signal in the thread's signal mask. |
| Queue Zero Only | GFXRECON_QUEUE_ZERO_ONLY | BOOL | false | Forces to using only QueueFamilyIndex: 0 and queueCount: 1 on capturing to avoid replay error for unavailable VkQueue. |
| Quit after capturing frame ranges | GFXRECON_QUIT_AFTER_CAPTURE_FRAMES | BOOL | false | Specify one or more comma-separated frame ranges to capture. Each range will be written to its own file. A frame range can be specified as a single value, to specify a single frame to capture, or as two hyphenated values, to specify the first and last frame to capture. Frame ranges should be specified in ascending order and cannot overlap. Note that frame numbering is 1-based (i.e. the first frame is frame 1). Example: 200,301-305 will create two capture files, one containing a single frame and one containing five frames. If not defined, all frames are captured. |
| Specify GPU Descriptor Handle RV Annotation | GFXRECON_RV_ANNOTATION_DESCRIPTOR | STRING | <empty> | [D3D12 Only] By default, the 2-byte identifier values are hard-coded. This option forces a specific identifier value to be used for annotating GPU descriptors. The value should be specified as a 2-byte hexadecimal string, e.g., `0xAB12` or `ab12`. |
| Enable Experimental RV Search | GFXRECON_RV_ANNOTATION_EXPERIMENTAL | BOOL | false | [D3D12 Only] Experimental feature to help enable replay of certain DXR/ExecuteIndirect workloads. RV annotation is a capture mode which attempts to detect when applications write Resource Values (RVs) to memory. Conceptually, RVs represent different types of GPU pointers that applications write as resource data. This capture mode writes GFXR-specific identifier values into unoccupied bits of application-facing RVs and then searches for the identifier values when the application performs a memory write. This allows GFXR to better track RV locations and eventually produce an RV-optimized capture file that may be replayed. Enabling this feature introduces performance overhead during capture, and may result in unstable capture and/or replay. |
| Specify GPU VA RV Annotation | GFXRECON_RV_ANNOTATION_GPUVA | STRING | <empty> | [D3D12 Only] By default, the 2-byte identifier values are hard-coded. This option forces a specific identifier value to be used for annotating GPUVAs. The value should be specified as a 2-byte hexadecimal string, e.g., `0xAB12` or `ab12`. |
| Use Random RV Annotation | GFXRECON_RV_ANNOTATION_RAND | BOOL | false | [D3D12 Only] By default, the 2-byte identifier values are hard-coded. This option generates random identifier values used for annotating GPUVAs and GPU descriptors. Use this if capture-time crashes are observed. |
| Screenshot Directory | GFXRECON_SCREENSHOT_DIR | STRING | <empty> | Directory used where screenshots are saved to.  Default is the current working directory. |
| Screenshot Format | GFXRECON_SCREENSHOT_FORMAT | STRING with possible values of: "BMP", "PNG" | BMP | Image file format to use when saving screenshots. <format> may be the following: <ul>    <li>`bmp`  :  Bitmap File  Format. </li>    <li>`png`  :  Portable  Network Graphics Format. </li> </ul> |
| Screenshot Specific Frames | GFXRECON_SCREENSHOT_FRAMES | STRING | <empty> | Specify one or more comma-separated frame ranges to screenshot. Each range will be written to its own file. A frame range can be specified as a single value, to specify a single frame to capture, or as two hyphenated values, to specify the first and last frame to capture. Frame ranges should be specified in ascending order and cannot overlap. Note that frame numbering is 1-based (i.e. the first frame is frame 1). |
| Screenshot Interval | GFXRECON_SCREENSHOT_INTERVAL | INT | 1 | Sets the number between every screenshot taken.  1 means every frame is screenshot, 2 means every other, etc. Example: If screenshot range is 10-15 and interval is 2, screenshot will be generated for frames 10, 12 and 14. |
<!-- CAPTURE_SETTINGS_OPTIONS TABLE CODEGEN END -->


### Capture Files

When a capture is created depends on the type of capture is being taken. If taking a full capture (all frames) then the capture file gets created when an export in `dxgi.dll` or `d3d12.dll` is called. If taking a trimmed capture (subset of frames), the capture file gets created on the starting condition (trigger hotkey or frame number).

#### Specifying Capture File Location

The capture file's save location can be specified by setting the`GFXRECON_CAPTURE_FILE` environment variable, described above in the [Layer Options](#layer-options) section.

#### Timestamps

When capture file timestamps are enabled, a timestamp with an ISO 8601-based
format will be added to the name of every file created by the layer. The timestamp is generated when the capture file is created and is added to the base filename specified through the `GFXRECON_CAPTURE_FILE` environment variable. Timestamps have
the form:

```text
_yyyymmddThhmmss
```

where the lower-case letters stand for: Year, Month, Day, Hours, Minutes, Seconds.  The `T` is a designator that separates the date and time components. Time is reported for the local timezone and is specified with the 24-hour format.

The following example shows a timestamp that was added to a file that was originally named `gfxrecon_capture.gfxr` and was created at 2:35 PM on November 25, 2018:  `gfxrecon_capture_20181125T143527.gfxr`

### Trimmed Captures

Trimmed captures are created when GFXR is configured to start capturing at some later
time in execution.

To create a trimmed capture one of the trimming options can be used.
For example on desktop there is the `GFXRECON_CAPTURE_FRAMES` environment variable,
which specifies the frame ranges to capture, each range generating a separate
trimmed capture file. There's also the `GFXRECON_CAPTURE_TRIGGER` environment
variable. Each time the hot key is pressed a new trimmed capture is started/stopped.

An existing capture file can be trimmed by replaying the capture with the capture layer
enabled and a trimming frame range or trimming hot key enabled. (However, replay for
some content may be fast enough using the hot key may be difficult.) Here's an example
command-line that replays an existing capture with the capture layer enabled and
configured to capture only from frame 100 through frame 200 into a new capture file:

`gfxrecon-capture.py -f 100-200 gfxrecon-replay gfxrecon-example-capture.gfxr``

## Replaying API Calls

The GFXReconstruct Replay tool, `gfxrecon-replay`, can be used to replay files captured with or generated by other GFXReconstruct components.

GFXReconstruct leverages the Agility SDK runtime in order to replay. This means that GFXReconstruct's D3D12 support inherits the same Windows OS version requirements that are imposed by the Agility SDK. Please see the official Agility SDK documentation for the most up-to-date requirements.

A folder named `D3D12` that contains the required Agility SDK runtime must exist in the same folder as `gfxrecon-replay.exe`. When building GFXReconstruct, the `D3D12` folder will be created in the `gfxrecon-replay` output build folder. If `gfxrecon-replay.exe` is copied to another folder, the `D3D12` folder must be copied along with it.

### Command Line Arguments

The `gfxrecon-replay` command is used to perform a replay of a capture file.

The `gfxrecon-replay` command has the following usage:

```text
gfxrecon-replay <options> <file>
```

With `<file>` being the name of the required input capture file.

`<options>` can be any of the following:

<!-- REPLAY_SETTINGS_OPTIONS TABLE CODEGEN BEGIN -->
| Command-Line Argument | Required | Description | Default | Valid for APIs |
| --------------------- | -------- | ----------- | ------- | -------------- |
| --debug-device-lost | Optional | [D3D12 Only] Enables automatic injection of breadcrumbs into command buffers and page fault reporting. Used to debug device removed problems. | false | D3D12 |
| --log-file &lt;log_file&gt; | Optional | When set, log messages will be written to a file at the specified location. | <empty> | ALL |
| --log-level &lt;level&gt; | Optional | Specify the highest level message to log.  The specified level and all levels listed after it will be enabled for logging.  For example, choosing the `warning` level will also enable the `error` and `fatal` levels. <level> may be the following: <ul>    <li>`debug`  :  Messages  with debug-severity and  higher </li>    <li>`info`  :  Messages  with info-severity and  higher </li>    <li>`warning`  :  Messages  with warning-severity and  higher </li>    <li>`error`  :  Only  Error/Fatal messages </li>    <li>`fatal`  :  Only Fatal  messages </li> </ul> | info | ALL |
| --log-timestamps | Optional | Output a timestamp in front of each log message. | false | ALL |
| --screenshot-dir &lt;dir&gt; | Optional | Directory used where screenshots are saved to.  Default is the current working directory. | <empty> | ALL |
| --screenshot-format &lt;format&gt; | Optional | Image file format to use when saving screenshots. <format> may be the following: <ul>    <li>`bmp`  :  Bitmap File  Format. </li>    <li>`png`  :  Portable  Network Graphics Format. </li> </ul> | bmp | ALL |
| --screenshots &lt;range&gt; | Optional | Generate screenshots for the specified frames. Target frames are specified as a comma separated list of frame ranges.  A frame range can be specified as a single value, to specify a single frame, or as two hyphenated values, to specify the first and last frames to process.  Frame ranges should be specified in ascending order and cannot overlap.  Note that frame numbering is 1-based (i.e. the first frame is frame 1). Example: 200,301-305 will generate six screenshots. | <empty> | ALL |
| --screenshot-interval &lt;N&gt; | Optional | Sets the number between every screenshot taken.  1 means every frame is screenshot, 2 means every other, etc. Example: If screenshot range is 10-15 and interval is 2, screenshot will be generated for frames 10, 12 and 14. | 1 | ALL |
| --add-new-pipeline-caches | Optional | If set, allows gfxreconstruct to create new vkPipelineCache objects when it encounters a pipeline created without cache. This option can be used in coordination with `--save-pipeline-cache` and `--load-pipeline-cache`. | false | ALL |
| --allowed-messages &lt;messages&gt; | Optional | [Windows D3D12 Only] Identify messages to allow D3D12 debugging to output | <empty> | D3D12 |
| --batching-memory-usage &lt;percentage&gt; | Optional | [Windows D3D12 Only] Max amount of memory consumption while loading a trimmed capture file. Acceptable values range from 0 to 100. 0 means no batching at all 100 means use all available system and GPU memory | 80 | D3D12 |
| --capture | Optional | Capture the replaying GFXR file. Capture uses the same log options as replay. All other capture option behavior and usage is the same as when capturing with the GFXR layer. The capture functionality is included in the `gfxrecon-replay` executable -- no GFXR capture layer is added to the Vulkan layer chain. | false | ALL |
| --cpu-mask &lt;binary-mask&gt; | Optional | Set of CPU cores used by the replayer. `binary-mask` is a succession of '0' and '1' read from left to right that specifies used/unused cores. For example '10010' activates the first and fourth cores and deactivate all other cores. If the option is not set, all cores can be used. If the option is set only for some cores, the other cores are not used. | <empty> | ALL |
| --create-dummy-allocations | Optional | [Windows Only] Enable creation of dummy heaps and resources for replay validation. | false | ALL |
| --debug-messenger-level &lt;level&gt; | Optional | Specify highest debug messenger severity level. Options are: debug, info, warning, and error. <level> may be the following: <ul>    <li>`debug`  :  Messages  with debug-severity and  higher </li>    <li>`info`  :  Messages  with info-severity and  higher </li>    <li>`warning`  :  Messages  with warning-severity and  higher </li>    <li>`error`  :  Only  Error/Fatal messages </li>    <li>`fatal`  :  Only Fatal  messages </li> </ul> | warning | ALL |
| --denied-messages &lt;messages&gt; | Optional | [Windows D3D12 Only] Identify messages to deny D3D12 debugging to output | <empty> | D3D12 |
| --deduplicate-device | Optional | If set, at most one VkDevice will be created for each VkPhysicalDevice for RenderDoc and DXVK case. | false | ALL |
| --dcp\|--discard-cached-psos | Optional | [D3D12 Only] Discard cached PSOs [Deprecated, please use '--use-cached-psos']. | false | D3D12 |
| --dump-resources &lt;args&gt; | Optional | Dump out resources during replay <args> may be the following: <ul>    <li>`<submit-index,command-index,draw-call-index>`  :   The capture file will be   examined, and   &lt;submit-index,command-index,draw-call-index&gt;   will be used to dump   resources for the given   submit, command and draw   indices. </li>    <li>`<filename>.json`  :   Extract `--dump-resources`   block indices args from the   specified json file. The   format for the json file is   documented in detail in the   gfxreconstruct   documentation. </li> </ul> | <empty> | ALL |
| --dump-resources-before-draw | Optional | In addition to dumping GPU resources after the draw calls specified by the `--dump-resources` argument, also dump resources before the draw calls. | false | ALL |
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
| --dump-resources-modifiable-state-only | Optional | [D3D12 Only] Only dump resources that are in a modifiable state set by D3D12 ResourceBarrier. | false | D3D12 |
| --dump-resources-scale &lt;scale&gt; | Optional | Scale images generated by dump resources by the given scale factor. The scale factor must be a floating point number greater than 0. Values greater than 10 are capped at 10. Default value is 1.0. | 1.0 | ALL |
| --dx12-ags-inject-markers | Optional | [D3D12 Only] Label each API calls as block index of the trace Radeon GPU Detective could dump the label for debugging. | false | D3D12 |
| --dx12-override-object-names | Optional | [D3D12 Only] Generate unique names for all ID3D12Objects and assign each object the generated name.  This is intended to assist in replay debugging. | false | D3D12 |
| --dx12-two-pass-replay | Optional | [D3D12 Only] Enable D3D12 Two Pass Replay. | false | D3D12 |
| --flush-inside-measurement-range | Optional | If this is specified the replayer will flush and wait for all current GPU work to finish at the end of each frame inside the measurement range. | false | ALL |
| --fmr\|--flush-measurement-range | Optional | If this is specified the replayer will flush and wait for all current GPU work to finish at the start and end of the measurement range. | false | ALL |
| --fw\|--force-windowed &lt;width,height&gt; | Optional | [Windows D3D12 Only] Setup windowed and override resolution. | <empty> | D3D12 |
| --fwo\|--force-windowed-origin &lt;x,y&gt; | Optional | [Windows D3D12 Only] Setup windowed origin. | <empty> | D3D12 |
| --gpu &lt;index&gt; | Optional | Use the specified device for replay, where index is the zero-based index to the array of physical devices returned by vkEnumeratePhysicalDevices or IDXGIFactory1::EnumAdapters1. Replay may fail if the specified device is not compatible with the original capture devices. | <empty> | ALL |
| --gpu-group &lt;index&gt; | Optional | Use the specified device group for replay, where index is the zero-based index to the array of physical device group returned by vkEnumeratePhysicalDeviceGroups. Replay may fail if the specified device group is not compatible with the original capture device group. | <empty> | ALL |
| -h\|--help | Optional | Print usage information and exit | false | ALL |
| --input-capture &lt;input_file&gt; | Optional | The name of the capture file to use for processing.  This overrides the non-tagged input filename that used to appear after all optional arguments | <empty> | ALL |
| --load-pipeline-cache &lt;cache-file&gt; | Optional | If set, loads data created by the `--save-pipeline-cache` option in <cache-file> and uses it to create the pipelines instead of the pipeline caches saved at capture time. | <empty> | ALL |
| --log-debugview | Optional | Log messages with OutputDebugStringA. | false | ALL |
| --measurement-file &lt;file&gt; | Optional | Write measurements to a file at the specified path. Default is: '/sdcard/gfxrecon-measurements.json' on Android and'./gfxrecon-measurements.json' on desktop. | gfxrecon-measurement.gfxr | ALL |
| --mfr\|--measurement-frame-range &lt;start_frame&gt;-&lt;end_frame&gt; | Optional | Custom framerange to measure FPS for. This range will include the start frame but not the end frame. The measurement frame range defaults to all frames except the loading frame but can be configured for any range. If the end frame is past the last frame in the trace it will be clamped to the frame after the last (so in that case the results would include the last frame). | <empty> | ALL |
| -m\|--memory-translation &lt;mode&gt; | Optional | Enable memory translation for replay on GPUs with memory types that are not compatible with the capture GPU's memory types <mode> may be the following: <ul>    <li>`none`  :  No memory  translation is performed  (Default). </li>    <li>`remap`  :  Attempt to  mapcapture memory types to  compatible replay memory  types, without altering  memory allocation behavior. </li>    <li>`realign`  :  Adjust  memory allocation sizes and  resource binding offsets  based on replay memory  properties. </li>    <li>`rebind`  :  Change  memory allocation behavior  based on resource usage and  replay memory properties.   Resources may be bound to  different allocations with  different offsets.  Uses VMA  tomanage allocations and  suballocations. </li> </ul> | none | ALL |
| --no-debug-popup | Optional | [Windows Only] Disable the 'Abort, Retry, Ignore' message box displayed when abort() is called (Windows debug only). | false | ALL |
| --offscreen-swapchain-frame-boundary | Optional | Should only be used with offscreen swapchain. Activates the extension VK_EXT_frame_boundary (always supported if trimming, checks for driver support otherwise) and inserts command buffer submission with VkFrameBoundaryEXT where vkQueuePresentKHR was called in the original capture.  This allows preserving frames when capturing a replay that uses offscreen swapchain. | false | ALL |
| --opcd\|--omit-pipeline-cache-data | Optional | Omit pipeline cache data from calls to vkCreatePipelineCache and skip calls to vkGetPipelineCacheData | false | ALL |
| --pause-frame &lt;frame&gt; | Optional | Pause after replay frame number <frame> | -1 | ALL |
| --paused | Optional | Pause after replaying the first frame (same as `--pause-frame 1`). | false | ALL |
| --pbi-all | Optional | Print all block information. | false | ALL |
| --pbis &lt;index1,index2&gt; | Optional | Print block information between block index1 and block index2. | <empty> | ALL |
| --pcj\|--pipeline-creation-jobs &lt;num_jobs&gt; | Optional | Specify the number of asynchronous pipeline-creation jobs as integer. If <num_jobs> is negative it will be added to the number of cpu-cores Default: 0 (do not use asynchronous operations). | 0 | ALL |
| --preload-measurement-range | Optional | Preload the provided measurement range of frames. | false | ALL |
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
| --swapchain &lt;mode&gt; | Optional | Choose a swapchain mode to replay. <mode> may be the following: <ul>    <li>`captured`  :  Use the  swapchain indices stored in  thecapture directly on the  swapchain setup for replay. </li>    <li>`offscreen`  :   Disable creating swapchains,  surfaces and windows. To see  rendering, add the  --screenshots option. </li>    <li>`virtual`  :  Virtual  Swapchain of images which  match the swapchain in  effect at capture time and  which are copied to the  underlying swapchain of the  implementation being  replayed on (Default). </li> </ul> | virtual | ALL |
| --sync | Optional | Synchronize after each queue submission with vkQueueWaitIdle. | false | ALL |
| --use-cached-psos | Optional | [Windows D3D12 Only] Permit using cached PSOs when creating graphics or compute pipelines.  Using cached PSOs may reduce PSO creation time but may result in replay errors. | false | D3D12 |
| --use-captured-swapchain-indices | Optional | Same as `--swapchain captured`.  Ignored if the `--swapchain` option is used. | false | ALL |
| --use-colorspace-fallback | Optional | Swap the swapchain color space if unsupported by replay device. Check if color space is not supported by replay device and fallback to VK_COLOR_SPACE_SRGB_NONLINEAR_KHR. | false | ALL |
| --validate | Optional | Enable the Khronos Vulkan validation layer when replaying a Vulkan capture or the Direct3D debug layer when replaying a Direct3D 12 capture. | false | ALL |
| --version | Optional | Print version information and exit. | false | ALL |
| --vssb\|--virtual-swapchain-skip-blit | Optional | If using Virtual Swapchain, skip the actual BLTs to the actual HW surface to gain performance during replay. | false | ALL |
| --wait-before-present | Optional | Force wait on completion of queue operations for all queues before calling Present. This is needed for accurate acquisition of instrumentation data on some platforms. | false | ALL |
| --wsi &lt;platform&gt; | Optional | Force replay to use the specified wsi platform. <platform> may be the following: <ul>    <li>`auto`  :   Automatically attempt to  determine WSI used based on  platform (Default). </li>    <li>`win32`  :  Force  WIN32 WSI type WSI (Valid on  Windows). </li>    <li>`xlib`  :  Force using  XLIB WSI (Valid on Linux). </li>    <li>`xcb`  :  Force using  XCBWSI (Valid on Linux). </li>    <li>`wayland`  :  Force  using Wayland WSI (Valid on  Linux). </li>    <li>`display`  :  Force  using Display WSI. </li>    <li>`headless`  :  Force  using headless WSI. </li> </ul> | auto | ALL |
<!-- REPLAY_SETTINGS_OPTIONS TABLE CODEGEN END -->


### Keyboard Controls

The `gfxrecon-replay` tool supports the following key controls:

Key(s) | Action
-------|-------
Space, p | Toggle pause/play.
Right arrow, n | Advance to the next frame when paused.



### Capturing Replay (Recapture)

`gfxrecon-replay` can be run with GFXReconstruct capture enabled. The process to capture the output of `gfxrecon-replay` is the same as capturing any other application as described in [Capturing API calls](#capturing-api-calls). This process is referred to as "recapture".

When recapturing DXR or EI enabled captures, those captures must first be optimized using [DXR Optimization](#dxr-optimization).



## Other Capture File Processing Tools

### Capture File Info

The `gfxrecon-info` tool prints statistics for a GFXReconstruct capture file.

This includes:

- API-agnostic information about the capture file
- Executable name of the captured application
- Capture-time adapter/GPU information
- Capture-time driver information
- Swapchain dimensions (resolution)
- Whether it contains DXR workloads and has been optimized

```text
gfxrecon-info.exe - Print statistics for a GFXReconstruct capture file.

Usage:
  gfxrecon-info.exe [-h | --help] [--version] [--exe-info-only] <file>

Required arguments:
  <file>                The GFXReconstruct capture file to be processed.

Optional arguments:
  -h                    Print usage information and exit (same as --help).
  --version             Print version information and exit.
  --exe-info-only       Quickly exit after extracting captured application's executable name
```

### Capture File Compression

The `gfxrecon-compress` tool compresses or decompresses GFXReconstruct capture files. It can also be used to change the compression format used in a capture file.

```text
gfxrecon-compress.exe - A tool to compress/decompress GFXReconstruct capture files.

Usage:
  gfxrecon-compress.exe [-h | --help] [--version] <input_file> <output_file> <compression_format>

Required arguments:
  <input_file>          Path to the input file to process.
  <output_file>         Path to the output file to generate.
  <compression_format>  Compression format to apply to the output file.
                        Options are:
                          LZ4  - Use LZ4 compression.
                          ZLIB - Use zlib compression.
                          NONE - Remove compression.

Optional arguments:
  -h                    Print usage information and exit (same as --help).
  --version             Print version information and exit.
```

### Capture File Optimizer

The `gfxrecon-optimize` tool produces new capture files with improved replay performance.

Before attempting to optimize a capture file, please ensure it is able to replay first.

Like `gfxrecon-replay`, `gfxrecon-optimize` also requires the `D3D12` folder to exist beside it. As mentioned previously, this folder is where GFXReconstruct references the Agility SDK runtime.

There are two optimizations implemented for D3D12:

#### DXR Optimization

In order to replay DXR workloads, GFXReconstruct needs to ensure that GPUVAs are valid before submission time. This can be a challenge because GPUVAs recorded at capture-time cannot be reused verbatim at replay-time. Therefore GFXReconstruct replay needs to employ sophisticated GPUVA tracking mechanisms with the ultimate goal of patching resources with valid replay-time GPUVAs, before each submission. This is undesirable from a replay performance standpoint, but the optimizer tool can be used to remove this overhead.

The first pass of this optimization replays the capture, to gather information about how the application filled in DXR buffers with which GPUVAs. The second pass of this optimization takes this information and creates a new GFXReconstruct capture with DXR resources populated with correct GPUVAs, without the need to patch them before submit time.

It is strongly recommended that this optimization is executed after capturing any application with a DXR workload. It needs to only be ran **once** per capture, and **must** be done so on a machine that can successfully replay the original capture. The resulting optimized capture can be then replayed using `gfxrecon-replay`

#### Redundant PSO Removal

Many applications create thousands of pipeline state objects (PSOs) during load time, which get set as GPU state at some point later in time. However, this does not mean a GFXReconstruct capture will be long enough to actually use all PSOs that were created. It is therefore redundant for GFXR to replay time-consuming creation of thousands of PSOs, which will never actually get referenced in the capture.

The first pass of this optimization scans a capture, and gathers information about which PSOs were referenced or not. The second pass uses this information to construct a new capture file that excludes creation of PSOs which were never used to set GPU state. This results in huge time savings during replay load time for many applications.

It is strongly recommended that this optimization is executed after capturing any application. It needs to only be ran **once** per capture, and can be done so on **any** system. The resulting optimized capture can then be replayed using `gfxrecon-replay`

```text
gfxrecon-optimize.exe - Produce new captures with enhanced performance characteristics
                        For Vulkan, the optimizer will remove unused buffer and image initialization data (for trimmed captures)
                        For D3D12, the optimizer will improve DXR replay performance and remove unused PSOs (for all captures)

Usage:
  gfxrecon-optimize.exe [-h | --help] [--version] [--d3d12-pso-removal] [--dxr] [--gpu <index>] <input-file> <output-file>

Required arguments:
  <input-file>          The path to input GFXReconstruct capture file to be processed.
  <output-file>         The path to output GFXReconstruct capture file to be created.

Optional arguments:
  -h                    Print usage information and exit (same as --help).
  --version             Print version information and exit.
  --d3d12-pso-removal   D3D12-only: Remove creation of unreferenced PSOs.
  --dxr                 D3D12-only: Optimize for DXR replay.
  --gpu <index>         D3D12-only: Use the specified device for the optimizer replay, where index is the zero-based index to the array
                        of physical devices returned by vkEnumeratePhysicalDevices or the optimizer replay may fail if the specified
                        device is not compatible with the IDXGIFactory1::EnumAdapters1. The optimizer replay may fail if the specified
                        device is not compatible with the original capture devices.

Note: running without optional arguments will instruct the optimizer to detect API and run all available optimizations.
```

### Renaming Scripts

In order to obtain application-matching playback, it is sometimes necessary to rename `gfxrecon-replay.exe` to match the captured application's executable name. This is because driver behavior can sometimes change depending on the executable name of the running application. Renaming is not always required, but it is an added measure towards functional replay. The two following utility scripts are bundled, which extract the application name from the capture file and make the renaming task automatic. Renaming can also be done manually, since the application executable name can be extracted using the `gfxrecon-info` tool.

#### gfxrecon-replay-renamed.py

This script can be used to replay capture files. It performs automatic renaming and execution of `gfxrecon-replay.exe`

```text
gfxrecon-replay-renamed.py - Helper script to perform automatic renaming of gfxrecon-replay.exe prior to playback.

Usage:
  gfxrecon-replay-renamed.py <file> [optional_replayer_args]

Required arguments:
  <file>                     Path to the capture file to replay.

Optional arguments:
  [optional_replayer_args]   All optional arguments exposed by gfxrecon-replay.exe
```

#### gfxrecon-optimize-renamed.py

This script can be used to optimize capture files. It performs automatic renaming and execution of `gfxrecon-optimize.exe`

The optimizer is a candidate for renaming since the DXR portion of it runs a replay of the capture.

```text
gfxrecon-optimize-renamed.py - Helper script to perform automatic renaming of gfxrecon-optimize.exe prior to optimization.

Usage:
  gfxrecon-optimize-renamed.py <input-file> <output-file> [--dxr] [--d3d12-pso-removal]

Required arguments:
  <input-file>          The path to input GFXReconstruct capture file to be processed.
  <output-file>         The path to output GFXReconstruct capture file to be created.

Optional arguments:
  --d3d12-pso-removal   D3D12-only: Remove creation of unreferenced PSOs.
  --dxr                 D3D12-only: Optimize for DXR replay.
  --gpu <index>         D3D12-only: Use the specified device for the optimizer replay, where index is the zero-based index to the array
                        of physical devices returned by vkEnumeratePhysicalDevices or The optimizer replay may fail if the specified
                        device is not compatible with the IDXGIFactory1::EnumAdapters1. The optimizer replay may fail if the specified
                        device is not compatible with the original capture devices.

Note: running without optional arguments will instruct the optimizer to detect API and run all available optimizations.
```



## AMD GPU Services Support

Some applications adopt vendor-specific libraries to leverage GPU capabilities not exposed by graphics APIs. For example, the AMD GPU Services (AGS) library is commonly loaded by applications that have implemented features specific to AMD.

The GFXReconstruct capture process for AGS also leans on DLL substitution for interception. When an application loads amd_ags_x64.dll, it loads a proxy version provided by GFXReconstruct instead. From that point on, GFXReconstruct can record AGS function calls, process them, and call into the real AGS runtime.

This is supported for AGS versions 6.0 - 6.2.0. Versions 5.x or older are not supported. Only x64 applications are supported.


### How to Capture AGS

The process is the same as normal, with the addition that we must also perform some AGS DLL renaming. There are two versions of the AGS DLL:
- The official one, which comes bundled with the application (`amd_ags_x64.dll`)
- The proxy one, which comes bundled with GFXReconstruct (`amd_ags_x64_capture.dll`)

Steps:
1.	Identify the app executable.
2.	Identify the official AGS DLL that came bundled with the application, which usually lives beside its executable.
3.	Verify the AGS version that was shipped with the application. This can be done by inspecting its file properties. If the version is 6.0 - 6.2.0 then AGS calls made by this application can be captured.
4.	Rename the official AGS DLL to `amd_ags_x64_orig.dll`.
5.	Copy the GFXReconstruct capture libraries, plus the proxy AGS DLL, beside the application executable.
6.	Rename the proxy AGS DLL to `amd_ags_x64.dll`.
7.	At this point, the file structure should look like this:
```bash
    C:\AppPath\d3d12_app.exe
    C:\AppPath\d3d12.dll
    C:\AppPath\dxgi.dll
    C:\AppPath\d3d12_capture.dll
    C:\AppPath\amd_ags_x64.dll
    C:\AppPath\amd_ags_x64_orig.dll
```
8.	Resume standard full/trim capture procedures, and obtain a capture file.
9.	When finished, make sure to remove the GFXReconstruct capture libraries, and restore the official AGS DLL bundled with the application.

### How to Process AGS Files

Both gfxrecon-replay and gfxrecon-optimize are able to read and process capture files that contain AGS calls. From a user point of view, their usage remains unchanged.