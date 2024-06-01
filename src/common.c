#include "env_utils.h"

const char* envuGetVersion() {
    return ENVU_VERSION;
}

int envuGetVersionAsInt() {
    return ENVU_VERSION_INT;
}

char *envuGetExecutableDir() {
    char *exe_path = envuGetExecutablePath();
    char *exe_dir = envuGetDirectory(exe_path);
    envuFree(exe_path);
    return exe_dir;
}

void envuFree(void *p) {
    free(p);
}
