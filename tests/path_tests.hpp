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
        { "C:\\usr\\lib", "C:\\usr\\lib" },
        { "C:\\usr\\lib\\", "C:\\usr\\lib" },
        { "C:\\usr\\lib\\.", "C:\\usr\\lib" },
        { "C:\\usr\\lib\\..", "C:\\usr" },
        { "C:\\usr\\.\\lib", "C:\\usr\\lib" },
        { "C:\\usr\\..\\lib/", "C:\\lib" },
        { "C:\\.", "C:\\" },
        { "C:\\..", "C:\\" },
        { "/usr/lib", "C:\\usr\\lib" },
        { "/usr/lib/", "C:\\usr\\lib" },
        { "/usr/lib/.", "C:\\usr\\lib" },
        { "/usr/lib/..", "C:\\usr" },
        { "/usr/./lib", "C:\\usr\\lib" },
        { "/usr/../lib/", "C:\\lib" },
        { "/.", "C:\\" },
        { "/..", "C:\\" },
        { "/./", "C:\\" },
        { "/../", "C:\\" },
        { "/", "C:\\" },
        // Windows reads a sequence of dots as "."
        { "/usr/lib/....", "C:\\usr\\lib" },
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
        { "C:\\usr\\lib", "C:\\usr" },
        { "C:\\usr\\", "C:\\" },
        { "C:\\", "C:\\" },
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
