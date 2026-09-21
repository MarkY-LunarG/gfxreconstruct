#include <gtest/gtest.h>

#include "verify-gfxr.h"

#include <cstdlib>

// The leak check. A build with GFXRECON_ENABLE_SANITIZERS runs every process of a case under
// LeakSanitizer, and the ctest environment turns a leak report into an exit code of 23, so the
// case fails at its exit-code assertion. This case proves that the check runs: the app leaks one
// block on request, and the case requires the failure and the report. GFXRECON_TEST_LEAK_CHECK is
// in the environment when the build has the check, and the case skips without it, so a build
// without sanitizers passes the suite.
TEST(LeakCheck, LeakedBlockFailsTheApp)
{
    if (std::getenv("GFXRECON_TEST_LEAK_CHECK") == nullptr)
    {
        GTEST_SKIP() << "the build has no leak check; configure with -DGFXRECON_ENABLE_SANITIZERS=ON";
    }
    app_expect_failure("triangle", { { "GFXRECON_TESTAPP_LEAK", "1" } }, "LeakSanitizer: detected memory leaks");
}
