#ifndef __C_ENV_UTILS_INCLUDE_ENV_UTILS_H__
#define __C_ENV_UTILS_INCLUDE_ENV_UTILS_H__
#include <stdlib.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _ENVU_EXTERN
#ifdef _WIN32
#define _ENVU_EXTERN __declspec(dllexport) extern
#else
#define _ENVU_EXTERN __attribute__((visibility("default"))) extern
#endif
#endif

#define _ENVU_ENUM(s) typedef unsigned int s; enum

// Version info
#define ENVU_VERSION "0.1.0"
#define ENVU_VERSION_INT 100

/**
 * Gets the version of c-env-utils.
 * It should be of the form `x.x.x`.
 *
 * @returns A string that represents the version.
 */
_ENVU_EXTERN const char* envuGetVersion();

/**
 * Gets the version of c-env-utils as an integer.
 * The value should be `major * 100 + minor * 100 + patch`.
 * If `revGetVersion() == "1.2.3"` then `revGetVersionAsInt() == 10203`.
 *
 * @returns An integer that represents the version.
 */
_ENVU_EXTERN int envuGetVersionAsInt();

/**
 * Frees the memory of a string allocated by c-env-utils.
 *
 * @param p A pointer to a string.
 */
_ENVU_EXTERN void envuFree(void *p);

/**
 * Gets the path to the executing binary.
 *
 * @note Strings that are returned from this method should be freed with envuFree().
 *
 * @returns A string that represents the path to the executing binary. Or a null pointer if failed.
 */
_ENVU_EXTERN char *envuGetExecutablePath();

/**
 * Returns if the specified path is a file for not.
 *
 * @param path A path.
 * @returns If the specified path is a regular file or not.
 */
_ENVU_EXTERN int envuFileExists(const char *path);

/**
 * Gets a full path of the specified path.
 *
 * @note Strings that are returned from this method should be freed with envuFree().
 *
 * @param path A path.
 * @returns A full path of the specified path. Or a null pointer if failed.
 */
_ENVU_EXTERN char *envuGetFullPath(const char *path);

/**
 * Gets a parent directory of the specified path.
 *
 * @note Strings that are returned from this method should be freed with envuFree().
 *
 * @param path A path.
 * @returns A parent directory of the specified path. Or a null pointer if failed.
 */
_ENVU_EXTERN char *envuGetDirectory(const char *path);

/**
 * Gets the directory of the executing binary.
 *
 * @note Strings that are returned from this method should be freed with envuFree().
 *
 * @returns A string that represents the directory of the executing binary. Or a null pointer if failed.
 */
_ENVU_EXTERN char *envuGetExecutableDir();

/**
 * Gets the current working directory.
 *
 * @note Strings that are returned from this method should be freed with envuFree().
 *
 * @returns A string that represents the current working directory. Or a null pointer if failed.
 */
_ENVU_EXTERN char *envuGetCwd();

/**
 * Sets the current working directory.
 *
 * @param path A string that represents a directory.
 * @returns 0 if successful. -1 indicates failure.
 */
_ENVU_EXTERN int envuSetCwd(const char *path);

/**
 * Gets a value of an environment variable.
 *
 * @note Strings that are returned from this method should be freed with envuFree().
 *
 * @param name A name of an environment variable.
 * @returns A value of an environment variable. Or a null pointer if failed.
 */
_ENVU_EXTERN char *envuGetEnv(const char *name);

/**
 * Sets an environment variable.
 *
 * @note On Windows, empty strings will be treated as null pointers.
 *
 * @param name A name of an environment variable.
 * @param value A value of an environment variable. Or a null pointer to remove the variable.
 * @returns 0 if successful. -1 indicates failure.
 */
_ENVU_EXTERN int envuSetEnv(const char *name, const char *value);

/**
 * Gets user's home directory.
 *
 * @note Strings that are returned from this method should be freed with envuFree().
 *
 * @returns A string that represents user's home directory. Or a null pointer if failed.
 */
_ENVU_EXTERN char *envuGetHome();

/**
 * Gets user name.
 *
 * @note Strings that are returned from this method should be freed with envuFree().
 *
 * @returns A string that represents user name. Or a null pointer if failed.
 */
_ENVU_EXTERN char *envuGetUsername();

/**
 * Gets the name of running OS.
 * e.g. "Windows" for Windows, "Darwin" for macOS, and "Linux" for Linux distros.
 *
 * @note Strings that are returned from this method should be freed with envuFree().
 *
 * @returns A string that represents running OS. Or a null pointer if failed.
 */
_ENVU_EXTERN char *envuGetOS();

/**
 * Gets the environment paths from the PATH variable.
 *
 * @note Arrays that are returned from this method should be freed with envuFreeEnvPaths().
 *
 * @param path_count The number of paths will be stored here if it's not a null pointer.
 * @returns A null-terminated array of strings. Or a null pointer if failed.
 */
_ENVU_EXTERN char **envuGetEnvPaths(int *path_count);

/**
 * Parses the PATH variable.
 * Its paths should be separated by semicolons on Windows, or by colons on other platforms.
 *
 * @note Arrays that are returned from this method should be freed with envuFreeEnvPaths().
 *
 * @param env_path The value of the PATH variable.
 * @param path_count The number of paths will be stored here if it's not a null pointer.
 * @returns A null-terminated array of strings. Or a null pointer if failed.
 */
_ENVU_EXTERN char **envuParseEnvPaths(const char *env_path, int *path_count);

/**
 * Frees the memory of an array allocated by envuGetEnvPaths() or envuParseEnvPaths().
 *
 * @param paths A null-terminated array of strings.
 */
_ENVU_EXTERN void envuFreeEnvPaths(char **paths);

#ifdef __cplusplus
}
#endif

#endif  // __C_ENV_UTILS_INCLUDE_ENV_UTILS_H__
