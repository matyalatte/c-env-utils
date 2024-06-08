#include <string.h>  // for strlen
#ifdef _WIN32
#include <malloc.h>  // for malloc
#else
#include <stdlib.h>  // for malloc
#endif

#include "env_utils.h"
#include "env_utils_priv.h"

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

static inline char **envuParseEnvPathsBase(const char *env_path, int *path_count, char delim) {
    if (env_path == NULL)
        return NULL;

    char *copied_env_path = envuAllocStrWithConst(env_path);
    int count = 0;
    {
        char *p = copied_env_path;
        char *start_p = copied_env_path;
        while (*p != '\0') {
            if (*p == delim) {
                *p = '\0';
                if (p - start_p > 0)
                    count++;
                start_p = p + 1;
            }
            p++;
        }
        if (p - start_p > 0)
            count++;
    }
    if (path_count != NULL)
        *path_count = count;

    char** paths = malloc((count + 1) * sizeof(char*));
    if (paths == NULL) {
        envuFree(copied_env_path);
        return NULL;
    }

    {
        char **p = paths;
        char *env_p = copied_env_path;
        while (p < paths + count) {
            size_t len = strlen(env_p);
            if (len > 0) {
                *p = envuAllocStrWithConst(env_p);
                if (*p == NULL) {
                    // Failed to alloc a path
                    envuFreeEnvPaths(paths);
                    envuFree(copied_env_path);
                    return NULL;
                }
                p++;
            }
            env_p += strlen(env_p) + 1;
        }
        *p = NULL;
    }
    envuFree(copied_env_path);
    return paths;
}

char **envuParseEnvPaths(const char *env_path, int *path_count) {
#ifdef _WIN32
    return envuParseEnvPathsBase(env_path, path_count, ';');
#else
    return envuParseEnvPathsBase(env_path, path_count, ':');
#endif
}

char **envuGetEnvPaths(int *path_count) {
    char *env_path = envuGetEnv("PATH");
    char **paths = envuParseEnvPaths(env_path, path_count);
    envuFree(env_path);
    return paths;
}

void envuFreeEnvPaths(char **paths) {
    if (paths == NULL)
        return;

    char **p = paths;
    while (*p != NULL) {
        envuFree(*p);
        p++;
    }
    envuFree(paths);
}
