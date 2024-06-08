#define _GNU_SOURCE

#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#include <mach-o/dyld.h>
#elif defined(__FreeBSD__) || defined(__OpenBSD__)
// for GetExecutablePath()
#include <sys/param.h>
#include <sys/sysctl.h>
#elif defined(__HAIKU__)
// for GetExecutablePath()
#include <kernel/image.h>
#endif

#include <sys/stat.h>
#include <sys/utsname.h>
#include <unistd.h>
#include <pwd.h>
#include <limits.h>
#include <libgen.h>
#include <string.h>
#include <stdio.h>

#include "env_utils.h"
#include "env_utils_priv.h"

#ifdef __sun
// Solaris uses MAXPATHLEN instead of PATH_MAX
#include <sys/param.h>
#define PATH_MAX MAXPATHLEN
// _GNU_SOURCE does not seem to work fine on Solaris.
// So, we declare some functions here.
int setenv(const char *envname, const char *envval, int overwrite);
int unsetenv(const char *name);
ssize_t getline(char **lineptr, size_t *n, FILE *stream);
#endif

#ifndef PATH_MAX
#warning("Warning: Failed to get PATH_MAX. The compiler uses 512 for it.")
#define PATH_MAX  512
#endif

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
    memcpy(str, c, str_len);
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
    memcpy(str + str_len1, str2, str_len2);
    str[str_len1 + str_len2] = '\0';
    return str;
}

char *envuGetRealPath(const char *path) {
    char str[PATH_MAX + 1];
    str[PATH_MAX] = '\0';
    char *resolved = realpath(path, str);
    return envuAllocStrWithConst(resolved);
}

#ifdef __APPLE__
// macOS requires _NSGetExecutablePath to get the executable path.
static inline char *getExecutablePathApple() {
    char path[PATH_MAX + 1];
    path[PATH_MAX] = '\0';
    uint32_t bufsize = PATH_MAX;
    int ret = _NSGetExecutablePath(path, &bufsize);
    if (ret || bufsize == 0 || *path == '\0') {
        // Failed to get exe path.
        return NULL;
    }

    // resolve symlinks and dot segments
    return envuGetRealPath(path);
}
#elif defined(__FreeBSD__)
// FreeBSD requires sysctl to get the executable path.
static inline char *getExecutablePathFreeBSD() {
    char path[PATH_MAX + 1];
    path[PATH_MAX] = 0;

    size_t path_size = PATH_MAX;
    int mib[] = { CTL_KERN, KERN_PROC, KERN_PROC_PATHNAME, -1 };
    int error = sysctl(mib, 4, path, &path_size, NULL, 0);
    if (error < 0 || path_size == 0)
        path_size = 0;
    path[path_size] = 0;
    if (*path == '\0')
        return NULL;
    return envuAllocStrWithConst(path);
}
#elif defined(__OpenBSD__)
// OpenBSD has no api to get executable path.
// So, we need to guess it from argv[0]
static char *getArgv0() {
    char **argv;
    size_t len;
    int mib[4] = { CTL_KERN, KERN_PROC_ARGS, getpid(), KERN_PROC_ARGV };
    if (sysctl(mib, 4, NULL, &len, NULL, 0) < 0)
        return NULL;
    argv = malloc(len);
    if (argv == NULL)
        return NULL;
    if (sysctl(mib, 4, argv, &len, NULL, 0) < 0) {
        free(argv);
        return NULL;
    }
    char *argv0 = envuAllocStrWithConst(argv[0]);
    free(argv);
    return argv0;
}

static inline char *getExecutablePathOpenBSD() {
    // try readlink
    char path[PATH_MAX + 1];
    path[PATH_MAX] = 0;

    // try to get argv[0]
    char *argv0 = getArgv0();
    if (argv0 == NULL)  // failed to get argv[0]
        return NULL;

    if (*argv0 == '/' || *argv0 == '.') {
        // argv[0] is an absolute path or a related path
        return envuGetRealPath(argv0);
    }

    // Assume that argv[0] exists in one of environment paths
    int count = 0;
    char **paths = envuGetEnvPaths(&count);
    if (paths == NULL) {
        envuFree(argv0);
        return NULL;
    }

    for (char **p = paths; p < paths + count; p++) {
        int len = strlen(*p);

        // concat an environment path with argv[0]
        char *abs_path = envuAllocStrWithConst(*p);
        if ((*p)[len - 1] == '/') {
            abs_path = envuAppendStr(abs_path, argv0);
        } else {
            abs_path = envuAppendStr(abs_path, "/");
            abs_path = envuAppendStr(abs_path, argv0);
        }

        char *fullpath = envuGetRealPath(abs_path);
        envuFree(abs_path);

        if (fullpath != NULL) {
            // Found exe path
            envuFreeEnvPaths(paths);
            envuFree(argv0);
            return fullpath;
        }
    }

    // Failed to get exe path
    envuFreeEnvPaths(paths);
    envuFree(argv0);
    return NULL;
}
#elif defined(__HAIKU__)
// Haiku OS requires get_next_image_info to get the executable path.
static inline char *getExecutablePathHaiku() {
    int32_t cookie = 0;
    image_info info;
    while (get_next_image_info(B_CURRENT_TEAM, &cookie, &info) == B_OK) {
        if (info.type == B_APP_IMAGE)
            return envuAllocStrWithConst(info.name);
    }
    return NULL;
}
#else
static int tryReadlink(const char *link, char *path, int path_size) {
    int new_path_size;
    if (path_size != 0)
        return path_size;
    new_path_size = readlink(link, path, PATH_MAX);
    if (new_path_size == -1)
        new_path_size = 0;
    return new_path_size;
}

static inline char *getExecutablePathProcfs() {
    // get an executable path with readlink()
    char path[PATH_MAX + 1];
    path[PATH_MAX] = 0;
    int path_size = 0;
#ifdef __linux__
    path_size = tryReadlink("/proc/self/exe", path, path_size);  // Linux
#elif defined(__NetBSD__)
    path_size = tryReadlink("/proc/curproc/exe", path, path_size);  // NetBSD
#elif defined(__sun)
    path_size = tryReadlink("/proc/self/path/a.out", path, path_size);  // Solaris
#else
    path_size = tryReadlink("/proc/curproc/file", path, path_size);  // Other BSD variants?
    // Try others
    path_size = tryReadlink("/proc/self/exe", path, path_size);
    path_size = tryReadlink("/proc/curproc/exe", path, path_size);
    path_size = tryReadlink("/proc/self/path/a.out", path, path_size);
#endif
    path[path_size] = 0;

    if (*path != '\0') {
#ifdef __NetBSD__
        // procfs does not resolve paths on NetBSD.
        // So, we need to do it by ourselves.
        return envuGetRealPath(path);
#else
        return envuAllocStrWithConst(path);
#endif
    }
    // Failed to get exe path
    return NULL;
}
#endif

char *envuGetExecutablePath() {
#ifdef __APPLE__
    return getExecutablePathApple();
#elif defined(__FreeBSD__)
    return getExecutablePathFreeBSD();
#elif defined(__OpenBSD__)
    return getExecutablePathOpenBSD();
#elif defined(__HAIKU__)
    return getExecutablePathHaiku();
#else
    return getExecutablePathProcfs();
#endif
}

int envuFileExists(const char *path) {
    struct stat buffer;
    return (stat(path, &buffer) == 0) && S_ISREG(buffer.st_mode);
}

int envuPathExists(const char *path) {
    struct stat buffer;
    return stat(path, &buffer) == 0;
}

// TODO: Clean this dirty code up
char *envuGetFullPath(const char *path) {
    if (path == NULL)
        return NULL;

    char *abs_path;
    if (path[0] == '/') {
        abs_path = envuAllocStrWithConst(path);
    } else {
        // append working directory
        abs_path = envuGetCwd();
        abs_path = envuAppendStr(abs_path, "/");
        abs_path = envuAppendStr(abs_path, path);
    }

    char *resolved = envuAllocStr(strlen(abs_path));
    if (abs_path == NULL || resolved == NULL) {
        envuFree(abs_path);
        envuFree(resolved);
        return NULL;
    }

    char *abs_p = abs_path;
    char *res_p = resolved;
    int dot_count = 0;
    int is_not_dot = 0;
    while (*abs_p != 0) {
        if (*abs_p == '/') {
            if (dot_count > 0 && dot_count <= 2 && is_not_dot == 0) {
                // back to a previous slash when "." or ".." found
                while (dot_count > 0 && res_p != resolved) {
                    res_p--;
                    while (*res_p != '/' && res_p != resolved) {
                        res_p--;
                    }
                    dot_count--;
                }
            }
            dot_count = 0;
            is_not_dot = 0;
        } else if (*abs_p == '.') {
            dot_count++;
        } else {
            dot_count = 0;
            is_not_dot = 1;
        }
        *res_p = *abs_p;
        res_p++;
        abs_p++;
    }
    if (dot_count > 0 && dot_count <= 2 && is_not_dot == 0) {
        // back to a previous slash when "." or ".." found
        while (dot_count > 0 && res_p != resolved) {
            res_p--;
            while (*res_p != '/' && res_p != resolved) {
                res_p--;
            }
            dot_count--;
        }
        res_p++;
    }
    *res_p = 0;
    if (res_p > resolved + 1) {
        // remove the last slash
        res_p--;
        if (*res_p == '/')
            *res_p = '\0';
    }
    char *ret = envuAllocStrWithConst(resolved);
    envuFree(abs_path);
    envuFree(resolved);
    return ret;
}

char *envuGetDirectory(const char *path) {
    if (path == NULL)
        return NULL;
    // dirname() may modify the contents of path. So, we use copied one.
    char *copied_path = envuAllocStrWithConst(path);
    char *dir = dirname(copied_path);
    if (dir == NULL) {
        free(copied_path);
        return NULL;
    }
    char *ret = envuAllocStrWithConst(dir);
    free(copied_path);
    return ret;
}

char *envuGetCwd() {
    char cwd[PATH_MAX + 1];
    cwd[PATH_MAX] = 0;
    char *ret = getcwd(cwd, PATH_MAX);
    return envuAllocStrWithConst(ret);
}

int envuSetCwd(const char *path) {
    if (path == NULL)
        return -1;
    int ret = chdir(path);
    return -(ret != 0);
}

char *envuGetEnv(const char *name) {
    if (name == NULL)
        return NULL;
    char *str = getenv(name);
    if (str == NULL)
        return NULL;
    return envuAllocStrWithConst(str);
}

int envuSetEnv(const char *name, const char *value) {
    if (name == NULL)
        return -1;
    int ret;
    if (value == NULL)
        ret = unsetenv(name);
    else
        ret = setenv(name, value, 1);
    return -(ret != 0);
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
        return NULL;
    }
    char *str = envuAllocStrWithConst(homedir);
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
        return NULL;
    }
    char *str = envuAllocStrWithConst(name);
    free(buf);
    return str;
}

// Darwin, Linux, FreeBSD, OpenBSD, NetBSD, Haiku, SunOS, etc.
char *envuGetOS() {
    struct utsname buf = { 0 };
    // Note: uname(&buf) can be positive on Solaris
    if (uname(&buf) == -1) {
        return NULL;
    }
    return envuAllocStrWithConst(buf.sysname);
}

char *envuGetOSVersion() {
    struct utsname buf = { 0 };
    // Note: uname(&buf) can be positive on Solaris
    if (uname(&buf) == -1) {
        return NULL;
    }
    char *ver = buf.release;
    if (ver == NULL || *ver == '\0')
        return NULL;

    // buf.release could be of the form x.y.z-*
    // We try to make it numeric (x.y.z) here.
    char *vp = ver;
    while (is_numeric(*vp)) {
        vp++;
    }
    if (vp != ver && *vp == '-') {
        // replace the first '-' with a null terminator.
        *vp = '\0';
    }

    return envuAllocStrWithConst(ver);
}

#ifdef __APPLE__
CFDictionaryRef _CFCopyServerVersionDictionary();
CFDictionaryRef _CFCopySystemVersionDictionary();

static char *CFStoChar(CFStringRef cfstr) {
    // Note: The length of string should be smaller than 256.
    char str[256];
    int ret = CFStringGetCString(cfstr, str, sizeof(str), kCFStringEncodingUTF8);
    if (ret) {
        str[255] = '\0';
        return envuAllocStrWithConst(str);
    }
    return NULL;
}

static inline char *getOSProductNameApple() {
    // Get ProductName and ProductVersion
    // from /System/Library/CoreServices/*Version.plist

    CFDictionaryRef dict = NULL;
    CFStringRef prod_name = NULL;
    CFStringRef prod_ver = NULL;

    // Try ServerVersion.plist
    dict = _CFCopyServerVersionDictionary();
    if (dict == NULL) {
        // Try SystemVersion.plist
        dict = _CFCopySystemVersionDictionary();
    }
    if (dict == NULL)
        return NULL;  // Failed to get *.plist

    prod_name = CFDictionaryGetValue(dict, CFSTR("ProductName"));
    if (prod_name == NULL)
        return NULL;

    prod_ver = CFDictionaryGetValue(dict, CFSTR("ProductVersion"));
    if (prod_ver == NULL) {
        // Return prod_name
        char *cstr = CFStoChar(prod_name);
        CFRelease(prod_name);
        return cstr;
    }

    // Concat prod_name and prod_ver
    CFStringRef str = NULL;
    str = CFStringCreateWithFormat(
            NULL, NULL, CFSTR("%@ %@"), prod_name, prod_ver);
    CFRelease(prod_name);
    CFRelease(prod_ver);
    if (str == NULL)
        return NULL;

    char *cstr = CFStoChar(str);
    CFRelease(str);
    return cstr;
}
#elif defined(__linux__)
static inline char *getOSProductNameLinux() {
    // Get the value of "PRETTY_NAME" in /etc/os-release
    FILE *fptr;
    fptr = fopen("/etc/os-release", "r");
    if (!fptr)
        return NULL;

    char *lineptr = NULL;
    size_t linemax = 0;
    ssize_t size = 0;
    const char key[] = "PRETTY_NAME";
    char *pretty_name = NULL;
    while (1) {
        // get a line.
        size = getline(&lineptr, &linemax, fptr);
        if (size < 0)
            break;
        lineptr[size - 1] = '\0';

        // check if a key is "PRETTY_NAME"
        const char *lp = lineptr;
        const char *kp = key;
        while (*kp != '\0' && *lp != '\0' && *lp != '=' && *kp == *lp) {
            lp++;
            kp++;
        }
        if (*kp != '\0' || *lp != '=') {
            // The key is not "PRETTY_NAME"
            continue;
        }
        lp++;

        // remove quotes if exist
        if (*lp == '"')
            lp++;
        if (lineptr[size - 2] == '"')
            lineptr[size - 2] = '\0';

        pretty_name = envuAllocStrWithConst(lp);
        break;
    }
    fclose(fptr);
    free(lineptr);
    return pretty_name;
}
#elif defined(__sun)
static inline char *getOSProductNameSolaris() {
    // Get the first alphanumeric part in /etc/release
    FILE *fptr;
    fptr = fopen("/etc/release", "r");
    if (!fptr)
        return NULL;

    char *lineptr = NULL;
    size_t linemax = 0;
    ssize_t size = 0;
    size = getline(&lineptr, &linemax, fptr);
    lineptr[size - 1] = '\0';
    fclose(fptr);

    // skip white spaces
    char *start_p = lineptr;
    while (*start_p == ' ' && *start_p != '\0') {
        start_p++;
    }
    if (*start_p == '\0') {
        free(lineptr);
        return NULL;
    }

    // cut non-alphanumeric part off.
    char *end_p = start_p;
    while (*end_p != '\0' && (is_alphabet(*end_p) || is_numeric(*end_p) || *end_p == ' ')) {
        end_p++;
    }
    if (*end_p != *start_p && *(end_p - 1) == ' ') {
        end_p--;
    }
    *end_p = '\0';

    char *pretty_name = envuAllocStrWithConst(start_p);
    free(lineptr);
    return pretty_name;
}
#else
static inline char *getOSProductNameOthers() {
    // concat envuGetOS and envuGetOSVersion on other platforms.
    char *os = envuGetOS();
    if (os == NULL)
        return NULL;

#ifdef __HAIKU__
    // Haiku requires native APIs to get the true version string.
    // https://discuss.haiku-os.org/t/getting-the-haiku-version/13899
    char *os_ver = getOSVersionHaiku();
#else
    char *os_ver = envuGetOSVersion();
#endif
    if (os_ver == NULL)
        return os;

    os = envuAppendStr(os, " ");
    os = envuAppendStr(os, os_ver);
    return os;
}
#endif

char *envuGetOSProductName() {
#ifdef __APPLE__
    return getOSProductNameApple();
#elif defined(__linux__)
    return getOSProductNameLinux();
#elif defined(__sun)
    return getOSProductNameSolaris();
#else
    return getOSProductNameOthers();
#endif
}
