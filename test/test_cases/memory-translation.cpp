#include <gtest/gtest.h>

#include "verify-gfxr.h"

// Each memory translation mode on the device that made the capture. The modes have their own
// code paths at replay start, so each must at least replay a capture on the same device without
// an error. These cases run on every driver. The realign case also proves that the pre-pass
// replaces the debug messenger callback that the triangle app puts in the instance pNext chain.

TEST(MemoryTranslation, RemapReplaysOnTheSameDevice)
{
    capture_and_replay("triangle", { "-m", "remap" });
}

TEST(MemoryTranslation, RealignReplaysOnTheSameDevice)
{
    capture_and_replay("triangle", { "-m", "realign" });
}

TEST(MemoryTranslation, RebindReplaysOnTheSameDevice)
{
    capture_and_replay("triangle", { "-m", "rebind" });
}
