#include <Windows.h>
#include <direct.h>
#include <malloc.h>
#include <Lmcons.h>
#include <limits.h>

#include "env_utils.h"
#include "env_utils_windows.h"
#include "env_utils_priv.h"

#ifndef UNLEN
#warning("Warning: Failed to get UNLEN. The compiler uses 256 for it.")
#define UNLEN 256
#endif

#define ENVU_CP_UTF8 65001

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define SIZET_TO_INT(N) (int)MIN(N, INT_MAX)

wchar_t *envuAllocWstr(size_t size) {
    wchar_t *wstr;
    wstr = (wchar_t *)calloc(size + 1, sizeof(wchar_t));
    return wstr;
}

wchar_t *envuAllocWstrWithConst(const wchar_t *c) {
    if (c == NULL)
        return NULL;
    size_t str_len = wcslen(c);
    wchar_t *wstr = envuAllocWstr(str_len);
    if (wstr == NULL)
        return NULL;
    memcpy_s(wstr, str_len * sizeof(wchar_t), c, str_len * sizeof(wchar_t));
    return wstr;
}

wchar_t *envuUTF8toUTF16(const char* str) {
    if (str == NULL)
        return NULL;
    if (*str == '\0')
        return envuAllocEmptyWstr();

    wchar_t *wstr;
    int str_len = SIZET_TO_INT(strlen(str));
    int wstr_len = MultiByteToWideChar(ENVU_CP_UTF8, 0, str, str_len + 1, NULL, 0);
    if (wstr_len == 0)
        return envuAllocEmptyWstr();

    wstr = envuAllocWstr(wstr_len);
    int res = MultiByteToWideChar(ENVU_CP_UTF8, 0, str, str_len + 1, wstr, wstr_len);
    if (res != wstr_len)
        *wstr = L'\0';
    return wstr;
}

char *envuAllocStr(size_t size) {
    char *str;
    str = (char *)calloc(size + 1, sizeof(char));
    return str;
}

char *envuAllocStrWithConst(const char *c) {
    if (c == NULL)
        return NULL;
    size_t str_len = strlen(c);
    char *str = envuAllocStr(str_len);
    if (str == NULL)
        return NULL;
    memcpy_s(str, str_len, c, str_len);
    return str;
}

char *envuAppendStr(char *str1, const char *str2) {
    if (str1 == NULL || str2 == NULL)
        return str1;
    size_t str_len1 = strlen(str1);
    size_t str_len2 = strlen(str2);
    char *str = realloc(str1, (str_len1 + str_len2 + 1) * sizeof(char));
    if (str == NULL) {
        envuFree(str1);
        return NULL;
    }
    memcpy_s(str + str_len1, str_len2, str2, str_len2);
    str[str_len1 + str_len2] = '\0';
    return str;
}

char *envuUTF16toUTF8(const wchar_t* wstr) {
    if (wstr == NULL)
        return NULL;
    if (*wstr == L'\0')
        return envuAllocEmptyStr();

    char *str;
    int wstr_len = SIZET_TO_INT(wcslen(wstr));
    int str_len = WideCharToMultiByte(ENVU_CP_UTF8, 0, wstr, wstr_len + 1, NULL, 0, NULL, NULL);
    if (str_len == 0)
        return envuAllocEmptyStr();

    str = envuAllocStr(str_len);
    int res = WideCharToMultiByte(ENVU_CP_UTF8, 0, wstr, wstr_len + 1, str, str_len, NULL, NULL);
    if (res != str_len)
        *str = '\0';
    return str;
}

char *envuGetExecutablePath(void) {
    wchar_t *wpath = NULL;
    int max_size = 256;
    int size;

    // Note: The max length of file paths is not MAX_PATH now.
    while (max_size <= 32768) {
        wpath = envuAllocWstr(max_size);
        size = GetModuleFileNameW(NULL, wpath, max_size);
        if (size < max_size)
            break;

        // the buffer size was not enough.
        envuFree(wpath);
        max_size *= 2;
    }

    if (size == 0) {
        envuFree(wpath);
        return NULL;
    }

    char *path = envuUTF16toUTF8(wpath);
    envuFree(wpath);
    return path;
}

static inline DWORD getFileAttributes(const char *path) {
    wchar_t *wpath = envuUTF8toUTF16(path);
    DWORD ret = GetFileAttributesW(wpath);
    envuFree(wpath);
    return ret;
}

int envuFileExists(const char *path) {
    DWORD ret = getFileAttributes(path);
    return (ret != INVALID_FILE_ATTRIBUTES) && ((ret & FILE_ATTRIBUTE_DIRECTORY) == 0);
}

int envuPathExists(const char *path) {
    DWORD ret = getFileAttributes(path);
    return ret != INVALID_FILE_ATTRIBUTES;
}

char *envuGetRealPath(const char *path) {
    // TODO: Search the PATH variables, and resolve symlinks.
    char *fullpath = envuGetFullPath(path);
    if (fullpath == NULL || !envuPathExists(fullpath))
        return NULL;
    return fullpath;
}

char *envuGetFullPath(const char *path) {
    if (path == NULL)
        return NULL;
    if (path[0] == '\0')
        return envuGetCwd();

    wchar_t *wpath = envuUTF8toUTF16(path);

    wchar_t *wfullpath = _wfullpath(NULL, wpath, 0);
    envuFree(wpath);

    size_t size = wcslen(wfullpath);
    if (size < 2) {
        envuFree(wfullpath);
        return NULL;
    }

    // remove the last slash
    if (wfullpath[size - 1] == L'\\' && wfullpath[size - 2] != L':')
        wfullpath[size - 1] = L'\0';

    char *fullpath = envuUTF16toUTF8(wfullpath);
    envuFree(wfullpath);
    return fullpath;
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
        return envuAllocStrWithConst(".");

    char *copied_path = envuAllocStrWithConst(path);

    // TODO: read the path backwards.
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
        return envuAllocStrWithConst(".");
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
            return envuAllocStrWithConst(".");
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
    char *str = envuAllocStrWithConst(copied_path);
    envuFree(copied_path);
    return str;
}

char *envuGetCwd(void) {
    wchar_t *cwd = _wgetcwd(NULL, 0);
    if (cwd == NULL)
        return NULL;
    char *ret = envuUTF16toUTF8(cwd);
    free(cwd);
    return ret;
}

int envuSetCwd(const char *path) {
    if (path == NULL)
        return -1;
    wchar_t *wpath = envuUTF8toUTF16(path);
    int ret = _wchdir(wpath);
    envuFree(wpath);
    return -(ret != 0);
}

char *envuGetEnv(const char *name) {
    if (name == NULL)
        return NULL;
    wchar_t *wname = envuUTF8toUTF16(name);
    size_t size;
    if (_wgetenv_s(&size, NULL, 0, wname) || size == 0) {
        envuFree(wname);
        return NULL;
    }
    wchar_t* wstr = envuAllocWstr(size);
    int ret = _wgetenv_s(&size, wstr, size, wname);
    envuFree(wname);
    if (ret) {
        envuFree(wstr);
        return NULL;
    }
    char *str = envuUTF16toUTF8(wstr);
    envuFree(wstr);
    return str;
}

int envuSetEnv(const char *name, const char *value) {
    if (name == NULL)
        return -1;
    wchar_t *wname = envuUTF8toUTF16(name);
    wchar_t *wvalue = envuUTF8toUTF16(value);
    errno_t ret;
    if (wvalue == NULL)
        ret = _wputenv_s(wname, L"");
    else
        ret = _wputenv_s(wname, wvalue);
    envuFree(wname);
    envuFree(wvalue);
    return -(ret != 0);
}

char *envuGetHome(void) {
    // Check USERPROFILE
    char *userprof = envuGetEnv("USERPROFILE");
    if (userprof != NULL)
        return userprof;
    envuFree(userprof);

    // Check HOMEDRIVE and HOMEPATH
    char *drive = envuGetEnv("HOMEDRIVE");  // "C:"
    char *path = envuGetEnv("HOMEPATH");  // "\Users\name"
    if (drive == NULL || drive[strlen(drive) - 1] != ':') {
        envuFree(drive);
        envuFree(path);
        return NULL;
    }
    if (path == NULL) {
        envuFree(path);
        path = envuAllocStrWithConst("\\");
    }
    char *str = envuAppendStr(drive, path);
    envuFree(path);
    return str;
}

char *envuGetUsername(void) {
    // Try GetUserNameW
    wchar_t wname[UNLEN + 1];
    wname[UNLEN] = 0;
    DWORD bufsize = UNLEN + 1;
    if (GetUserNameW(wname, &bufsize) != 0) {
        return envuUTF16toUTF8(wname);
    }

    // Check USERNAME
    char *name = envuGetEnv("USERNAME");
    if (name != NULL)
        return name;
    envuFree(name);

    return NULL;
}

char *envuGetOS(void) {
    return envuAllocStrWithConst("Windows");
}

char *envuGetOSVersion(void) {
    wchar_t *wstr = getOSInfoFromWMI(L"Version");
    return envuUTF16toUTF8(wstr);
}

char *envuGetOSProductName(void) {
    wchar_t *wstr = getOSInfoFromWMI(L"Caption");
    return envuUTF16toUTF8(wstr);
}
