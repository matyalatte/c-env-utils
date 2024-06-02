#include <stdio.h>
#include <gtest/gtest.h>
#include "util_tests.hpp"
#include "true_env_info.h"

int main(int argc, char* argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    printf("true_env_info.h:\n");
    printf("  build dir: %s\n", TRUE_BUILD_DIR);
    printf("  exe path: %s\n", TRUE_EXE_PATH);
    printf("  cwd: %s\n", TRUE_CWD);
    printf("  os: %s\n", TRUE_OS);
    return RUN_ALL_TESTS();
}
