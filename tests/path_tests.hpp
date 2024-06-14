#pragma once
// Tests for envu*EnvPaths functions and the functions that take paths

#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <utility>
#include "env_utils.h"
#include "true_env_info.h"

// TODO: test with long paths
// TODO: test with unicode strings

TEST(PathTest, envuFileExists) {
    std::vector<std::pair<const char*, int>> cases = {
        { NULL, 0 },
        { "", 0 },
        { TRUE_EXE_PATH, 1 },
        { TRUE_BUILD_DIR, 0 },
        // TODO: test with relative paths
    };
    for (auto c : cases) {
        EXPECT_EQ(c.second, envuFileExists(c.first)) << "  c.first: " << c.first << std::endl;
    }
}

TEST(PathTest, envuGetFullPathAbsolute) {
    std::vector<std::pair<const char*, const char*>> cases = {
#ifdef _WIN32
        // Note: (WIN_DRIVE ":\\") == ("C:\\" on C drive).
        { WIN_DRIVE ":\\usr\\lib", WIN_DRIVE ":\\usr\\lib" },
        { WIN_DRIVE ":\\usr\\lib\\", WIN_DRIVE ":\\usr\\lib" },
        { WIN_DRIVE ":\\usr\\lib\\.", WIN_DRIVE ":\\usr\\lib" },
        { WIN_DRIVE ":\\usr\\lib\\..", WIN_DRIVE ":\\usr" },
        { WIN_DRIVE ":\\usr\\.\\lib", WIN_DRIVE ":\\usr\\lib" },
        { WIN_DRIVE ":\\usr\\..\\lib/", WIN_DRIVE ":\\lib" },
        { WIN_DRIVE ":\\.", WIN_DRIVE ":\\" },
        { WIN_DRIVE ":\\..", WIN_DRIVE ":\\" },
        { "/usr/lib", WIN_DRIVE ":\\usr\\lib" },
        { "/usr/lib/", WIN_DRIVE ":\\usr\\lib" },
        { "/usr/lib/.", WIN_DRIVE ":\\usr\\lib" },
        { "/usr/lib/..", WIN_DRIVE ":\\usr" },
        { "/usr/./lib", WIN_DRIVE ":\\usr\\lib" },
        { "/usr/../lib/", WIN_DRIVE ":\\lib" },
        { "/.", WIN_DRIVE ":\\" },
        { "/..", WIN_DRIVE ":\\" },
        { "/./", WIN_DRIVE ":\\" },
        { "/../", WIN_DRIVE ":\\" },
        { "/", WIN_DRIVE ":\\" },
        // Windows reads a sequence of dots as "."
        { "/usr/lib/....", WIN_DRIVE ":\\usr\\lib" },
        // Windows removes empty directory names
        { "/usr/lib//", WIN_DRIVE ":\\usr\\lib" },
#else
        { "/usr/lib", "/usr/lib" },
        { "/usr/lib/", "/usr/lib" },
        { "/usr/lib/.", "/usr/lib" },
        { "/usr/lib/..", "/usr" },
        { "/usr/./lib", "/usr/lib" },
        { "/usr/../lib", "/lib" },
        { "/.", "/" },
        { "/..", "/" },
        { "/./", "/" },
        { "/../", "/" },
        { "/", "/" },
        // a sequence of dots can be a file name on unix
        { "/usr/lib/....", "/usr/lib/...." },
        // empty directory names remain on unix
        { "/usr/lib//", "/usr/lib/" },
#endif
        { NULL, NULL },
    };
    for (auto c : cases) {
        char* fullpath = envuGetFullPath(c.first);
        EXPECT_STREQ(c.second, fullpath) << "  c.first: " << c.first << std::endl;
        envuFree(fullpath);
    }
}

#define TEST_DIRS "/testdir/testdir/testdir/testdir/testdir/testdir/testdir/testdir"
#define TEST_DIRS_WIN "\\testdir\\testdir\\testdir\\testdir\\testdir\\testdir\\testdir\\testdir"

TEST(PathTest, envuGetFullPathLong) {
    const char* longpath =
        TEST_DIRS TEST_DIRS TEST_DIRS TEST_DIRS
        TEST_DIRS TEST_DIRS TEST_DIRS TEST_DIRS "/../testdir/";
    const char* expected =
#ifdef _WIN32
        WIN_DRIVE ":"
        TEST_DIRS_WIN TEST_DIRS_WIN TEST_DIRS_WIN TEST_DIRS_WIN
        TEST_DIRS_WIN TEST_DIRS_WIN TEST_DIRS_WIN TEST_DIRS_WIN;
#else
        TEST_DIRS TEST_DIRS TEST_DIRS TEST_DIRS
        TEST_DIRS TEST_DIRS TEST_DIRS TEST_DIRS;
#endif
    char* fullpath = envuGetFullPath(longpath);
    EXPECT_STREQ(expected, fullpath);
    envuFree(fullpath);
}

TEST(PathTest, envuGetFullPathUnicode) {
    // "/フォルダ/폴더/文件/."
    const char* unipath = u8"/\u30d5\u30a9\u30eb\u30c0/\ud3f4\ub354/\u6587\u4ef6/.";
    const char* expected =
#ifdef _WIN32
        WIN_DRIVE u8":\\\u30d5\u30a9\u30eb\u30c0\\\ud3f4\ub354\\\u6587\u4ef6";
#else
        u8"/\u30d5\u30a9\u30eb\u30c0/\ud3f4\ub354/\u6587\u4ef6";
#endif
    char* fullpath = envuGetFullPath(unipath);
    EXPECT_STREQ(expected, fullpath);
    envuFree(fullpath);
}

TEST(PathTest, envuGetFullPathRelative) {
    std::vector<std::pair<const char*, const char*>> cases = {
        { "usr", "usr" },
        { "usr/.", "usr" },
        { "usr/..", NULL },
        { ".", NULL },
        { "", NULL },
    };
    for (auto c : cases) {
        char* fullpath = envuGetFullPath(c.first);
        std::string expected(TRUE_CWD);
        if (c.second != NULL) {
        #ifdef _WIN32
            expected += std::string("\\") + c.second;
        #else
            expected += std::string("/") + c.second;
        #endif
        }
        EXPECT_STREQ(expected.c_str(), fullpath) << "  c.first: " << c.first << std::endl;
        envuFree(fullpath);
    }
}

TEST(PathTest, envuGetDirectory) {
    std::vector<std::pair<const char*, const char*>> cases = {
#ifdef _WIN32
        { WIN_DRIVE ":\\usr\\lib", WIN_DRIVE ":\\usr" },
        { WIN_DRIVE ":\\usr\\", WIN_DRIVE ":\\" },
        { WIN_DRIVE ":\\", WIN_DRIVE ":\\" },
#endif
        { "/usr/lib", "/usr" },
        { "/usr/", "/" },
        { "usr", "." },
        { "usr/", "." },
        { "/", "/" },
        { ".", "." },
        { "..", "." },
        { "", "." },
        { NULL, NULL },
    };
    for (auto c : cases) {
        char* dir = envuGetDirectory(c.first);
        EXPECT_STREQ(c.second, dir) << "  c.first: " << c.first << std::endl;
        envuFree(dir);
    }
}

TEST(PathTest, envuParseEnvPaths) {
    std::vector<std::pair<const char*, std::vector<const char*>>> cases = {
        { "", {} },
        { "path", { "path" } },
#ifdef _WIN32
        { ";", {} },
        { "path;", { "path" } },
        { "path;;", { "path" } },
        { "path;path2", { "path", "path2" } },
        { "path;path2;", { "path", "path2" } },
        { "path;;path2", { "path", "path2" } },
        { "path;path2;path3", { "path", "path2", "path3" } },
        { "C:\\Program Files", { "C:\\Program Files" } },
        { "C:\\Program Files;C:\\Users\\me", { "C:\\Program Files", "C:\\Users\\me" } },
#else
        { ":", {} },
        { "path:", { "path" } },
        { "path::", { "path" } },
        { "path:path2", { "path", "path2" } },
        { "path:path2:", { "path", "path2" } },
        { "path:path2", { "path", "path2" } },
        { "path:path2:path3", { "path", "path2", "path3" } },
        { "/Program Files", { "/Program Files" } },
        { "/Program Files:/Users/me", { "/Program Files", "/Users/me" } },
#endif
    };
    for (auto c : cases) {
        int count;
        char** paths = envuParseEnvPaths(c.first, &count);
        EXPECT_EQ(c.second.size(), count);
        for (int i = 0; i < count; i++) {
            EXPECT_STREQ(c.second[i], paths[i]) << "  c.first: " << c.first << std::endl;
        }
        EXPECT_EQ(NULL, paths[count]);
        envuFreeEnvPaths(paths);
    }
}

TEST(PathTest, envuParseEnvPathsNull) {
    char** paths = envuParseEnvPaths(NULL, NULL);
    EXPECT_EQ(NULL, paths);
}

TEST(PathTest, envuFreeEnvPathsNull) {
    envuFreeEnvPaths(NULL);
}

TEST(PathTest, envuGetEnvPaths) {
    std::vector<std::pair<const char*, std::vector<const char*>>> cases = {
        { "path", { "path" } },
#ifdef _WIN32
        { "C:\\Program Files;C:\\Users\\me", { "C:\\Program Files", "C:\\Users\\me" } },
#else
        { "/Program Files:/Users/me", { "/Program Files", "/Users/me" } },
#endif
    };
    char *env_path = envuGetEnv("PATH");
    for (auto c : cases) {
        int count;
        envuSetEnv("PATH", c.first);
        char** paths = envuGetEnvPaths(&count);
        EXPECT_EQ(c.second.size(), count);
        for (int i = 0; i < count; i++) {
            EXPECT_STREQ(c.second[i], paths[i]) << "  c.first: " << c.first << std::endl;
        }
        EXPECT_EQ(NULL, paths[count]);
        envuFreeEnvPaths(paths);
    }

    // restore the PATH variable
    envuSetEnv("PATH", env_path);
    envuFree(env_path);
}

TEST(PathTest, envuGetEnvPathsNull) {
    char *env_path = envuGetEnv("PATH");

    envuSetEnv("PATH", NULL);
    char** paths = envuGetEnvPaths(NULL);
    EXPECT_EQ(NULL, paths);

    // restore the PATH variable
    envuSetEnv("PATH", env_path);
    envuFree(env_path);
}
