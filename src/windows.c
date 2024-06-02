#include <Windows.h>
#include <direct.h>
#include <malloc.h>
#include <Lmcons.h>
#include <limits.h>

#include "env_utils.h"
#include "env_utils_windows.h"

#ifndef MAX_PATH
#define MAX_PATH  260
#endif
#ifndef UNLEN
#define UNLEN 256
#endif

#define ENVU_CP_UTF8 65001

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define SIZET_TO_INT(N) (int)MIN(N, INT_MAX)

static wchar_t *AllocWstr(size_t size) {
    wchar_t *wstr;
    wstr = (wchar_t *)calloc(size + 1, sizeof(wchar_t));
    return wstr;
}

#define AllocEmptyWstr() AllocWstr(0)

wchar_t *envuUTF8toUTF16(const char* str) {
    if (*str == '\0')
        return AllocEmptyWstr();

    wchar_t *wstr;
    int str_len = SIZET_TO_INT(strlen(str));
    int wstr_len = MultiByteToWideChar(ENVU_CP_UTF8, 0, str, str_len + 1, NULL, 0);
    if (wstr_len == 0)
        return AllocEmptyWstr();

    wstr = AllocWstr(wstr_len);
    int res = MultiByteToWideChar(ENVU_CP_UTF8, 0, str, str_len + 1, wstr, wstr_len);
    if (res != wstr_len)
        *wstr = L'\0';
    return wstr;
}

static char *AllocStr(size_t size) {
    char *str;
    str = (char *)calloc(size + 1, sizeof(char));
    return str;
}

#define AllocEmptyStr() AllocStr(0)

static char *AllocStrWithConst(const char *c) {
    size_t str_len = strlen(c);
    char *str = AllocStr(str_len);
    memcpy_s(str, str_len, c, str_len);
    return str;
}

static char *AllocStrWithTwoConsts(const char *c1, const char *c2) {
    size_t str_len1 = strlen(c1);
    size_t str_len2 = strlen(c2);
    char *str = AllocStr(str_len1 + str_len2);
    memcpy_s(str, str_len1, c1, str_len1);
    memcpy_s(str + str_len1, str_len2, c2, str_len2);
    return str;
}

char *envuUTF16toUTF8(const wchar_t* wstr) {
    if (*wstr == L'\0')
        return AllocEmptyStr();

    char *str;
    int wstr_len = SIZET_TO_INT(wcslen(wstr));
    int str_len = WideCharToMultiByte(ENVU_CP_UTF8, 0, wstr, wstr_len + 1, NULL, 0, NULL, NULL);
    if (str_len == 0)
        return AllocEmptyStr();

    str = AllocStr(str_len);
    int res = WideCharToMultiByte(ENVU_CP_UTF8, 0, wstr, wstr_len + 1, str, str_len, NULL, NULL);
    if (res != str_len)
        *str = '\0';
    return str;
}

char *envuGetExecutablePath() {
    wchar_t filename[MAX_PATH + 1];
    filename[MAX_PATH] = 0;
    GetModuleFileNameW(NULL, filename, MAX_PATH);
    return envuUTF16toUTF8(filename);
}

int envuFileExists(const char *path) {
    wchar_t *wpath = envuUTF8toUTF16(path);
    int ret = GetFileAttributesW(wpath) != INVALID_FILE_ATTRIBUTES;
    envuFree(wpath);
    return ret;
}

char *envuGetFullPath(const char *path) {
    if (path == NULL)
        return NULL;
    if (path[0] == '\0')
        return envuGetCwd();

    wchar_t *wpath = envuUTF8toUTF16(path);
    wchar_t fullpath[MAX_PATH + 1];
    fullpath[MAX_PATH] = 0;
    int size = GetFullPathNameW(wpath, MAX_PATH, fullpath, NULL);
    if (size < 2)  // failed to get full path.
        return NULL;

    // remove the last slash
    if (fullpath[size - 1] == L'\\' && fullpath[size - 2] != L':')
        fullpath[size - 1] = L'\0';

    envuFree(wpath);
    return envuUTF16toUTF8(fullpath);
}

static int isAbsPath(const char *path) {
    if (path[0] == '/' || path[0] == '\\')
        return 1;
    while (path[0] != '\0' && path[0] != '/' && path[0] != '\\') {
        if (path[0] == ':' && (path[1] == '/' || path[1] == '\\'))
            return 1;
        path++;
    }
    return 0;
}

char *envuGetDirectory(const char *path) {
    // TODO: should we support the "\?" prefix?
    if (path == NULL)
        return NULL;
    if (*path == '\0')
        return AllocStrWithConst(".");

    char *copied_path = AllocStrWithConst(path);

    char *p = copied_path;
    char *slash_p[3] = { NULL, NULL, NULL };
    while (*p != '\0') {
        if (*p == '\\' || *p == '/') {
            // store the last three slashes.
            slash_p[2] = slash_p[1];
            slash_p[1] = slash_p[0];
            slash_p[0] = p;
        }
        p++;
    }
    if (slash_p[0] == NULL) {
        // slash not found
        envuFree(copied_path);
        return AllocStrWithConst(".");
    }
    if (slash_p[0] + 1 == p) {
        // the last character is a slash
        if (slash_p[1] == NULL) {
            // only the last character is a slash
            if (isAbsPath(path)) {
                // drive root. ("/" or "*:/")
                return copied_path;
            }
            envuFree(copied_path);
            return AllocStrWithConst(".");
        }
        slash_p[0] = slash_p[1];
        slash_p[1] = slash_p[2];
    }
    if (slash_p[1] == NULL) {
        // keep the last slash because there is no other slashes.
        slash_p[0][1] = '\0';
    } else {
        // overwrite the last slash with null
        slash_p[0][0] = '\0';
    }
    char *str = AllocStrWithConst(copied_path);
    envuFree(copied_path);
    return str;
}

char *envuGetCwd() {
    wchar_t cwd[MAX_PATH + 1];
    cwd[MAX_PATH] = 0;
    _wgetcwd(cwd, MAX_PATH);
    return envuUTF16toUTF8(cwd);
}

void envuSetCwd(const char *path) {
    wchar_t *wpath = envuUTF8toUTF16(path);
    _wchdir(wpath);
    envuFree(wpath);
}

char *envuGetEnv(const char *name) {
    wchar_t *wname = envuUTF8toUTF16(name);
    size_t size;
    if (_wgetenv_s(&size, NULL, 0, wname)) {
        envuFree(wname);
        return AllocStrWithConst("");
    }
    wchar_t* wstr = AllocWstr(size + 1);
    int ret = _wgetenv_s(&size, wstr, size, wname);
    envuFree(wname);
    if (ret) {
        envuFree(wstr);
        return AllocStrWithConst("");
    }
    char *str = envuUTF16toUTF8(wstr);
    envuFree(wstr);
    return str;
}

void envuSetEnv(const char *name, const char *value) {
    wchar_t *wname = envuUTF8toUTF16(name);
    wchar_t *wvalue = envuUTF8toUTF16(value);
    _wputenv_s(wname, wvalue);
    envuFree(wname);
    envuFree(wvalue);
}

char *envuGetHome() {
    // Check USERPROFILE
    char *userprof = envuGetEnv("USERPROFILE");
    if (*userprof != 0)
        return userprof;
    envuFree(userprof);

    // Check HOMEDRIVE and HOMEPATH
    char *drive = envuGetEnv("HOMEDRIVE");
    char *path = envuGetEnv("HOMEPATH");
    if (*drive == 0) {
        envuFree(drive);
        envuFree(path);
        return AllocStrWithConst("C:\\");
    }
    if (*path == 0) {
        envuFree(path);
        path = AllocStrWithConst("\\");
    }
    char *str = AllocStrWithTwoConsts(drive, path);
    envuFree(drive);
    envuFree(path);
    return str;
}

char *envuGetUsername() {
    // Try GetUserNameW
    wchar_t wname[UNLEN + 1];
    wname[UNLEN] = 0;
    DWORD bufsize = UNLEN + 1;
    if (GetUserNameW(wname, &bufsize) != 0) {
        return envuUTF16toUTF8(wname);
    }

    // Check USERNAME
    char *name = envuGetEnv("USERNAME");
    if (*name != 0)
        return name;
    envuFree(name);

    return AllocEmptyStr();
}

char *envuGetOS() {
    return AllocStrWithConst("Windows");
}
