#include "env_utils.h"
#include <stdio.h>

#ifdef _WIN32
#include "env_utils_windows.h"
#include <stdarg.h>
#include <malloc.h>
void PrintUTF8(const char* fmt, ...) {
    va_list va;
    va_start(va, fmt);

    va_list va2;
    va_copy(va2, va);
    size_t n = _vscprintf(fmt, va2);
    va_end(va2);
    n++;

    char *buf = (char *)calloc(n + 1, sizeof(char));
    if (buf == NULL) return;
    vsprintf_s(buf, n, fmt, va);
    va_end(va);

    wchar_t *wfmt = envuUTF8toUTF16(buf);
    wprintf(L"%ls", wfmt);
    envuFree(wfmt);
    free(buf);
}
#define PRINTF(fmt, ...) PrintUTF8(fmt, __VA_ARGS__)
#else  // _WIN32
#define PRINTF(fmt, ...) printf(fmt, __VA_ARGS__)
#endif  // _WIN32

int main(void) {
    PRINTF("c-env-utils v%s\n", envuGetVersion());
    PRINTF("%s", "\n");

    char *exe_path = envuGetExecutablePath();
    PRINTF("Executable path: %s\n", exe_path);
    envuFree(exe_path);

    char *exe_dir = envuGetExecutableDir();
    PRINTF("Executable dir: %s\n", exe_dir);
    envuFree(exe_dir);

    char *cwd = envuGetCwd();
    PRINTF("CWD: %s\n", cwd);
    envuFree(cwd);

    char *home = envuGetHome();
    PRINTF("Home: %s\n", home);
    envuFree(home);

    char *username = envuGetUsername();
    PRINTF("User: %s\n", username);
    envuFree(username);

    char *os = envuGetOS();
    PRINTF("OS: %s\n", os);
    envuFree(os);

    char *os_ver = envuGetOSVersion();
    PRINTF("OS version: %s\n", os_ver);
    envuFree(os_ver);

    char *os_pn = envuGetOSProductName();
    PRINTF("OS porduct name: %s\n", os_pn);
    envuFree(os_pn);

    int count;
    char **paths = envuGetEnvPaths(&count);
    PRINTF("%s", "PATH:\n");
    for (char **p = paths; p < paths + count; p++) {
        PRINTF("  %s\n", *p);
    }
    envuFreeEnvPaths(paths);
}
