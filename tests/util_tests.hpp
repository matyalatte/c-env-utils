#pragma once
#include <gtest/gtest.h>
#include <algorithm>
#include <cctype>
#include <string>
#include <vector>
#include <utility>
#include "env_utils.h"
#include "env_utils_windows.h"
#include "true_env_info.h"

TEST(UtilTest, envuGetVersion) {
    ASSERT_STREQ(ENVU_VERSION, envuGetVersion());
}

TEST(UtilTest, envuGetVersionInt) {
    ASSERT_EQ(ENVU_VERSION_INT, envuGetVersionAsInt());
}

#ifdef _WIN32
TEST(UtilTest, envuUTF8toUTF16) {
    std::vector<std::pair<const char*, const wchar_t*>> cases = {
        { NULL, NULL},
        { "", L"" },
        { "abc", L"abc" },
    };
    for (auto c : cases) {
        wchar_t *wstr = envuUTF8toUTF16(c.first);
        EXPECT_STREQ(c.second, wstr) << "  c.first: " << c.first << std::endl;
        envuFree(wstr);
    }
}

TEST(UtilTest, envuUTF16toUTF8) {
    std::vector<std::pair<const wchar_t*, const char*>> cases = {
        { NULL, NULL},
        { L"", "" },
        { L"abc", "abc" },
    };
    for (auto c : cases) {
        char* str = envuUTF16toUTF8(c.first);
        EXPECT_STREQ(c.second, str) << "  c.first: " << c.first << std::endl;
        envuFree(str);
    }
}
#endif

TEST(UtilTest, envuGetOS) {
    char* os = envuGetOS();
    std::string os_lower(os);
    std::transform(os_lower.begin(), os_lower.end(), os_lower.begin(),
                   [](unsigned char c){ return (unsigned char)std::tolower(c); });
    envuFree(os);
    ASSERT_STREQ(TRUE_OS, os_lower.c_str());
}

// TODO: test with long paths
// TODO: test with unicode strings
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

TEST(UtilTest, envuSetCwd) {
    int ret = envuSetCwd(TRUE_HOME);
    EXPECT_EQ(0, ret);

    // move cwd to home directory
    char* cwd = envuGetCwd();
    ASSERT_STREQ(TRUE_HOME, cwd);
    envuFree(cwd);

    // restore cwd
    ret = envuSetCwd(TRUE_CWD);
    EXPECT_EQ(0, ret);
    cwd = envuGetCwd();
    ASSERT_STREQ(TRUE_CWD, cwd);
    envuFree(cwd);
}

TEST(UtilTest, envuSetCwdNull) {
    int ret = envuSetCwd(NULL);
    EXPECT_EQ(-1, ret);

    // check cwd
    char *cwd = envuGetCwd();
    ASSERT_STREQ(TRUE_CWD, cwd);
    envuFree(cwd);
}

TEST(UtilTest, envuSetCwdEmpty) {
    int ret = envuSetCwd("");
    EXPECT_EQ(-1, ret);

    // check cwd
    char *cwd = envuGetCwd();
    ASSERT_STREQ(TRUE_CWD, cwd);
    envuFree(cwd);
}

TEST(UtilTest, envuGetHome) {
    char* home = envuGetHome();
    ASSERT_STREQ(TRUE_HOME, home);
    envuFree(home);
}

#ifdef _WIN32
TEST(UtilTest, envuGetHomeWithoutUserprofile) {
    char* userprofile = envuGetEnv("USERPROFILE");
    envuSetEnv("USERPROFILE", NULL);

    char* home = envuGetHome();
    ASSERT_STREQ(TRUE_HOME, home);
    envuFree(home);

    envuSetEnv("USERPROFILE", userprofile);
    envuFree(userprofile);
}
#endif

TEST(UtilTest, envuGetUsername) {
    char* username = envuGetUsername();
    ASSERT_STREQ(TRUE_USERNAME, username);
    envuFree(username);
}

TEST(UtilTest, envuGetEnvNull) {
    char* env = envuGetEnv(NULL);
    ASSERT_EQ(NULL, env);
}

TEST(UtilTest, envuGetEnvEmpty) {
    char* env = envuGetEnv("");
    ASSERT_EQ(NULL, env);
}

TEST(UtilTest, envuSetEnvNull) {
    int ret;
    ret = envuSetEnv(NULL, "test");
    EXPECT_EQ(-1, ret);
    envuSetEnv("test", NULL);
    EXPECT_EQ(-1, ret);
}

TEST(UtilTest, envuSetEnvEmpty) {
    // This should do nothing.
    int ret = envuSetEnv("", "test");
    EXPECT_EQ(-1, ret);
    char* env = envuGetEnv("");
    ASSERT_EQ(NULL, env);
}

TEST(UtilTest, envuSetEnvNew) {
    // confirm that there is no variable "NO_ONE_USE_THIS_VAR"
    char* env = envuGetEnv("NO_ONE_USE_THIS_VAR");
    ASSERT_STREQ(NULL, env);
    envuFree(env);

    // set the variable
    int ret = envuSetEnv("NO_ONE_USE_THIS_VAR", "STUPID");
    EXPECT_EQ(0, ret);
    env = envuGetEnv("NO_ONE_USE_THIS_VAR");
    ASSERT_STREQ("STUPID", env);
    envuFree(env);

    // set NULL to the variable
    ret = envuSetEnv("NO_ONE_USE_THIS_VAR", NULL);
    EXPECT_EQ(0, ret);
    env = envuGetEnv("NO_ONE_USE_THIS_VAR");
    ASSERT_STREQ(NULL, env);
    envuFree(env);
}

#ifdef _WIN32
TEST(UtilTest, envuSetEnvEmptyWin) {
    int ret = envuSetEnv("NO_ONE_USE_THIS_VAR", "STUPID");
    EXPECT_EQ(0, ret);
    char *env = envuGetEnv("NO_ONE_USE_THIS_VAR");
    ASSERT_STREQ("STUPID", env);
    envuFree(env);

    // An empty string should remove a variable on windows
    ret = envuSetEnv("NO_ONE_USE_THIS_VAR", "");
    EXPECT_EQ(0, ret);
    env = envuGetEnv("NO_ONE_USE_THIS_VAR");
    ASSERT_STREQ(NULL, env);
    envuFree(env);
}
#else
TEST(UtilTest, envuSetEnvEmptyUnix) {
    int ret = envuSetEnv("NO_ONE_USE_THIS_VAR", "STUPID");
    EXPECT_EQ(0, ret);
    char *env = envuGetEnv("NO_ONE_USE_THIS_VAR");
    ASSERT_STREQ("STUPID", env);
    envuFree(env);

    // An empty string should be acceptable on unix
    ret = envuSetEnv("NO_ONE_USE_THIS_VAR", "");
    EXPECT_EQ(0, ret);
    env = envuGetEnv("NO_ONE_USE_THIS_VAR");
    ASSERT_STREQ("", env);
    envuFree(env);
}
#endif
