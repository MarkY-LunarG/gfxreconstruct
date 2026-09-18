/*
** Copyright (c) 2026 LunarG, Inc.
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

#include <gtest/gtest.h>

#include "capture_mutations.h"
#include "verify-gfxr.h"

#include <string>
#include <vector>

// The tools on a damaged capture. Each case makes one bad file from known_good/triangle.gfxr in
// its results directory, so nothing damaged is committed, and runs one tool on it. A tool must
// refuse a file it cannot read with a message and a non-zero exit, and must never crash. A file
// that ends early is the common case of a capture through an application crash, so a tool must
// process what is there, exit 0, and say that the file ended inside a block when it did.

enum class Expect
{
    kSuccess,
    kFailure,
};

struct BadFileCase
{
    CaptureMutation mutation;
    const char*     tool;
    Expect          expect;
    const char*     log_pattern;
};

static std::string TestName(const testing::TestParamInfo<BadFileCase>& info)
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

class BadFile : public testing::TestWithParam<BadFileCase>
{};

TEST_P(BadFile, ToolReactsAsDocumented)
{
    const BadFileCase& row = GetParam();

    const std::filesystem::path results  = prepare_results_directory(TestName({ row, 0 }).c_str());
    const std::filesystem::path bad_file = results / (std::string(to_string(row.mutation)) + ".gfxr");
    std::string                 error;
    ASSERT_TRUE(write_mutated_capture(row.mutation, "known_good/triangle.gfxr", bad_file, error)) << error;

    std::vector<std::string> args;
    if (std::string(row.tool) == "gfxrecon-convert")
    {
        args = { "--output", (results / "out.json").string(), bad_file.string() };
    }
    else if (std::string(row.tool) == "gfxrecon-replay")
    {
        args = { "--swapchain", "offscreen", bad_file.string() };
    }
    else
    {
        args = { bad_file.string() };
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
static const char* const kInfo    = "gfxrecon-info";
static const char* const kReplay  = "gfxrecon-replay";

// What the tools do today, and what they must keep doing.
static const BadFileCase kDocumentedCases[] = {
    // A cut on a block boundary looks like a normal end of file. No tool can tell, and none does.
    { CaptureMutation::kTruncatedAtBlock, kConvert, Expect::kSuccess, "" },
    { CaptureMutation::kTruncatedAtBlock, kInfo, Expect::kSuccess, "" },
    { CaptureMutation::kTruncatedAtBlock, kReplay, Expect::kSuccess, "" },
    // A cut inside a block is the capture of an application that crashed. The tools take what
    // is there and say so.
    { CaptureMutation::kTruncatedInBlock, kConvert, Expect::kSuccess, "Incomplete block at end of file" },
    { CaptureMutation::kTruncatedInBlock, kInfo, Expect::kSuccess, "Incomplete block at end of file" },
    { CaptureMutation::kTruncatedInBlock, kReplay, Expect::kSuccess, "Incomplete block at end of file" },
    { CaptureMutation::kBadCompressedPayload, kConvert, Expect::kFailure, "Failed to decompress block data" },
    { CaptureMutation::kBadCompressedPayload, kInfo, Expect::kFailure, "Failed to decompress block data" },
    { CaptureMutation::kBadCompressedPayload, kReplay, Expect::kFailure, "Failed to decompress block data" },
    { CaptureMutation::kBadMagic, kInfo, Expect::kFailure, "invalid four character code" },
    { CaptureMutation::kBadMagic, kReplay, Expect::kFailure, "invalid four character code" },
};

INSTANTIATE_TEST_SUITE_P(BadFiles, BadFile, testing::ValuesIn(kDocumentedCases), TestName);

// What the tools must do and do not do yet. Each row names the defect. Move a row up when its
// fix lands.
static const BadFileCase kKnownDefects[] = {
    // gfxrecon-convert prints the message and then exits 0.
    { CaptureMutation::kBadMagic, kConvert, Expect::kFailure, "invalid four character code" },
    // The block parser allocates the size that the block header claims before any check, so a
    // corrupt size raises std::bad_alloc. gfxrecon-convert and gfxrecon-info abort on it with
    // no message, and gfxrecon-replay reports only "std::bad_alloc".
    { CaptureMutation::kBadBlockSize, kConvert, Expect::kFailure, "block" },
    { CaptureMutation::kBadBlockSize, kInfo, Expect::kFailure, "block" },
    { CaptureMutation::kBadBlockSize, kReplay, Expect::kFailure, "block" },
    // The format string of the version error in ValidateFileHeader ends in a lone "%", so the
    // logger asserts before it prints, and every tool aborts.
    { CaptureMutation::kBadVersion, kConvert, Expect::kFailure, "later than currently supported version" },
    { CaptureMutation::kBadVersion, kInfo, Expect::kFailure, "later than currently supported version" },
    { CaptureMutation::kBadVersion, kReplay, Expect::kFailure, "later than currently supported version" },
};

INSTANTIATE_TEST_SUITE_P(DISABLED_BadFilesWithKnownDefects, BadFile, testing::ValuesIn(kKnownDefects), TestName);
