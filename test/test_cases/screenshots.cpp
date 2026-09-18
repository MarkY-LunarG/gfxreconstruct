#include <gtest/gtest.h>

#include "verify-gfxr.h"

#include <string>

// The pixel tests. Each app renders the same image in every frame, so one frame is enough, and
// frame 5 is well inside the ten frames that an app renders. The reference images live in
// known_good/<driver>/, and only a driver that executes shaders has any. ctest disables these
// cases on the mock, which draws nothing.
static const char* const kScreenshotApps[] = {
    "triangle",
    "multisample-depth",
    "shader-objects",
    "sparse-resources",
};

static const unsigned int kScreenshotFrame = 5;

static std::string TestName(const testing::TestParamInfo<const char*>& info)
{
    std::string name = info.param;
    for (char& c : name)
    {
        if (c == '-')
        {
            c = '_';
        }
    }
    return name;
}

class Screenshot : public testing::TestWithParam<const char*>
{};

TEST_P(Screenshot, MatchesReference)
{
    verify_screenshot(GetParam(), kScreenshotFrame);
}

INSTANTIATE_TEST_SUITE_P(RealDriverApps, Screenshot, testing::ValuesIn(kScreenshotApps), TestName);

// The comparison itself, on the reference images, so it runs on any driver and proves that the
// rule tells two different images apart.
TEST(ScreenshotCompare, SameImageIsZero)
{
    std::string  error;
    const double rms = rms_difference_percent(
        "known_good/lavapipe/triangle_frame_5.png", "known_good/lavapipe/triangle_frame_5.png", error);
    ASSERT_TRUE(error.empty()) << error;
    ASSERT_EQ(rms, 0.0);
}

TEST(ScreenshotCompare, DifferentImagesExceedTheThreshold)
{
    std::string  error;
    const double rms = rms_difference_percent(
        "known_good/lavapipe/triangle_frame_5.png", "known_good/lavapipe/shader-objects_frame_5.png", error);
    ASSERT_TRUE(error.empty()) << error;
    ASSERT_GT(rms, kRmsThresholdPercent);
}

TEST(ScreenshotCompare, MissingImageIsAnError)
{
    std::string  error;
    const double rms = rms_difference_percent(
        "known_good/lavapipe/no-such-image.png", "known_good/lavapipe/triangle_frame_5.png", error);
    ASSERT_FALSE(error.empty());
    ASSERT_EQ(rms, 100.0);
}
