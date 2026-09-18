# GFXReconstruct tests

This directory holds the tests that run without a GPU.
[TESTING_test_apps.md](../TESTING_test_apps.md) explains how to write a test app.
This file explains how the tests fit together, how to run them, and how to keep them honest.

## What is here

| Path | Content |
|---|---|
| `icd/` | A mock Vulkan ICD. The loader finds it through `VkICD_mock_icd.json`. It returns success and does no work. |
| `android_mock/` | A host build of the Android hardware buffer functions, so the `ahb` app runs on a desktop. |
| `test_apps/` | The test apps and the launcher, `gfxrecon-test-launcher`, that runs one app by name. |
| `test_cases/` | The gtest cases. Each file holds the cases for one app or one topic. |
| `known_good/` | One reference capture per app. A case compares a new capture against it. `known_good/<driver>/` holds the reference images for the pixel cases on that driver. |
| `verify-gfxr.{h,cpp}` | The harness functions that the cases call. |
| `run-tests.sh.in`, `run-tests.ps1.in`, `run-tests_macos.sh.in` | Templates for the run scripts. CMake fills in the paths. |
| `test_environment.cmake.in` | The loader environment and the driver label that ctest gives every case, for the driver that `GFXRECON_TEST_DRIVER` names. |

## The tiers

By default every case in this directory runs on the mock ICD.
That proves what the capture layer wrote and what the replayer read.
It does not prove that a draw put the correct pixels in an image, because the mock draws nothing.

The second tier runs the same apps on lavapipe, the Mesa software rasterizer.
Lavapipe executes shaders and has memory properties that differ from the mock.
It is the tier that proves a pixel: the pixel cases compare a replay screenshot against a
reference image in `known_good/lavapipe/`.
`GFXRECON_TEST_DRIVER=lavapipe` in the shell selects it, as the next section shows.
The known-good files are captures on the mock, so a case that compares against one holds only on
the mock.
Every case carries a label that says which driver it needs, and a run on lavapipe disables the
cases that need the mock.
The Linux CI jobs run the suite on both drivers.

## Build and run

Build with the test apps and install:

```bash
python3 scripts/build.py --skip-check-code-style --test-apps
```

The install directory is `build/<platform>/<arch>/output`.
`ctest` starts each case from `<install>/test`, so the install must exist before a run.

Run everything from the build directory:

```bash
ctest --test-dir build/linux/x64 --output-on-failure
```

Select cases by label or by name:

```bash
ctest --test-dir build/linux/x64 -L unit          # The Catch2 unit tests only.
ctest --test-dir build/linux/x64 -L smoke         # The test app cases only.
ctest --test-dir build/linux/x64 -L any-driver    # The test app cases that hold on every driver.
ctest --test-dir build/linux/x64 -R Triangle      # Every case whose name matches.
ctest --test-dir build/linux/x64 -N               # List the cases and run nothing.
```

Each test app case has the label `smoke` and one driver label.
`mock-only` marks a case that compares against a known-good file, or an app that needs the mock
or an extension that lavapipe does not have.
`any-driver` marks the rest.
`real-driver` marks the pixel comparisons, which need a driver that executes shaders, and the
cross-driver cases, which need lavapipe.
`test_environment.cmake.in` holds the name patterns that select `mock-only` and `real-driver`,
with a reason each.
ctest disables every case whose driver is not the current one.
A plain run on lavapipe runs the `any-driver` and `real-driver` cases and reports the rest as not
run.
A plain run on the mock runs the `any-driver` and `mock-only` cases.

The run script in `<install>/test` does the same for the test app cases and adds one mode.
An argument that is an app name runs that app alone, without a comparison:

```bash
cd build/linux/x64/output/test
./run-tests.sh                 # Every test app case.
./run-tests.sh -R Triangle     # Arguments that start with "-" go to ctest.
./run-tests.sh triangle        # One app, no comparison.
```

Each run script run also writes a JUnit file in the test directory.
A run on the mock writes `ctest-results.xml` and a run on lavapipe writes
`ctest-results-lavapipe.xml`, so the two do not overwrite each other.
CI shows both files on the job summary page.

Select the driver with `GFXRECON_TEST_DRIVER`.
The value is `mock`, the default, or `lavapipe`.
Both ctest and the run script read it, and a switch needs no reconfigure:

```bash
GFXRECON_TEST_DRIVER=lavapipe ctest --test-dir build/linux/x64 -L smoke
GFXRECON_TEST_DRIVER=lavapipe ./run-tests.sh triangle
```

On Linux the lavapipe manifest comes from the `mesa-vulkan-drivers` package, at
`/usr/share/vulkan/icd.d/lvp_icd.x86_64.json`.
Another location, or another platform, sets `GFXRECON_TEST_LAVAPIPE_ICD_JSON` at configure time.

## The environment

`GFXRECON_TEST_ENVIRONMENT` in `CMakeLists.txt` is the common list of environment variables.
It selects the capture layer and makes the apps headless.
`GFXRECON_TEST_ENVIRONMENT_MOCK` and `GFXRECON_TEST_ENVIRONMENT_LAVAPIPE` each select one driver.
ctest applies the common list and one driver list to every case, and the run scripts export the
same lists.
`test_environment.cmake` picks the driver list at ctest time from `GFXRECON_TEST_DRIVER`.
A variable that you export in your shell does not override an entry in these lists.

## What a case does

`verify_gfxr(name)` runs the app with the capture layer on, converts the new capture to JSON,
converts `known_good/<name>.gfxr` to JSON, drops the fields that differ between runs, and
compares the two documents.
Every file that a case writes goes to `results/<case name>/` under the test directory, and the
harness empties that directory first.
The case name is the gtest name, such as `CaptureApps_Serialized.CorrectGFXR_triangle`, so two
cases never share a file and `ctest -j` is safe.
`known_good/` is read only.
The list of dropped fields is in `verify-gfxr.cpp`, with a reason next to each entry.
`json-normalizer.cpp` tests that list.

`capture_and_replay(name, args)` runs the app with the capture layer on and then replays the
capture with `gfxrecon-replay` and the given arguments.
It checks only the exit code.

`verify_screenshot(name, frame)` runs the app with the capture layer on, replays the capture with
a PNG screenshot of one frame, and compares that image against
`known_good/<driver>/<name>_frame_<frame>.png`.
The comparison is the root mean square of the per-channel differences, as a percentage of the
full range, and the two images match when it is 1.18 percent or less.
That rule and that threshold come from `imageutils.py` in the VulkanTests project.
The cases are in `test_cases/screenshots.cpp`, and `ScreenshotCompare` there proves the rule on
the reference images with no driver.

`capture_on_replay_on(name, capture_driver, replay_driver, args)` captures on one driver and
replays on the other, with the given replay arguments.
The mock has six memory types and lavapipe has one, so this is the test of the memory
translation modes across real memory properties.
It checks only the exit codes.
The cases are in `test_cases/cross-driver.cpp`.
`test_cases/memory-translation.cpp` runs each mode on the same device, on every driver.

`reruns.cpp` runs each capture app two more times.
One run has `GFXRECON_CAPTURE_PROCESS_NAME` set to a name that does not match.
The layer must load and write no file.
The other run has `GFXRECON_FORCE_COMMAND_SERIALIZATION=true` and must match the same known good.
When you add an app, add its name to the lists in that file.

## Add a case

1. Add a file to `test_cases/` with one `TEST` that calls a harness function.
2. Add the file to `GFXRECON_TESTCASES` in `CMakeLists.txt`.
3. Make the known-good file as the next section describes.
4. Add the app name to the lists in `test_cases/reruns.cpp`.

If the app cannot run on the mock yet, add the case with the `DISABLED_` prefix.
Write the blocker in a comment above it.
ctest lists a disabled case and does not run it.

If the app cannot run on lavapipe, add a name pattern for it to `test_environment.cmake.in`.
Write the reason next to the pattern.

## Make or update a reference image

The reference image is a replay screenshot on lavapipe.
Every app renders the same image in every frame, so one frame per app is enough.

1. Add the app to `kScreenshotApps` in `test_cases/screenshots.cpp`, then build and install.
2. Run the case once on lavapipe.
   It fails with the path of the screenshot it made.
   Open the image and make sure that it shows what the app draws.
3. Copy the image to `test/known_good/lavapipe/`, then install again.
4. Run the case two more times.
   Both must pass.

A reference image is small, from a few hundred bytes to a few kilobytes, because the apps draw
flat shapes and PNG compresses them well.
Lavapipe changes with the Mesa version, so a runner image update can move a pixel.
The threshold absorbs a small move, and a large one means a new reference and a note in the pull
request.

## Make or update a known-good file

The known-good file is a capture of the app on the mock ICD.

1. Delete `known_good/<name>.gfxr` if one exists, then build and install.
2. Run the case once.
   It fails, and it leaves `<name>.gfxr` in `<install>/test/results/<Suite>.<Case>/`.
3. Copy that file to `test/known_good/<name>.gfxr`, then install again.
4. Run the case two more times.
   Both must pass.
   If one fails, the capture is not deterministic.
   Find the source of the difference before you commit.
5. Look at the size.
   An app renders 10 frames unless a comment says why it needs more.
   The whole `known_good/` directory stays under 5 MB, reference images included.
   CMake fails the configure step when the directory is over that budget.

A known-good file holds on every platform only if the app never derives a value from the
OS page size.
Two things in the capture layer depend on it.
The layer aligns the memory requirements that the app receives to the page size, so the mock
reports every requirement in 64 KiB units and that alignment changes nothing.
The layer records dirty mapped memory in whole pages, so an app must write mapped memory in
whole 64 KiB blocks at 64 KiB offsets.
A page is 4 KiB on Linux x86 and 16 KiB on Apple Silicon.

## Keep the tests honest

Two habits catch a test that passes for the wrong reason.

Disable the code that your test covers, then run the test.
If the test still passes, it does not test that code.

Prove a harness assertion before you trust it.
Start the runner with the mock driver and no capture layer.
The app needs the headless flag, or it stops at window creation and the case fails too early:

```bash
cd build/linux/x64/output/test
env -i PATH="$PATH" HOME="$HOME" GFXRECON_TESTAPP_HEADLESS=true \
    VK_DRIVER_FILES="$PWD/test_apps/VkICD_mock_icd.json" \
    GFXRECON_TESTAPP_MOCK_ICD="$PWD/test_apps/libVkICD_mock_icd.so" \
    ./gfxrecon-testapp-runner --gtest_filter=Triangle.CorrectGFXR
```

The case must fail at "capture file was not produced".
A pass here means the harness reads a file from an earlier run.

## Known gaps

`HostImageCopy.CorrectGFXR` is disabled.
The app copies bytes through `VK_EXT_host_image_copy` and compares them.
The mock moves no bytes.
It runs when the mock backs device memory and executes copies.

`CrossDriver.DISABLED_MockCaptureRemapsOnLavapipe` waits for a harness function that expects a
failure.
The replayer refuses the mapping by design, because lavapipe has no lazily allocated memory type.
