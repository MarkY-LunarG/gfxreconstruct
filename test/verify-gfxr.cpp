// stb_image decodes the PNG screenshots for the pixel comparison. This is the one translation unit
// that holds its implementation.
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#include <stb_image.h>

#include <cmath>
#include <regex>
#if !defined(_WIN32)
#include <sys/wait.h>
#endif
#include "verify-gfxr.h"

#include <gtest/gtest.h>
#include <fstream>
#include <filesystem>
#include <nlohmann/json.hpp>
#include <stdlib.h>

#include <util/logging.h>

// Keys whose values differ between two captures of one app, or between two machines. The comparison
// drops each of them wherever it appears.
static const char* const kIgnoredKeys[] = {
    "api_version",         // The version that the layer reports. It moves with every header update.
    "apiVersion",          // The same value in VkApplicationInfo and VkPhysicalDeviceProperties.
    "hinstance",           // Win32 handles differ per process.
    "hwnd",                //
    "pipelineCacheUUID",   // Driver identity.
    "pipeline_cache_uuid", //
    "ppData",              // Host pointers that vkMapMemory returns.
    "fd",                  // File descriptors from an external memory export.
    "app_name",            // The path of the launcher differs per machine.
};

// A key that starts with one of these names a function pointer, which differs per process.
static const char* const kIgnoredKeyPrefixes[] = { "pfn" };

// A top-level block that holds one of these keys is dropped whole. The header carries the source
// path and the tool version. An annotation carries per-run text.
static const char* const kIgnoredBlockKeys[] = { "header", "annotation" };

// The Android hardware buffer import struct and the AHB properties query carry a "buffer" field that
// is a host pointer. Only that "buffer" must go. The value that names the struct or the call arrives
// first, and the "buffer" key arrives in a later event.
static const char* const kAhbBufferMarkers[] = { "VK_STRUCTURE_TYPE_IMPORT_ANDROID_HARDWARE_BUFFER_INFO_ANDROID",
                                                 "vkGetAndroidHardwareBufferPropertiesANDROID" };

static bool is_ignored_key(const std::string& key)
{
    for (const char* ignored : kIgnoredKeys)
    {
        if (key == ignored)
        {
            return true;
        }
    }
    for (const char* prefix : kIgnoredKeyPrefixes)
    {
        if (key.rfind(prefix, 0) == 0)
        {
            return true;
        }
    }
    return false;
}

bool clean_gfxr_json(int depth, nlohmann::json::parse_event_t event, nlohmann::json& parsed)
{
    // The marker value and the "buffer" key arrive in separate events. This flag carries the state
    // between them, and the "buffer" key resets it.
    static bool skip_next_buffer = false;

    switch (event)
    {
        case nlohmann::json::parse_event_t::key:
        {
            const std::string key = parsed.get<std::string>();
            if (is_ignored_key(key))
            {
                return false;
            }
            if (skip_next_buffer && key == "buffer")
            {
                skip_next_buffer = false;
                return false;
            }
            break;
        }
        case nlohmann::json::parse_event_t::value:
        {
            if (parsed.is_string())
            {
                const std::string value = parsed.get<std::string>();
                for (const char* marker : kAhbBufferMarkers)
                {
                    if (value == marker)
                    {
                        skip_next_buffer = true;
                    }
                }
            }
            break;
        }
        case nlohmann::json::parse_event_t::object_end:
        {
            if (depth == 1)
            {
                for (const char* block_key : kIgnoredBlockKeys)
                {
                    if (parsed.contains(block_key))
                    {
                        return false;
                    }
                }
            }
            break;
        }
        default:
            break;
    }

    return true;
}

#if defined(__linux__) || defined(__APPLE__)
static char const* CONVERT_FILENAME = "gfxrecon-convert";
static char const* REPLAY_FILENAME  = "gfxrecon-replay";
#elif defined(_WIN32)
static char const* CONVERT_FILENAME = "gfxrecon-convert.exe";
static char const* REPLAY_FILENAME  = "gfxrecon-replay.exe";
#endif

// The name of the running gtest case, with the characters that a path cannot hold replaced. The
// name is unique in the runner, so two cases never write the same file, and a parallel ctest run
// is safe. Outside a gtest case, the app name serves.
static std::string case_directory_name(const char* test_name)
{
    std::string name;
    auto        test_info = ::testing::UnitTest::GetInstance()->current_test_info();
    if (test_info != nullptr)
    {
        name = std::string(test_info->test_suite_name()) + "." + test_info->name();
    }
    else
    {
        name = test_name;
    }
    for (char& c : name)
    {
        if (c == '/' || c == '\\' || c == ':')
        {
            c = '_';
        }
    }
    return name;
}

struct Paths
{
    std::filesystem::path base_path{ std::filesystem::current_path() };
    std::filesystem::path working_directory{ base_path };
    std::filesystem::path full_app_directory{ base_path };
    std::filesystem::path full_executable_path;
    std::filesystem::path convert_path{ base_path };
    std::filesystem::path replay_path{ base_path };
    // Every file that a case writes goes under results/<case name>/. known_good/ is read only.
    std::filesystem::path case_directory{ base_path };
    std::filesystem::path capture_path;
    std::filesystem::path known_good_path{ base_path };
    std::filesystem::path app_json_path;
    std::filesystem::path known_good_json_path;

    std::filesystem::path capture_trimming_path;
    std::filesystem::path known_good_trimming_path{ base_path };
    std::filesystem::path app_trimming_json_path;
    std::filesystem::path known_good_trimming_json_path;

    void trimming_paths(char const* test_name, char const* trimming_frames, bool trigger_trimming)
    {
        std::string trimming_suffix;
        if (trimming_frames != nullptr)
        {
            GFXRECON_ASSERT(!trigger_trimming);

            // Trimming suffix is like "_frame_10" or "_frames_10_through_100"
            std::string s_trimming_frames = trimming_frames;
            trimming_suffix               = "_frame";
            std::string range_begin       = "";
            std::string range_end         = "";

            auto index = s_trimming_frames.find("-");
            if (index == std::string::npos)
            {
                range_begin = s_trimming_frames;
            }
            else
            {
                range_begin = s_trimming_frames.substr(0, index);
                range_end   = s_trimming_frames.substr(index + 1);
            }

            if (!range_end.empty())
            {
                trimming_suffix += "s";
            }
            trimming_suffix += "_";
            trimming_suffix += range_begin;
            if (!range_end.empty())
            {
                trimming_suffix += "_through_";
                trimming_suffix += range_end;
            }
        }
        else if (trigger_trimming)
        {
            trimming_suffix = "_trim_trigger";
        }

        std::string capture_trimming_file = test_name + trimming_suffix;
        capture_trimming_file += ".gfxr";
        capture_trimming_path = case_directory / capture_trimming_file;

        known_good_trimming_path.append("known_good");
        known_good_trimming_path.append(capture_trimming_file);

        app_trimming_json_path = std::filesystem::path{ capture_trimming_path };
        app_trimming_json_path.replace_extension(".json");

        known_good_trimming_json_path = std::filesystem::path{ capture_trimming_path };
        known_good_trimming_json_path.replace_extension(".known_good.json");
    }

    Paths(char const* test_name, char const* trimming_frames, bool trigger_trimming)
    {
        working_directory = full_app_directory;
        working_directory.append("res");
        full_app_directory.append("test_apps");

        full_app_directory.append("launcher");
        full_executable_path = full_app_directory;

#ifdef WIN32
        full_executable_path.append("gfxrecon-test-launcher.exe");
#else
        full_executable_path.append("gfxrecon-test-launcher");
#endif

        convert_path.append(CONVERT_FILENAME);
        replay_path.append(REPLAY_FILENAME);

        case_directory.append("results");
        case_directory.append(case_directory_name(test_name));

        std::string gfxr_file_name = test_name + std::string(".gfxr");
        capture_path               = case_directory / gfxr_file_name;

        known_good_path.append("known_good");
        known_good_path.append(gfxr_file_name);

        app_json_path = std::filesystem::path{ capture_path };
        app_json_path.replace_extension(".json");

        known_good_json_path = std::filesystem::path{ capture_path };
        known_good_json_path.replace_extension(".known_good.json");

        if (trimming_frames != nullptr || trigger_trimming)
        {
            trimming_paths(test_name, trimming_frames, trigger_trimming);
        }
    }
};

// destructor unsets all env vars. It helps when the user forget to unset to affect the other tests.
class EnvironmentVariables
{
  private:
    std::unordered_map<std::string, std::string> env_vars;

  public:
    ~EnvironmentVariables()
    {
        for (auto& env_var : env_vars)
        {
#if defined(__linux__) || defined(__APPLE__)
            unsetenv(env_var.first.c_str());
#elif defined(_WIN32)
            _putenv_s(env_var.first.c_str(), "");
#else
#error "Unsupported platform"
#endif
        }
        env_vars.clear();
    }

    void SetEnv(const char* env_name, const char* env_var)
    {
#if defined(__linux__) || defined(__APPLE__)
        ASSERT_EQ(setenv(env_name, env_var, 1), 0) << "set env var: " << env_name << ": " << env_var << " failed.";
#elif defined(_WIN32)
        ASSERT_EQ(_putenv_s(env_name, env_var), 0) << "set env var: " << env_name << ": " << env_var << " failed.";
#else
#error "Unsupported platform"
#endif
        env_vars.insert(std::pair(env_name, env_var));
    }

    void UnsetEnv(const char* env_name)
    {
#if defined(__linux__) || defined(__APPLE__)
        ASSERT_EQ(unsetenv(env_name), 0) << "unset env var: " << env_name << " failed.";
#elif defined(_WIN32)
        ASSERT_EQ(_putenv_s(env_name, ""), 0) << "unset env var: " << env_name << " failed.";
#else
#error "Unsupported platform"
#endif
        auto entry = env_vars.find(env_name);
        if (entry != env_vars.end())
        {
            env_vars.erase(entry);
        }
    }
};

// True when the build has the leak check, so every process runs under AddressSanitizer with
// LeakSanitizer. The ctest environment sets the variable, see test/CMakeLists.txt.
static bool leak_check_is_on()
{
    return std::getenv("GFXRECON_TEST_LEAK_CHECK") != nullptr;
}

int run_command(std::filesystem::path const& working_directory,
                std::filesystem::path const& command,
                std::vector<std::string>     args,
                const std::string&           command_prefix = "")
{
    std::string command_string;
#if !defined(_WIN32)
    if (leak_check_is_on())
    {
        // AddressSanitizer makes every stack frame larger, and the pNext decoder uses one frame
        // per struct in a chain, so the deep-pnext-chain capture overflows the default stack in
        // the tools. Give every process a larger stack. See defect 42 in the test plan.
        command_string += "ulimit -s 262144 && ";
    }
#endif
    command_string += command_prefix;
    command_string += command.string();
    for (auto& arg : args)
    {
        command_string += " ";
        command_string += arg;
    }

    auto previous_path = std::filesystem::current_path();
    std::filesystem::current_path(working_directory);
    auto result = std::system(command_string.c_str());
    std::filesystem::current_path(previous_path);
    return result;
}

// Make an empty results directory for the case. An earlier run's outputs go first. When they stay
// in place and the app writes no capture, the convert step reads the old file and the case passes
// for the wrong reason.
static void prepare_case_directory(const Paths& paths)
{
    std::error_code error;
    std::filesystem::remove_all(paths.case_directory, error);
    std::filesystem::create_directories(paths.case_directory, error);
    ASSERT_FALSE(error) << "could not create the results directory " << paths.case_directory << ": " << error.message();
}

// The driver that the suite runs on. The ctest environment list sets it. A runner started by hand
// without it is on the mock.
static std::string test_driver()
{
    const char* driver = std::getenv("GFXRECON_TEST_DRIVER");
    return (driver != nullptr && driver[0] != '\0') ? driver : "mock";
}

// Capture the app. The layer writes to paths.capture_path.
static void capture_app(EnvironmentVariables& env_vars, const Paths& paths, const char* test_name)
{
    env_vars.SetEnv("GFXRECON_CAPTURE_FILE", paths.capture_path.string().c_str());
    int result = run_command(paths.working_directory, paths.full_executable_path, { test_name });
    ASSERT_EQ(result, 0) << "capture command failed " << paths.full_executable_path << " " << test_name << " in path "
                         << paths.working_directory;
    ASSERT_TRUE(std::filesystem::exists(paths.capture_path)) << "capture file was not produced: " << paths.capture_path;
}

// Replay the capture with an offscreen swapchain and the given extra arguments. The capture layer
// is still in the environment, so GFXRECON_CAPTURE_FILE points at a throwaway path, and a layer
// that loads in the replayer cannot write over the capture it reads.
static void replay_capture(EnvironmentVariables&           env_vars,
                           const Paths&                    paths,
                           const char*                     test_name,
                           const std::vector<std::string>& extra_replay_args)
{
    std::filesystem::path replay_capture_path = paths.case_directory / (test_name + std::string("_replay.gfxr"));
    env_vars.SetEnv("GFXRECON_CAPTURE_FILE", replay_capture_path.string().c_str());

    std::vector<std::string> replay_args = { "--swapchain", "offscreen" };
    replay_args.insert(replay_args.end(), extra_replay_args.begin(), extra_replay_args.end());
    replay_args.push_back(paths.capture_path.string());

    int result = run_command(paths.base_path, paths.replay_path, replay_args);
    ASSERT_EQ(result, 0) << "replay command failed " << paths.replay_path << " for capture " << paths.capture_path
                         << " in path " << paths.base_path;
}

// Point the loader at one driver for the commands that follow. The manifests come from the ctest
// environment list. Returns false, with the reason, when the driver has no manifest configured.
static bool select_driver(EnvironmentVariables& env_vars, const std::string& driver, std::string& reason)
{
    const char* manifest = nullptr;
    if (driver == "mock")
    {
        manifest = std::getenv("GFXRECON_TEST_MOCK_ICD_JSON");
    }
    else if (driver == "lavapipe")
    {
        manifest = std::getenv("GFXRECON_TEST_LAVAPIPE_ICD_JSON");
    }
    else
    {
        reason = "unknown driver \"" + driver + "\"";
        return false;
    }
    if (manifest == nullptr || manifest[0] == '\0')
    {
        reason = "no manifest is configured for the " + driver + " driver";
        return false;
    }
    env_vars.SetEnv("VK_DRIVER_FILES", manifest);
    env_vars.SetEnv("VK_ICD_FILENAMES", manifest);
    if (driver == "mock")
    {
        const char* library = std::getenv("GFXRECON_TEST_MOCK_ICD_LIBRARY");
        env_vars.SetEnv("GFXRECON_TESTAPP_MOCK_ICD", library != nullptr ? library : "");
    }
    else
    {
        // A test app takes a missing variable as "no mock".
        env_vars.UnsetEnv("GFXRECON_TESTAPP_MOCK_ICD");
    }
    return true;
}

// The threshold comes from imageutils.py in VulkanTests. Differences under it are hard to see.
// Differences over it are visible.
const double kRmsThresholdPercent = 1.18;

struct DecodedImage
{
    int                  width  = 0;
    int                  height = 0;
    std::vector<uint8_t> rgb;
};

static bool load_rgb(const std::string& path, DecodedImage& image, std::string& error)
{
    int      channels = 0;
    uint8_t* pixels   = stbi_load(path.c_str(), &image.width, &image.height, &channels, 3);
    if (pixels == nullptr)
    {
        error = path + ": " + (stbi_failure_reason() != nullptr ? stbi_failure_reason() : "load failed");
        return false;
    }
    image.rgb.assign(pixels, pixels + static_cast<size_t>(image.width) * image.height * 3);
    stbi_image_free(pixels);
    return true;
}

double rms_difference_percent(const std::string& image_path, const std::string& reference_path, std::string& error)
{
    DecodedImage image;
    DecodedImage reference;
    if (!load_rgb(image_path, image, error) || !load_rgb(reference_path, reference, error))
    {
        return 100.0;
    }
    if (image.width != reference.width || image.height != reference.height)
    {
        error = image_path + " is " + std::to_string(image.width) + "x" + std::to_string(image.height) + " and " +
                reference_path + " is " + std::to_string(reference.width) + "x" + std::to_string(reference.height);
        return 100.0;
    }
    const size_t component_count = image.rgb.size();
    if (component_count == 0)
    {
        return 0.0;
    }
    double total_square_difference = 0.0;
    for (size_t i = 0; i < component_count; ++i)
    {
        const double difference = static_cast<double>(image.rgb[i]) - static_cast<double>(reference.rgb[i]);
        total_square_difference += difference * difference;
    }
    const double rms = std::sqrt(total_square_difference / static_cast<double>(component_count));
    return 100.0 * rms / 255.0;
}

std::filesystem::path prepare_results_directory(const char* name)
{
    Paths paths{ name, nullptr, false };
    prepare_case_directory(paths);
    return paths.case_directory;
}

// How a child process ended. std::system returns the raw wait status on POSIX and the exit code
// on Windows. On POSIX the command runs under a shell, and a shell reports a child that a signal
// killed as exit code 128 plus the signal number, so that range counts as a signal too. Signal
// numbers stop at 64 on Linux, and 255, the exit code of a fatal replay error, stays an exit.
struct ProcessEnd
{
    bool exited        = false; // False when a signal ended the process.
    int  exit_code     = 0;
    int  signal_number = 0;
};

static ProcessEnd decode_status(int status)
{
    ProcessEnd end;
#if defined(_WIN32)
    end.exited    = true;
    end.exit_code = status;
#else
    if (WIFSIGNALED(status))
    {
        end.signal_number = WTERMSIG(status);
    }
    else if (WIFEXITED(status))
    {
        const int code = WEXITSTATUS(status);
        if (code > 128 && code <= 128 + 64)
        {
            end.signal_number = code - 128;
        }
        else
        {
            end.exited    = true;
            end.exit_code = code;
        }
    }
#endif
    return end;
}

// Run a tool with its output in log_path. The redirection goes through the shell that
// std::system uses on every platform. On POSIX the tool runs under a cap on its address space,
// so a tool that trusts a count from a damaged file and asks for gigabytes fails with
// std::bad_alloc in the tool and not with the out-of-memory killer in the runner. The cap is off
// under the leak check, because AddressSanitizer reserves terabytes of address space for its
// shadow memory, and its own allocator refuses an allocation of gigabytes with a report.
static ProcessEnd run_tool(const char*                     tool,
                           const std::vector<std::string>& args,
                           const std::filesystem::path&    log_path,
                           std::string&                    log)
{
    Paths                 paths{ tool, nullptr, false };
    std::filesystem::path tool_path = paths.base_path / tool;
#if defined(_WIN32)
    tool_path += ".exe";
    const std::string prefix;
#else
    const std::string prefix = leak_check_is_on() ? "" : "ulimit -v 4194304 && ";
#endif
    std::vector<std::string> full_args = args;
    full_args.push_back(">");
    full_args.push_back("\"" + log_path.string() + "\"");
    full_args.push_back("2>&1");
    ProcessEnd end = decode_status(run_command(paths.base_path, tool_path, full_args, prefix));

    std::ifstream log_file{ log_path };
    log.assign(std::istreambuf_iterator<char>(log_file), std::istreambuf_iterator<char>());
    return end;
}

static void
expect_tool_end(const char* tool, std::vector<std::string> args, const char* log_pattern, bool expect_success)
{
    // The log lands next to the other outputs of the running case.
    const std::filesystem::path log_path = Paths{ tool, nullptr, false }.case_directory / (std::string(tool) + ".log");
    std::filesystem::create_directories(log_path.parent_path());

    std::string      log;
    const ProcessEnd end = run_tool(tool, args, log_path, log);
    ASSERT_TRUE(end.exited) << tool << " died from signal " << end.signal_number << ", see " << log_path;
    if (expect_success)
    {
        ASSERT_EQ(end.exit_code, 0) << tool << " exited " << end.exit_code << ", see " << log_path;
    }
    else
    {
        ASSERT_NE(end.exit_code, 0) << tool << " exited 0 and had to refuse, see " << log_path;
    }
    if (log_pattern != nullptr && log_pattern[0] != '\0')
    {
        ASSERT_TRUE(std::regex_search(log, std::regex(log_pattern)))
            << tool << " did not print a message that matches \"" << log_pattern << "\", see " << log_path;
    }
}

static void expect_app_end(const char*                app,
                           const std::vector<EnvVar>& env,
                           bool                       expect_success,
                           bool                       capture_expected,
                           const char*                log_pattern)
{
    EnvironmentVariables env_vars;

    Paths paths{ app, nullptr, false };
    ASSERT_TRUE(std::filesystem::exists(paths.working_directory))
        << "working directory does not exist: " << paths.working_directory;
    prepare_case_directory(paths);

    env_vars.SetEnv("GFXRECON_CAPTURE_FILE", paths.capture_path.string().c_str());
    for (const EnvVar& variable : env)
    {
        if (variable.value != nullptr)
        {
            env_vars.SetEnv(variable.name, variable.value);
        }
        else
        {
            env_vars.UnsetEnv(variable.name);
        }
    }

    const std::filesystem::path log_path = paths.case_directory / (std::string(app) + ".log");
    const ProcessEnd            end      = decode_status(run_command(
        paths.working_directory, paths.full_executable_path, { app, ">", "\"" + log_path.string() + "\"", "2>&1" }));
    std::ifstream               log_file{ log_path };
    const std::string           log{ std::istreambuf_iterator<char>(log_file), std::istreambuf_iterator<char>() };

    ASSERT_TRUE(end.exited) << app << " died from signal " << end.signal_number << ", see " << log_path;
    if (expect_success)
    {
        ASSERT_EQ(end.exit_code, 0) << app << " exited " << end.exit_code << ", see " << log_path;
        ASSERT_EQ(std::filesystem::exists(paths.capture_path), capture_expected)
            << (capture_expected ? "no capture file at " : "a capture file appeared at ") << paths.capture_path;
    }
    else
    {
        ASSERT_NE(end.exit_code, 0) << app << " exited 0 and had to fail, see " << log_path;
    }
    if (log_pattern != nullptr && log_pattern[0] != '\0')
    {
        ASSERT_TRUE(std::regex_search(log, std::regex(log_pattern)))
            << app << " and the layer did not print a message that matches \"" << log_pattern << "\", see " << log_path;
    }
}

void app_expect_success(const char* app, std::vector<EnvVar> env, bool capture_expected, const char* log_pattern)
{
    expect_app_end(app, env, true, capture_expected, log_pattern);
}

void app_expect_failure(const char* app, std::vector<EnvVar> env, const char* log_pattern)
{
    expect_app_end(app, env, false, false, log_pattern);
}

void tool_expect_failure(const char* tool, std::vector<std::string> args, const char* log_pattern)
{
    expect_tool_end(tool, std::move(args), log_pattern, false);
}

void tool_expect_success(const char* tool, std::vector<std::string> args, const char* log_pattern)
{
    expect_tool_end(tool, std::move(args), log_pattern, true);
}

void run_in_background(const char* test_name)
{
    Paths paths{ test_name, nullptr, false };
    run_command(paths.working_directory, paths.full_executable_path, { test_name, "&" });
}

void run_trimming_app(const Paths& paths, const char* test_name, char const* trimming_frames, bool trigger_trimming)
{
    EnvironmentVariables env_vars;

    // To not affect the other tests, set env var programmatically, and unset it when it isn't needed.
    if (trimming_frames != nullptr)
    {
        env_vars.SetEnv("GFXRECON_CAPTURE_FRAMES", trimming_frames);
    }
    else
    {
        GFXRECON_ASSERT(trigger_trimming);
        env_vars.SetEnv("GFXRECON_CAPTURE_TRIGGER", "F12");
    }

    // The trimming run shares the results directory with the full run of the same case.
    std::error_code error;
    std::filesystem::remove(paths.capture_trimming_path, error);

    auto result = run_command(paths.working_directory, paths.full_executable_path, { test_name });
    ASSERT_EQ(result, 0) << "trimming command failed " << paths.full_executable_path << " in path "
                         << paths.working_directory;
    ASSERT_TRUE(std::filesystem::exists(paths.capture_trimming_path))
        << "trimmed capture file was not produced: " << paths.capture_trimming_path;

    env_vars.UnsetEnv("GFXRECON_CAPTURE_FRAMES");
    env_vars.UnsetEnv("GFXRECON_CAPTURE_TRIGGER");

    // convert actual gfxr
    result = run_command(paths.base_path, paths.convert_path, { paths.capture_trimming_path.string() });
    ASSERT_EQ(result, 0) << "trimming command failed " << paths.convert_path << " " << paths.capture_trimming_path
                         << " in path " << paths.base_path;

    // convert known good gfxr
    result = run_command(
        paths.base_path,
        paths.convert_path,
        { paths.known_good_trimming_path.string(), "--output", paths.known_good_trimming_json_path.string() });
    ASSERT_EQ(result, 0) << "trimming command failed " << paths.convert_path << " " << paths.known_good_trimming_path
                         << " in path " << paths.base_path;

    std::ifstream app_trimming_file{ paths.app_trimming_json_path };
    ASSERT_TRUE(app_trimming_file.is_open())
        << "app trimming json file: " << paths.app_trimming_json_path << " would not open";
    auto app_trimming_json = nlohmann::json::parse(app_trimming_file, clean_gfxr_json);

    std::ifstream known_trimming_file{ paths.known_good_trimming_json_path };
    ASSERT_TRUE(known_trimming_file.is_open())
        << "known good trimming json file: " << paths.known_good_trimming_json_path << " would not open ";
    auto known_trimming_json = nlohmann::json::parse(known_trimming_file, clean_gfxr_json);

    auto trimming_diff = nlohmann::json::diff(known_trimming_json, app_trimming_json);
    ASSERT_EQ(trimming_diff.size(), 0) << std::setw(4) << trimming_diff;
}

void verify_gfxr(const char* test_name, char const* trimming_frames, bool trigger_trimming)
{
    EnvironmentVariables env_vars;

    Paths paths{ test_name, trimming_frames, trigger_trimming };
    int   result;

    bool workind_directory_exists = std::filesystem::exists(paths.working_directory);
    ASSERT_TRUE(workind_directory_exists) << "working directory does not exist: " << paths.working_directory;

    prepare_case_directory(paths);

    // run app
    env_vars.SetEnv("GFXRECON_CAPTURE_FILE", paths.capture_path.string().c_str());
    result = run_command(paths.working_directory, paths.full_executable_path, { test_name });
    ASSERT_EQ(result, 0) << "command failed " << paths.full_executable_path << " " << test_name << " in path "
                         << paths.working_directory;
    ASSERT_TRUE(std::filesystem::exists(paths.capture_path)) << "capture file was not produced: " << paths.capture_path;

    // convert actual gfxr
    result = run_command(paths.base_path, paths.convert_path, { paths.capture_path.string() });
    ASSERT_EQ(result, 0) << "command failed " << paths.convert_path << " " << paths.capture_path << " in path "
                         << paths.base_path;

    // convert known good gfxr, into the results directory, so known_good/ stays read only
    result = run_command(paths.base_path,
                         paths.convert_path,
                         { paths.known_good_path.string(), "--output", paths.known_good_json_path.string() });
    ASSERT_EQ(result, 0) << "command failed " << paths.convert_path << " " << paths.known_good_path << " in path "
                         << paths.base_path;

    std::ifstream app_file{ paths.app_json_path };
    ASSERT_TRUE(app_file.is_open()) << "app json file: " << paths.app_json_path << " would not open";
    auto app_json = nlohmann::json::parse(app_file, clean_gfxr_json);

    std::ifstream known_file{ paths.known_good_json_path };
    ASSERT_TRUE(known_file.is_open()) << "known good json file: " << paths.known_good_json_path << " would not open";
    auto known_json = nlohmann::json::parse(known_file, clean_gfxr_json);

    auto diff = nlohmann::json::diff(known_json, app_json);
    ASSERT_EQ(diff.size(), 0) << std::setw(4) << diff;

    if (trimming_frames || trigger_trimming)
    {
        run_trimming_app(paths, test_name, trimming_frames, trigger_trimming);
    }
}

void verify_gfxr_serialized(const char* test_name)
{
    EnvironmentVariables env_vars;
    env_vars.SetEnv("GFXRECON_FORCE_COMMAND_SERIALIZATION", "true");
    verify_gfxr(test_name);
}

void verify_no_capture(const char* test_name)
{
    EnvironmentVariables env_vars;

    Paths paths{ test_name, nullptr, false };

    bool working_directory_exists = std::filesystem::exists(paths.working_directory);
    ASSERT_TRUE(working_directory_exists) << "working directory does not exist: " << paths.working_directory;

    prepare_case_directory(paths);

    // The launcher is named gfxrecon-test-launcher, so this name never matches.
    env_vars.SetEnv("GFXRECON_CAPTURE_PROCESS_NAME", "gfxrecon-no-such-process");
    env_vars.SetEnv("GFXRECON_CAPTURE_FILE", paths.capture_path.string().c_str());
    int result = run_command(paths.working_directory, paths.full_executable_path, { test_name });
    ASSERT_EQ(result, 0) << "command failed " << paths.full_executable_path << " " << test_name << " in path "
                         << paths.working_directory;
    ASSERT_FALSE(std::filesystem::exists(paths.capture_path))
        << "capture file was produced with a process name that does not match: " << paths.capture_path;
}

void capture_and_replay(const char* test_name, std::vector<std::string> extra_replay_args)
{
    EnvironmentVariables env_vars;

    Paths paths{ test_name, nullptr, false };

    bool working_directory_exists = std::filesystem::exists(paths.working_directory);
    ASSERT_TRUE(working_directory_exists) << "working directory does not exist: " << paths.working_directory;

    prepare_case_directory(paths);

    // Asserts only that the replay tool exits 0: no crash, no assertion, no replay error.
    ASSERT_NO_FATAL_FAILURE(capture_app(env_vars, paths, test_name));
    ASSERT_NO_FATAL_FAILURE(replay_capture(env_vars, paths, test_name, extra_replay_args));
}

void capture_on_replay_on(const char*              test_name,
                          const char*              capture_driver,
                          const char*              replay_driver,
                          std::vector<std::string> extra_replay_args)
{
    EnvironmentVariables env_vars;

    Paths paths{ test_name, nullptr, false };

    bool working_directory_exists = std::filesystem::exists(paths.working_directory);
    ASSERT_TRUE(working_directory_exists) << "working directory does not exist: " << paths.working_directory;

    std::string reason;
    if (!select_driver(env_vars, capture_driver, reason))
    {
        GTEST_SKIP() << reason;
    }
    prepare_case_directory(paths);
    ASSERT_NO_FATAL_FAILURE(capture_app(env_vars, paths, test_name));

    if (!select_driver(env_vars, replay_driver, reason))
    {
        GTEST_SKIP() << reason;
    }
    ASSERT_NO_FATAL_FAILURE(replay_capture(env_vars, paths, test_name, extra_replay_args));
}

void verify_screenshot(const char* test_name, unsigned int frame)
{
    EnvironmentVariables env_vars;

    Paths paths{ test_name, nullptr, false };

    bool working_directory_exists = std::filesystem::exists(paths.working_directory);
    ASSERT_TRUE(working_directory_exists) << "working directory does not exist: " << paths.working_directory;

    const std::string     image_name      = test_name + std::string("_frame_") + std::to_string(frame) + ".png";
    std::filesystem::path screenshot_path = paths.case_directory / image_name;
    std::filesystem::path reference_path  = paths.base_path / "known_good" / test_driver() / image_name;
    ASSERT_TRUE(std::filesystem::exists(reference_path))
        << "no reference image for this driver: " << reference_path << ". Run the case, then copy the screenshot from "
        << paths.case_directory << " to test/known_good/" << test_driver() << "/ if it is correct.";

    prepare_case_directory(paths);

    ASSERT_NO_FATAL_FAILURE(capture_app(env_vars, paths, test_name));
    ASSERT_NO_FATAL_FAILURE(replay_capture(env_vars,
                                           paths,
                                           test_name,
                                           { "--screenshots",
                                             std::to_string(frame),
                                             "--screenshot-format",
                                             "png",
                                             "--screenshot-dir",
                                             paths.case_directory.string(),
                                             "--screenshot-prefix",
                                             test_name }));
    ASSERT_TRUE(std::filesystem::exists(screenshot_path)) << "screenshot was not produced: " << screenshot_path;

    std::string  error;
    const double rms = rms_difference_percent(screenshot_path.string(), reference_path.string(), error);
    ASSERT_TRUE(error.empty()) << error;
    ASSERT_LE(rms, kRmsThresholdPercent) << "screenshot " << screenshot_path << " differs from " << reference_path
                                         << " by " << rms << " percent";
}
