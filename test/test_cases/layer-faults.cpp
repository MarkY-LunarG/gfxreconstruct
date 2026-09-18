#include <gtest/gtest.h>

#include "verify-gfxr.h"

#include <string>
#include <vector>

// The capture layer under a fault that the user made: a setting with a wrong value, or a capture
// path that cannot be opened. A wrong value is a warning that names the value, and the layer
// takes the default and captures. A capture file that cannot be opened fails the creation of the
// instance, so the app stops with the layer's message. The rows pin both, so a change to either
// is a visible change.

enum class Expect
{
    kSuccess,
    kFailure,
};

struct LayerFaultCase
{
    const char*         name;
    std::vector<EnvVar> env;
    Expect              expect;
    bool                capture_expected;
    const char*         log_pattern;
};

static std::string TestName(const testing::TestParamInfo<LayerFaultCase>& info)
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

class LayerFault : public testing::TestWithParam<LayerFaultCase>
{};

TEST_P(LayerFault, LayerReactsAsDocumented)
{
    const LayerFaultCase& row = GetParam();
    if (row.expect == Expect::kSuccess)
    {
        app_expect_success("triangle", row.env, row.capture_expected, row.log_pattern);
    }
    else
    {
        app_expect_failure("triangle", row.env, row.log_pattern);
    }
}

// What the layer does, and must keep doing. A row that the layer does not meet yet goes in the
// second list, under the instantiation with the DISABLED_ prefix, with the defect named above it.
static const LayerFaultCase kDocumentedCases[] = {
    { "capture-file-in-missing-directory",
      { { "GFXRECON_CAPTURE_FILE", "/no/such/dir/capture.gfxr" } },
      Expect::kFailure,
      false,
      "fopen\\(/no/such/dir/capture.gfxr, wb\\) failed" },
    { "capture-frames-not-a-range",
      { { "GFXRECON_CAPTURE_FRAMES", "garbage" } },
      Expect::kSuccess,
      true,
      "Ignoring invalid range \"garbage\" for capture frames" },
    { "capture-frames-reversed",
      { { "GFXRECON_CAPTURE_FRAMES", "9-3" } },
      Expect::kSuccess,
      true,
      "Ignoring invalid range \"9-3\" for capture frames, where range start is greater than range end" },
    { "memory-tracking-mode-bogus",
      { { "GFXRECON_MEMORY_TRACKING_MODE", "bogus" } },
      Expect::kSuccess,
      true,
      "Ignoring unrecognized memory tracking mode option value \"bogus\"" },
    { "compression-type-bogus",
      { { "GFXRECON_CAPTURE_COMPRESSION_TYPE", "bogus" } },
      Expect::kSuccess,
      true,
      "Ignoring unrecognized compression type option value \"bogus\"" },
    { "log-level-bogus",
      { { "GFXRECON_LOG_LEVEL", "bogus" } },
      Expect::kSuccess,
      true,
      "Ignoring unrecognized log level option value \"bogus\"" },
};

INSTANTIATE_TEST_SUITE_P(LayerFaults, LayerFault, testing::ValuesIn(kDocumentedCases), TestName);

static const LayerFaultCase kKnownDefects[] = {
    // ParseTrimKeyString takes any text as the key, so a key name that no keyboard has puts the
    // layer in trim mode with a trigger that never fires. The app runs, no capture appears, and
    // nothing is logged. The warning in that function is on the branch for an empty value,
    // which cannot be reached. The row expects a warning that names the value and a normal
    // capture.
    { "capture-trigger-bogus",
      { { "GFXRECON_CAPTURE_TRIGGER", "bogus" } },
      Expect::kSuccess,
      true,
      "Ignoring invalid trim trigger key \"bogus\"" },
};

INSTANTIATE_TEST_SUITE_P(DISABLED_LayerFaultsWithKnownDefects, LayerFault, testing::ValuesIn(kKnownDefects), TestName);
