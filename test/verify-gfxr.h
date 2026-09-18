#ifndef GFXRECONSTRUCT_VERIFY_GFXR_H
#define GFXRECONSTRUCT_VERIFY_GFXR_H

#include <nlohmann/json.hpp>

#include <filesystem>
#include <string>
#include <vector>

void run_in_background(const char* test_name);

/**
 * Parse callback for nlohmann::json::parse that drops the parts of a converted capture that differ
 * between two runs or two machines: version fields, handles, pointers, file descriptors, the header
 * block and every annotation block. Both the new capture and the known-good capture go through it
 * before the comparison.
 *
 * @param depth   - nesting depth of the current event
 * @param event   - the parse event
 * @param parsed  - the key, the value or the finished object
 * @return false to drop the key, the value or the object, true to keep it
 */
bool clean_gfxr_json(int depth, nlohmann::json::parse_event_t event, nlohmann::json& parsed);

/**
 * Run an application with capture enabled, and compare the resulting gfxr file to a known good gfxr
 *
 * @param test_name         - the name of the test to launch
 * @param trimming_frames   - It's not nullptr if it needs to run trimming app. frame info ex: "10" or "10-100"
 *
 * @note expects the following environment variables to be set
 * VK_LAYER_PATH     - path to VkLayer_gfxreconstruct.dll
 * VK_DRIVER_FILES   - path to VkICD_mock_icd.json
 *
 * @note expects the following environment variables to be set with the following values
 * VK_INSTANCE_LAYERS=VK_LAYER_LUNARG_gfxreconstruct
 * GFXRECON_CAPTURE_FILE_TIMESTAMP=false
 * GFXRECON_CAPTURE_FILE=actual.gfxr
 */
void verify_gfxr(const char* test_name, char const* trimming_frames = nullptr, bool trigger_trimming = false);

/**
 * Run verify_gfxr with GFXRECON_FORCE_COMMAND_SERIALIZATION=true. The layer then takes one lock around
 * every call. The capture must equal the same known good as the plain run.
 *
 * @param test_name - the name of the test app to launch
 */
void verify_gfxr_serialized(const char* test_name);

/**
 * Run an application with GFXRECON_CAPTURE_PROCESS_NAME set to a name that does not match the
 * launcher. The layer must load, stay passive, and write no capture file.
 *
 * @param test_name - the name of the test app to launch
 */
void verify_no_capture(const char* test_name);

/**
 * Run an application with capture enabled, then replay the resulting gfxr with gfxrecon-replay, asserting that the
 * replay process exits successfully.
 *
 * Replay is forced offscreen (--swapchain offscreen) so it runs headless against the mock ICD, and any additional
 * arguments are forwarded to the replay tool.
 *
 * @param test_name          - the name of the test app to launch and capture
 * @param extra_replay_args  - additional arguments forwarded verbatim to gfxrecon-replay
 *
 * @note expects the same environment variables as verify_gfxr().
 */
void capture_and_replay(const char* test_name, std::vector<std::string> extra_replay_args = {});

// Capture the app on one driver and replay the capture on the other, with the given extra replay
// arguments, such as a memory translation mode. The drivers are "mock" and "lavapipe". The case
// skips when the replay or capture driver has no manifest configured. Asserts the exit codes.
void capture_on_replay_on(const char*              test_name,
                          const char*              capture_driver,
                          const char*              replay_driver,
                          std::vector<std::string> extra_replay_args = {});

// Capture the app, replay it with a screenshot of one frame, and compare the screenshot against
// known_good/<driver>/<name>_frame_<frame>.png with the RMS rule. The driver is the value of
// GFXRECON_TEST_DRIVER, "mock" when unset.
void verify_screenshot(const char* test_name, unsigned int frame);

// The root mean square difference of two images as a percentage, 0 for identical images and 100
// for black against white. Alpha is dropped. On a load error or a size mismatch, the function
// returns 100 and puts the reason in error.
double rms_difference_percent(const std::string& image_path, const std::string& reference_path, std::string& error);

// Images that differ by no more than this percentage count as the same image.
extern const double kRmsThresholdPercent;

// Make an empty results directory for a case that is not about one app, and return it.
std::filesystem::path prepare_results_directory(const char* name);

// Run one tool from the test directory, such as "gfxrecon-convert", with its output in a log
// file in the results directory of the case, and check how it ended. A tool that dies from a
// signal fails either function, so a crash never passes as a refusal. When log_pattern is not
// empty, the log must match it as a regular expression.
void tool_expect_failure(const char* tool, std::vector<std::string> args, const char* log_pattern);
void tool_expect_success(const char* tool, std::vector<std::string> args, const char* log_pattern);

#endif // GFXRECONSTRUCT_VERIFY_GFXR_H
