#pragma once
#include <gtest/gtest.h>
#include "env_utils.h"
#include "true_env_info.h"
#include <algorithm>
#include <cctype>
#include <string>

TEST(UtilTest, envuGetVersion) {
    ASSERT_STREQ(ENVU_VERSION, envuGetVersion());
}

TEST(UtilTest, envuGetVersionInt) {
    ASSERT_EQ(ENVU_VERSION_INT, envuGetVersionAsInt());
}

TEST(UtilTest, envuGetOS) {
    char* os = envuGetOS();
    std::string os_lower(os);
    std::transform(os_lower.begin(), os_lower.end(), os_lower.begin(),
                   [](unsigned char c){ return std::tolower(c); });
    envuFree(os);
    ASSERT_STREQ(TRUE_OS, os_lower.c_str());
}

TEST(UtilTest, envuGetExecutablePath) {
    char* exe_path = envuGetExecutablePath();
    ASSERT_STREQ(TRUE_EXE_PATH, exe_path);
    envuFree(exe_path);
}

TEST(UtilTest, envuGetExecutableDir) {
    char* exe_dir = envuGetExecutableDir();
    ASSERT_STREQ(TRUE_BUILD_DIR, exe_dir);
    envuFree(exe_dir);
}

TEST(UtilTest, envuGetCwd) {
    char* cwd = envuGetCwd();
    ASSERT_STREQ(TRUE_CWD, cwd);
    envuFree(cwd);
}
