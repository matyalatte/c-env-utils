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
    char *exe_path = envuGetExecutablePath();
    char *exe_dir = envuGetExecutableDir();
    char *cwd = envuGetCwd();
    char *home = envuGetHome();
    char *username = envuGetUsername();
    char *os = envuGetOS();
    char *paths = envuGetEnv("PATH");
    PRINTF("c-env-utils v%s\n", envuGetVersion());
    PRINTF("%s", "\n");
    PRINTF("Executable path: %s\n", exe_path);
    PRINTF("Executable dir: %s\n", exe_dir);
    PRINTF("CWD: %s\n", cwd);
    PRINTF("Home: %s\n", home);
    PRINTF("User: %s\n", username);
    PRINTF("OS: %s\n", os);
    PRINTF("PATH: %s\n", paths);
    envuFree(exe_path);
    envuFree(exe_dir);
    envuFree(cwd);
    envuFree(home);
    envuFree(username);
    envuFree(os);
    envuFree(paths);
}
