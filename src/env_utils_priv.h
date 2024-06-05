#ifndef __C_ENV_UTILS_INCLUDE_ENV_UTILS_PRIV_H__
#define __C_ENV_UTILS_INCLUDE_ENV_UTILS_PRIV_H__

#ifdef __cplusplus
extern "C" {
#endif

extern char **ParseEnvPathsBase(const char *env_path, int *path_count, char delim);

extern char *AllocStr(size_t size);
#define AllocEmptyStr() AllocStr(0)
extern char *AllocStrWithConst(const char *c);

#ifdef __cplusplus
}
#endif

#endif  // __C_ENV_UTILS_INCLUDE_ENV_UTILS_PRIV_H__