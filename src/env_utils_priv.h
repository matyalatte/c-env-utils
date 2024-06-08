#ifndef __C_ENV_UTILS_INCLUDE_ENV_UTILS_PRIV_H__
#define __C_ENV_UTILS_INCLUDE_ENV_UTILS_PRIV_H__

#ifdef __cplusplus
extern "C" {
#endif

#define is_numeric(c) ((c >= '0' && c <= '9') || c == '.')
#define is_alphabet(c) ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))

/**
 * Allocates memory blocks of a string.
 *
 * @param size The buffer size of a string excluding the null terminator.
 * @return A pointer to an allocated string. Or a null pointer if failed.
 */
extern char *envuAllocStr(size_t size);
#define envuAllocEmptyStr() envuAllocStr(0)

/**
 * Allocates memory blocks and initialize them with a string.
 *
 * @param c An initial value for the allocated string.
 * @return A pointer to an allocated string. Or a null pointer if failed.
 */
extern char *envuAllocStrWithConst(const char *c);

/**
 * Reallocates memory blocks of a string and appends another string to it.
 *
 * @warning Don't free str1 after this function because return value takes its ownership.
 *
 * @param str1 A pointer to a string that will be reallocated.
 * @param str2 A pointer to a string that will be appended to str1.
 * @return A pointer to an reallocated string. Or a null pointer if failed.
 */
extern char *envuAppendStr(char *str1, const char *str2);

#ifdef _WIN32
extern wchar_t *envuAllocWstr(size_t size);
#define envuAllocEmptyWstr() envuAllocWstr(0)
extern wchar_t *envuAllocWstrWithConst(const wchar_t *c);
extern wchar_t *getOSInfoFromWMI(const wchar_t *key);
#endif

#ifdef __HAIKU__
extern char *getOSVersionHaiku();
#endif

#ifdef __cplusplus
}
#endif

#endif  // __C_ENV_UTILS_INCLUDE_ENV_UTILS_PRIV_H__
