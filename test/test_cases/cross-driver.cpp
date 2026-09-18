#include <gtest/gtest.h>

#include "verify-gfxr.h"

// Capture on one driver, replay on the other. The memory properties of the mock and lavapipe
// differ, which is what the memory translation modes exist for. The mock has six memory types
// and lavapipe has one. These cases need lavapipe, so ctest disables them on the mock.

TEST(CrossDriver, MockCaptureReplaysOnLavapipe)
{
    capture_on_replay_on("triangle", "mock", "lavapipe");
}

TEST(CrossDriver, MockCaptureRebindsOnLavapipe)
{
    capture_on_replay_on("triangle", "mock", "lavapipe", { "-m", "rebind" });
}

// The mock advertises a lazily allocated memory type and lavapipe has none, so remap has no
// mapping for it and the replayer stops with "Failed to find valid memory type mappings". That
// is the documented behavior. The case waits for a harness function that expects a failure and
// checks the message.
TEST(CrossDriver, DISABLED_MockCaptureRemapsOnLavapipe)
{
    capture_on_replay_on("triangle", "mock", "lavapipe", { "-m", "remap" });
}

// The realign pre-pass creates its instance with the captured VkInstanceCreateInfo as is. The
// pNext chain holds the app's VkDebugUtilsMessengerCreateInfoEXT with the app's callback pointer,
// and the loader calls it for its first message, so the replayer faults. The replay path replaces
// that pointer and the pre-pass must do the same. Enable both realign cases with that fix.
TEST(CrossDriver, DISABLED_MockCaptureRealignsOnLavapipe)
{
    capture_on_replay_on("triangle", "mock", "lavapipe", { "-m", "realign" });
}

TEST(CrossDriver, LavapipeCaptureReplaysOnMock)
{
    capture_on_replay_on("triangle", "lavapipe", "mock");
}

TEST(CrossDriver, LavapipeCaptureRemapsOnMock)
{
    capture_on_replay_on("triangle", "lavapipe", "mock", { "-m", "remap" });
}

TEST(CrossDriver, DISABLED_LavapipeCaptureRealignsOnMock)
{
    capture_on_replay_on("triangle", "lavapipe", "mock", { "-m", "realign" });
}

TEST(CrossDriver, LavapipeCaptureRebindsOnMock)
{
    capture_on_replay_on("triangle", "lavapipe", "mock", { "-m", "rebind" });
}
