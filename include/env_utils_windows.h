#ifndef __C_ENV_UTILS_INCLUDE_ENV_UTILS_WINDOWS_H__
#define __C_ENV_UTILS_INCLUDE_ENV_UTILS_WINDOWS_H__
#include "env_utils.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Convert a UTF8 string to UTF16.
 *
 * @note Strings that are returned from this method should be freed with envuFree().
 *
 * @param str a UTF8 string.
 * @returns A UTF16 string.
 */
_ENVU_EXTERN wchar_t *envuUTF8toUTF16(const char* str);

/**
 * Convert a UTF16 string to UTF8.
 *
 * @note Strings that are returned from this method should be freed with envuFree().
 *
 * @param wstr a UTF16 string.
 * @returns A UTF8 string.
 */
_ENVU_EXTERN char *envuUTF16toUTF8(const wchar_t* wstr);

#ifdef __cplusplus
}
#endif

#endif  // __C_ENV_UTILS_INCLUDE_ENV_UTILS_WINDOWS_H__
