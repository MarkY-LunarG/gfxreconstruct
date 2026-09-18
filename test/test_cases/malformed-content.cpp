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
    { CaptureMutation::kDrawBeforeBeginCommandBuffer, kGood, kConvert, Expect::kSuccess, "" },
    { CaptureMutation::kMissingStateSetup, kTrimmed, kConvert, Expect::kSuccess, "" },
};

INSTANTIATE_TEST_SUITE_P(MalformedContentFiles, MalformedContent, testing::ValuesIn(kDocumentedCases), TestName);

// What the tools must do and do not do yet. Each group names its defect. Move a row up when its
// fix lands.
static const MalformedContentCase kKnownDefects[] = {
    // The decoder allocates the number of elements that an array length field claims. A length
    // of 0x7fffffff is gigabytes, and the tools end in std::bad_alloc: convert aborts, replay
    // stops with only the exception text. The decoder must compare the length with the bytes
    // that remain in the parameter buffer and refuse the block.
    { CaptureMutation::kCountBomb, kGood, kConvert, Expect::kFailure, "block" },
    { CaptureMutation::kCountBomb, kGood, kReplay, Expect::kFailure, "block" },
    // The decoder reads a parameter past the end of its buffer and says nothing. A buffer cut in
    // half decodes vkCmdDraw with an instanceCount of 32610 from whatever follows it in memory,
    // and replay runs that draw. The decoder must stop the block with an error.
    { CaptureMutation::kParameterBufferShortByOne, kGood, kConvert, Expect::kFailure, "block" },
    { CaptureMutation::kParameterBufferShortByHalf, kGood, kConvert, Expect::kFailure, "block" },
    { CaptureMutation::kParameterBufferShortByHalf, kGood, kReplay, Expect::kFailure, "block" },
    // The replayer faults on a command buffer handle that no call created, instead of reporting
    // the id.
    { CaptureMutation::kHandleNeverCreated, kGood, kReplay, Expect::kFailure, "handle|id" },
    // The replayer faults on a trimmed capture whose state setup is gone, instead of reporting
    // the resources it cannot find.
    { CaptureMutation::kMissingStateSetup, kTrimmed, kReplay, Expect::kFailure, "handle|id|resource" },
    // The replayer passes a draw outside a command buffer to the driver. The mock ICD takes any
    // call, and lavapipe faults. The replayer must refuse the call with a message before any
    // driver sees it, so the row expects the same failure on every driver.
    { CaptureMutation::kDrawBeforeBeginCommandBuffer, kGood, kReplay, Expect::kFailure, "command buffer" },
    // After an unknown sType in a pNext chain the decoder reports it and then reads on out of
    // step, until a length field is garbage and std::bad_alloc ends the tool. The encoding has
    // no size for a struct the decoder does not know, so the block must stop at the error.
    { CaptureMutation::kUnknownStructureType, kGood, kConvert, Expect::kFailure, "unrecognized VkStructureType" },
    { CaptureMutation::kUnknownStructureType, kGood, kReplay, Expect::kFailure, "unrecognized VkStructureType" },
    // A block with an ApiCallId that this build does not know is dropped without a word. The
    // tools must say so once, with the id, so a capture from a newer layer is not read as a
    // capture with fewer calls.
    { CaptureMutation::kUnknownApiCallId, kGood, kConvert, Expect::kSuccess, "[Uu]nknown|[Uu]nrecognized" },
    { CaptureMutation::kUnknownApiCallId, kGood, kReplay, Expect::kSuccess, "[Uu]nknown|[Uu]nrecognized" },
};

INSTANTIATE_TEST_SUITE_P(DISABLED_MalformedContentFilesWithKnownDefects,
                         MalformedContent,
                         testing::ValuesIn(kKnownDefects),
                         TestName);
