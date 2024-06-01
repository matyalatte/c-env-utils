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
 * Initializes environment variables with the passed values from the parent process.
 *
 * @note This function should not be called twice in a process.
 */
_ENVU_EXTERN void envuInit();

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
 * @returns A string that represents the path to the executing binary.
 */
_ENVU_EXTERN char *envuGetExecutablePath();

/**
 * Returns if the specified path is a file for not.
 *
 * @param path A path.
 * @returns If the specified path is a file for not.
 */
_ENVU_EXTERN int envuFileExists(const char *path);

/**
 * Gets a full path of the specified path.
 *
 * @note Strings that are returned from this method should be freed with envuFree().
 *
 * @param path A path.
 * @returns A full path of the specified path.
 */
_ENVU_EXTERN char *envuGetFullPath(const char *path);

/**
 * Gets a parent directory of the specified path.
 *
 * @note Strings that are returned from this method should be freed with envuFree().
 *
 * @param path A path.
 * @returns A parent directory of the specified path.
 */
_ENVU_EXTERN char *envuGetDirectory(const char *path);

/**
 * Gets the directory of the executing binary.
 *
 * @note Strings that are returned from this method should be freed with envuFree().
 *
 * @returns A string that represents the directory of the executing binary.
 */
_ENVU_EXTERN char *envuGetExecutableDir();

/**
 * Gets the current working directory.
 *
 * @note Strings that are returned from this method should be freed with envuFree().
 *
 * @returns A string that represents the current working directory.
 */
_ENVU_EXTERN char *envuGetCwd();

/**
 * Sets the current working directory.
 *
 * @param path A string that represents a directory.
 */
_ENVU_EXTERN void envuSetCwd(const char *path);

/**
 * Gets a value of an environment variable.
 *
 * @note Strings that are returned from this method should be freed with envuFree().
 *
 * @param name A name of an environment variable.
 * @returns A value of an environment variable.
 */
_ENVU_EXTERN char *envuGetEnv(const char *name);

/**
 * Sets an environment variable.
 *
 * @param name A name of an environment variable.
 * @param value A value of an environment variable.
 */
_ENVU_EXTERN void envuSetEnv(const char *name, const char *value);

/**
 * Gets user's home directory.
 *
 * @note Strings that are returned from this method should be freed with envuFree().
 *
 * @returns A string that represents user's home directory.
 */
_ENVU_EXTERN char *envuGetHome();

/**
 * Gets user name.
 *
 * @note Strings that are returned from this method should be freed with envuFree().
 *
 * @returns A string that represents user name.
 */
_ENVU_EXTERN char *envuGetUsername();

/**
 * Gets the name of running OS.
 * e.g. "Windows" for Windows, "Darwin" for macOS, and "Linux" for Linux distros.
 *
 * @note Strings that are returned from this method should be freed with envuFree().
 *
 * @returns A string that represents running OS.
 */
_ENVU_EXTERN char *envuGetOS();

#ifdef __cplusplus
}
#endif

#endif  // __C_ENV_UTILS_INCLUDE_ENV_UTILS_H__
