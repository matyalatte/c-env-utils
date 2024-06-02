#define _GNU_SOURCE

#ifdef __APPLE__
#include <mach-o/dyld.h>
#elif defined(__FreeBSD__)
#include <sys/param.h>
#include <sys/sysctl.h>  // for GetExecutablePathFreeBSD()
#elif defined(__HAIKU__)
#include <kernel/image.h>  // for GetExecutablePathHaiku()
#endif

#include <sys/stat.h>
#include <sys/utsname.h>
#include <unistd.h>
#include <pwd.h>
#include <limits.h>
#include <libgen.h>
#include <string.h>

#include "env_utils.h"

#ifndef PATH_MAX
#define PATH_MAX  512
#endif

static char *AllocStr(size_t size) {
    char *str;
    str = (char *)calloc(size + 1, sizeof(char));
    return str;
}

#define AllocEmptyStr() AllocStr(0)

static char *AllocStrWithConst(const char *c) {
    size_t str_len = strlen(c);
    char *str = AllocStr(str_len);
    memcpy(str, c, str_len);
    return str;
}

static char *AllocStrWithTwoConsts(const char *c1, const char *c2) {
    size_t str_len1 = strlen(c1);
    size_t str_len2 = strlen(c2);
    char *str = AllocStr(str_len1 + str_len2);
    memcpy(str, c1, str_len1);
    memcpy(str + str_len1, c2, str_len2);
    return str;
}

#ifdef __APPLE__
// macOS requires _NSGetExecutablePath to get the executable path.
char *envuGetExecutablePath() {
    char path[PATH_MAX + 1];
    path[0] = 0;
    path[PATH_MAX] = 0;
    uint32_t bufsize = PATH_MAX;
    int ret = _NSGetExecutablePath(path, &bufsize);
    if (ret || bufsize == 0 || *path == '\0')
        return AllocStrWithConst("/");
    return AllocStrWithConst(path);
}
#elif defined(__FreeBSD__)
// FreeBSD requires sysctl to get the executable path.
static void GetExecutablePathFreeBSD(char *path) {
    size_t path_size = PATH_MAX;
    int mib[] = { CTL_KERN, KERN_PROC, KERN_PROC_PATHNAME, -1 };
    int error = sysctl(mib, 4, path, &path_size, NULL, 0);
    if (error < 0 || path_size == 0)
        path_size = 0;
    path[path_size] = 0;
}

char *envuGetExecutablePath() {
    char path[PATH_MAX + 1];
    path[PATH_MAX] = 0;
    GetExecutablePathFreeBSD(path);
    if (*path == '\0')
        return AllocStrWithConst("/");
    return AllocStrWithConst(path);
}
#elif defined(__HAIKU__)
// Haiku OS requires get_next_image_info to get the executable path.
char *envuGetExecutablePath() {
    int32_t cookie = 0;
    image_info info;
    while (get_next_image_info(B_CURRENT_TEAM, &cookie, &info) == B_OK) {
        if (info.type == B_APP_IMAGE)
            return AllocStrWithConst(info.name);
    }
    return AllocStrWithConst("/");
}
#else
// Linux distributons support readlink to get the executable path.
static int TryReadlink(const char *link, char *path, int path_size) {
    int new_path_size;
    if (path_size != 0)
        return path_size;
    new_path_size = readlink(link, path, PATH_MAX);
    if (new_path_size == -1)
        new_path_size = 0;
    return new_path_size;
}

static void GetExecutablePathUnix(char *path) {
    int path_size = 0;
    path_size = TryReadlink("/proc/self/exe", path, path_size);  // Linux
    path_size = TryReadlink("/proc/curproc/exe", path, path_size);  // NetBSD
    path_size = TryReadlink("/proc/curproc/file", path, path_size);  // Other BSD variants?
    path_size = TryReadlink("/proc/self/path/a.out", path, path_size);  // Solaris
    path[path_size] = 0;
}

// TODO: path contains "." on NetBSD somehow.
char *envuGetExecutablePath() {
    // try readlink
    char path[PATH_MAX + 1];
    path[PATH_MAX] = 0;
    GetExecutablePathUnix(path);
    if (*path != '\0')
        return AllocStrWithConst(path);

#ifndef __linux__
    // readlink() doesn't work on OpenBSD
    // https://stackoverflow.com/questions/31494901/how-to-get-the-executable-path-on-openbsd

    // try argv[0]
    char *argv0 = getenv("_");
    if (*argv0 == "\0")  // failed to get argv[0]
        return AllocStrWithConst("/");
    else if (*argv0 == "/")  // argv[0] is an absolute path
        return AllocStrWithConst(argv0);

    // use pwd if exists
    char *pwd = getenv("PWD");
    if (*pwd == "\0")
        return envuGetFullPath(argv0);

    // concatnate pwd and argv[0]
    size_t pwd_size = strlen(pwd);
    char *concat;
    if (pwd[pwd_size - 1] == "/") {
        concat = AllocStrWithTwoConsts(pwd, argv0);
    } else {
        char *pwd2 = AllocStrWithTwoConsts(pwd, "/");
        concat = AllocStrWithTwoConsts(pwd2, argv0);
        envuFree(pwd2);
    }
    char *ret = envuGetFullPath(concat);
    envuFree(concat);
    if (envuFileExists(ret))  // exe path found
        return ret;

    // TODO: check the PATH variable?

    envuFree(ret);
#endif
    // Failed to get exe path
    return AllocStrWithConst("/");
}
#endif

int envuFileExists(const char *path) {
    struct stat buffer;
    return (stat(path, &buffer) == 0);
}

char *envuGetFullPath(const char *path) {
    return realpath(path, NULL);
}

char *envuGetDirectory(const char *path) {
    // dirname() may modify the contents of path. So, we use copied one.
    char *copied_path = AllocStrWithConst(path);
    char *dir = dirname(copied_path);
    if (dir == NULL) {
        free(copied_path);
        return AllocStrWithConst("/");
    }
    char *ret = AllocStrWithConst(dir);
    free(copied_path);
    return ret;
}

char *envuGetCwd() {
    char cwd[PATH_MAX + 1];
    cwd[PATH_MAX] = 0;
    getcwd(cwd, PATH_MAX);
    return AllocStrWithConst(cwd);
}

void envuSetCwd(const char *path) {
    chdir(path);
}

char *envuGetEnv(const char *name) {
    char *str = getenv(name);
    if (str == NULL)
        return AllocStrWithConst("");
    return AllocStrWithConst(str);
}

void envuSetEnv(const char *name, const char *value) {
    setenv(name, value, 1);
}

static struct passwd *getpwuid_safe(char **buf) {
    struct passwd pwd;
    struct passwd *result = NULL;
    int bufsize;

    bufsize = sysconf(_SC_GETPW_R_SIZE_MAX);
    if (bufsize == -1)
        bufsize = 16384;

    *buf = calloc(bufsize + 1, 1);
    if (*buf == NULL)
        return NULL;

#ifdef __sun
    // Solaris has a little bit different APIs
    result = getpwuid_r(getuid(), &pwd, *buf, bufsize);
#else
    int ret = getpwuid_r(getuid(), &pwd, *buf, bufsize, &result);
    if (ret != 0) return NULL;
#endif
    return result;
}

char *envuGetHome() {
    char *buf;
    struct passwd *p = getpwuid_safe(&buf);

    char* homedir = NULL;
    if (p != NULL)  // try to get from user info
        homedir = p->pw_dir;
    if (homedir == NULL)  // try to get from env
        homedir = getenv("HOME");
    if (homedir == NULL) {
        // failed to get homedir
        free(buf);
        return AllocStrWithConst("/");
    }
    char *str = AllocStrWithConst(homedir);
    free(buf);
    return str;
}

char *envuGetUsername() {
    char *buf;
    struct passwd *p = getpwuid_safe(&buf);

    char* name = NULL;
    if (p != NULL)  // try to get from user info
        name = p->pw_name;
    if (name == NULL)  // try to get from env
        name = getenv("USER");
    if (name == NULL)  // try to get from env
        name = getenv("LOGNAME");
    if (name == NULL) {
        // failed to get username
        free(buf);
        return AllocStrWithConst("");
    }
    char *str = AllocStrWithConst(name);
    free(buf);
    return str;
}

// Darwin, Linux, FreeBSD, OpenBSD, NetBSD, Haiku, SunOS, etc.
char *envuGetOS() {
    struct utsname buf = { 0 };
    // Note: uname(&buf) can be positive on Solaris
    if (uname(&buf) == -1) {
        return AllocStrWithConst("");
    }
    return AllocStrWithConst(buf.sysname);
}
