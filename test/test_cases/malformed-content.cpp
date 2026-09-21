#include <gtest/gtest.h>

#include "capture_mutations.h"
#include "verify-gfxr.h"

#include <string>
#include <vector>

// The tools on a capture whose structure is valid and whose content is wrong. Each case makes
// one such file in its results directory with the same function that gfxrecon-capture-mutate
// uses, and runs one tool on it. gfxrecon-convert is the first oracle, because it needs no
// driver. gfxrecon-replay on the current driver is the second, for the mutations that reach the
// handle map and the state. A tool must report what it can see, must never crash, must never
// read past a parameter buffer, and must never allocate what a count field claims.

enum class Expect
{
    kSuccess,
    kFailure,
};

struct MalformedContentCase
{
    CaptureMutation mutation;
    const char*     source; // A known-good file. Only the state setup mutation needs a trimmed one.
    const char*     tool;
    Expect          expect;
    const char*     log_pattern;
};

static std::string TestName(const testing::TestParamInfo<MalformedContentCase>& info)
{
    std::string name = std::string(to_string(info.param.mutation)) + "_" + info.param.tool;
    for (char& c : name)
    {
        if (c == '-')
        {
            c = '_';
        }
    }
    return name;
}

class MalformedContent : public testing::TestWithParam<MalformedContentCase>
{};

TEST_P(MalformedContent, ToolReactsAsDocumented)
{
    const MalformedContentCase& row = GetParam();

    const std::filesystem::path results  = prepare_results_directory(TestName({ row, 0 }).c_str());
    const std::filesystem::path bad_file = results / (std::string(to_string(row.mutation)) + ".gfxr");
    std::string                 error;
    ASSERT_TRUE(write_mutated_capture(row.mutation, row.source, bad_file, error)) << error;

    std::vector<std::string> args;
    if (std::string(row.tool) == "gfxrecon-convert")
    {
        args = { "--output", (results / "out.json").string(), bad_file.string() };
    }
    else
    {
        args = { "--swapchain", "offscreen", bad_file.string() };
    }

    if (row.expect == Expect::kSuccess)
    {
        tool_expect_success(row.tool, args, row.log_pattern);
    }
    else
    {
        tool_expect_failure(row.tool, args, row.log_pattern);
    }
}

static const char* const kConvert = "gfxrecon-convert";
static const char* const kReplay  = "gfxrecon-replay";
static const char* const kGood    = "known_good/triangle.gfxr";
static const char* const kTrimmed = "known_good/trigger-trimming_trim_trigger.gfxr";

// What the tools do, and must keep doing.
static const MalformedContentCase kDocumentedCases[] = {
    // An annotation whose label claims more than the block holds is refused with its index.
    { CaptureMutation::kAnnotationLabelPastBlock,
      kGood,
      kConvert,
      Expect::kFailure,
      "Failed to read annotation block" },
    // A block that says it is compressed and is not fails in the decompressor, with its index.
    { CaptureMutation::kCompressionFlagOnUncompressed,
      kGood,
      kConvert,
      Expect::kFailure,
      "Failed to decompress block data" },
    // The converter does not map handles or track state, so a wrong handle id, a draw outside a
    // command buffer and a missing state setup convert as they are. The JSON shows them.
    { CaptureMutation::kHandleNeverCreated, kGood, kConvert, Expect::kSuccess, "" },
    { CaptureMutation::kSecondHandleNeverCreated, kGood, kConvert, Expect::kSuccess, "" },
    { CaptureMutation::kDrawBeforeBeginCommandBuffer, kGood, kConvert, Expect::kSuccess, "" },
    { CaptureMutation::kMissingStateSetup, kTrimmed, kConvert, Expect::kSuccess, "" },
    // A draw outside the recording state goes to the driver as recorded, because replay
    // reproduces the recorded behavior, and the replayer logs an error that names the call, the
    // buffer and the block. The mock takes the draw. Lavapipe faults on it, which is the recorded
    // behavior too, so the row is mock-only in test_environment.cmake.in.
    { CaptureMutation::kDrawBeforeBeginCommandBuffer,
      kGood,
      kReplay,
      Expect::kSuccess,
      "vkCmdDraw records into VkCommandBuffer .* which is not in the recording state \\(block" },
    // The replayer refuses a call whose first object no call created, and names the call, the
    // type and the id, before any driver sees the call. A later object that a replay override
    // reads gets the same refusal. A trimmed capture without its state setup fails the same way on
    // its first call.
    { CaptureMutation::kHandleNeverCreated,
      kGood,
      kReplay,
      Expect::kFailure,
      "vkCmdDraw names a VkCommandBuffer with id .* that no call created" },
    { CaptureMutation::kSecondHandleNeverCreated,
      kGood,
      kReplay,
      Expect::kFailure,
      "vkGetSwapchainImagesKHR names a VkSwapchainKHR with id .* that no call created" },
    { CaptureMutation::kMissingStateSetup,
      kTrimmed,
      kReplay,
      Expect::kFailure,
      "names a Vk[A-Za-z]* with id .* that no call created" },
    // A length field that cannot fit in the parameter buffer is refused before any allocation.
    { CaptureMutation::kCountBomb, kGood, kConvert, Expect::kFailure, "cannot fit in the .* bytes that remain" },
    { CaptureMutation::kCountBomb, kGood, kReplay, Expect::kFailure, "cannot fit in the .* bytes that remain" },
    // A parameter buffer that ends before the call does fails the block, with the block index.
    { CaptureMutation::kParameterBufferShortByOne,
      kGood,
      kConvert,
      Expect::kFailure,
      "the parameter buffer holds .* bytes where a value needs" },
    { CaptureMutation::kParameterBufferShortByHalf,
      kGood,
      kConvert,
      Expect::kFailure,
      "the parameter buffer holds .* bytes where a value needs" },
    { CaptureMutation::kParameterBufferShortByHalf,
      kGood,
      kReplay,
      Expect::kFailure,
      "the parameter buffer holds .* bytes where a value needs" },
    // A block with an ApiCallId that this build does not know is skipped, and the tool says so
    // once with the id and the block, so a capture from a newer layer is not read as a capture
    // with fewer calls.
    { CaptureMutation::kUnknownApiCallId, kGood, kConvert, Expect::kSuccess, "Unknown ApiCallId 0x0001ffff \\(block" },
    { CaptureMutation::kUnknownApiCallId, kGood, kReplay, Expect::kSuccess, "Unknown ApiCallId 0x0001ffff \\(block" },
    // An extension struct this build does not know cannot be skipped, so the block stops there.
    { CaptureMutation::kUnknownStructureType, kGood, kConvert, Expect::kFailure, "unrecognized VkStructureType" },
    { CaptureMutation::kUnknownStructureType, kGood, kReplay, Expect::kFailure, "unrecognized VkStructureType" },
};

INSTANTIATE_TEST_SUITE_P(MalformedContentFiles, MalformedContent, testing::ValuesIn(kDocumentedCases), TestName);
