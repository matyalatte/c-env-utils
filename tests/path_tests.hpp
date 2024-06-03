#pragma once
#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <utility>
#include "env_utils.h"
#include "true_env_info.h"

// TODO: test with long paths
// TODO: test with unicode strings

TEST(PathTest, envuGetFullPathAbsolute) {
    std::vector<std::pair<const char*, const char*>> cases = {
#ifdef _WIN32
        // Note: WIN_DRIVE ":\\" == "C:\\" on C drive.
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
#endif
        { NULL, NULL },
    };
    for (auto c : cases) {
        char* fullpath = envuGetFullPath(c.first);
        EXPECT_STREQ(c.second, fullpath);
        envuFree(fullpath);
    }
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
        EXPECT_STREQ(expected.c_str(), fullpath);
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
        EXPECT_STREQ(c.second, dir);
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
        if (c.first == NULL) {
            EXPECT_EQ(NULL, paths);
            continue;
        }
        EXPECT_EQ(c.second.size(), count);
        for (int i = 0; i < count; i++) {
            EXPECT_STREQ(c.second[i], paths[i]);
        }
        envuFreeEnvPaths(paths);
    }
}

TEST(PathTest, envuParseEnvPathsNull) {
    char** paths = envuParseEnvPaths(NULL, NULL);
    EXPECT_EQ(NULL, paths);
}
