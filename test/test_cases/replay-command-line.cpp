#include <gtest/gtest.h>

#include "verify-gfxr.h"

#include <fstream>
#include <string>
#include <vector>

// gfxrecon-replay on a command line that is wrong. A fatal problem, such as a file that does not
// exist or an option with no meaning, ends the tool with a message and a non-zero exit. A value
// that is not valid for an option with a fixed set of values is a warning, and the tool takes
// the default and runs. The rows pin both, so a change to either is a visible change.

enum class Expect
{
    kSuccess,
    kFailure,
};

struct CommandLineCase
{
    const char*              name;
    std::vector<std::string> args;
    Expect                   expect;
    const char*              log_pattern;
};

static std::string TestName(const testing::TestParamInfo<CommandLineCase>& info)
{
    std::string name = info.param.name;
    for (char& c : name)
    {
        if (c == '-')
        {
            c = '_';
        }
    }
    return name;
}

// A file that is a good capture, and one that is not JSON, both in the results directory.
static const char* const kGoodCapture = "known_good/triangle.gfxr";

class ReplayCommandLine : public testing::TestWithParam<CommandLineCase>
{};

TEST_P(ReplayCommandLine, ToolReactsAsDocumented)
{
    const CommandLineCase& row = GetParam();

    const std::filesystem::path results = prepare_results_directory(TestName({ row, 0 }).c_str());
    {
        std::ofstream not_json{ results / "not.json" };
        not_json << "{ this is not json";
    }

    std::vector<std::string> args;
    for (const std::string& arg : row.args)
    {
        // The row names the file that is not JSON by a placeholder, because its path is only known
        // at test time.
        args.push_back(arg == "<not-json>" ? (results / "not.json").string() : arg);
    }

    if (row.expect == Expect::kSuccess)
    {
        tool_expect_success("gfxrecon-replay", args, row.log_pattern);
    }
    else
    {
        tool_expect_failure("gfxrecon-replay", args, row.log_pattern);
    }
}

// What the tool does, and must keep doing. A row that the tool does not meet yet goes in the
// second list, under the instantiation with the DISABLED_ prefix, with the defect named above it.
static const CommandLineCase kDocumentedCases[] = {
    // Fatal: the tool cannot do what the command line asks.
    { "unknown-option",
      { "--swapchain", "offscreen", "--no-such-option", kGoodCapture },
      Expect::kFailure,
      "Invalid command-line setting '--no-such-option'" },
    { "missing-file", { "--swapchain", "offscreen", "/no/such/file.gfxr" }, Expect::kFailure, "Failed to open file" },
    { "no-file", { "--swapchain", "offscreen" }, Expect::kFailure, "Usage:" },
    { "two-files", { "--swapchain", "offscreen", kGoodCapture, kGoodCapture }, Expect::kFailure, "Usage:" },
    { "sgfs-out-of-range",
      { "--swapchain", "offscreen", "--sgfs", "5", kGoodCapture },
      Expect::kFailure,
      "Unexpected value after '--skip-get-fence-status': '5'" },
    { "dump-resources-not-json",
      { "--swapchain", "offscreen", "--dump-resources", "<not-json>", kGoodCapture },
      Expect::kFailure,
      "parse error" },
    { "dump-resources-missing-file",
      { "--swapchain", "offscreen", "--dump-resources", "/no/such.json", kGoodCapture },
      Expect::kFailure,
      "Could not open \"/no/such.json\"" },
    // A value outside the fixed set of an option: a warning, the default, and a normal run.
    { "memory-translation-bogus",
      { "--swapchain", "offscreen", "-m", "bogus", kGoodCapture },
      Expect::kSuccess,
      "The value \"bogus\" is not valid for -m" },
    { "swapchain-bogus",
      { "--swapchain", "bogus", kGoodCapture },
      Expect::kSuccess,
      "The value \"bogus\" is not valid for --swapchain" },
    { "wsi-bogus",
      { "--swapchain", "offscreen", "--wsi", "bogus", kGoodCapture },
      Expect::kSuccess,
      "Ignoring unrecognized wsi option \"bogus\"" },
    { "screenshot-format-bogus",
      { "--swapchain", "offscreen", "--screenshots", "1", "--screenshot-format", "bogus", kGoodCapture },
      Expect::kSuccess,
      "Ignoring unrecognized screenshot format option \"bogus\"" },
    { "screenshots-bogus",
      { "--swapchain", "offscreen", "--screenshots", "bogus", kGoodCapture },
      Expect::kSuccess,
      "Ignoring invalid range \"bogus\" for screenshot frames" },
};

INSTANTIATE_TEST_SUITE_P(ReplayCommandLines, ReplayCommandLine, testing::ValuesIn(kDocumentedCases), TestName);

static const CommandLineCase kKnownDefects[] = {
    // A value that is not a number after --sgfs ends the tool with the text "stoi", which is the
    // message of the exception that std::stoi threw, and nothing names the option or the value.
    { "sgfs-not-a-number",
      { "--swapchain", "offscreen", "--sgfs", "bogus", kGoodCapture },
      Expect::kFailure,
      "skip-get-fence-status" },
};

INSTANTIATE_TEST_SUITE_P(DISABLED_ReplayCommandLinesWithKnownDefects,
                         ReplayCommandLine,
                         testing::ValuesIn(kKnownDefects),
                         TestName);
