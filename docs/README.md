# c-env-utils

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

c-env-utils is a cross-platform C library to get environment information such as user name, working directory, and executable path.

## Documentation

Most of the functions are here.  
[c-env-utils: include/env_utils.h File Reference](https://matyalatte.github.io/c-env-utils/env__utils_8h.html)  

c-env-utils only supports utf8 strings.  
You should use `env_utils_windows.h` if you want to use utf16 strings on Windows.
[c-env-utils: include/env_utils_windows.h File Reference](https://matyalatte.github.io/c-env-utils/env__utils__windows_8h.html)  

## Example

```c
#include <stdio.h>
#include "env_utils.h"

int main(void) {
    // Executable path
    char *exe_path = envuGetExecutablePath();
    printf("Exe: %s\n", exe_path);
    envuFree(exe_path);

    // Working directory
    char *cwd = envuGetCwd();
    printf("CWD: %s\n", cwd);
    envuFree(cwd);

    // Username
    char *username = envuGetUsername();
    printf("User: %s\n", username);
    envuFree(username);

    // Get an environment variable
    char *paths = envuGetEnv("PATH");
    printf("PATH: %s\n", paths);
    envuFree(paths);
    return 0;
}
```

## Building

### Requirements

- [Meson](https://mesonbuild.com/) for building

### Build Whole Project

```bash
meson setup build
meson compile -C build
```

### Build Library Only

```bash
meson setup build -Dcli=false -Dtests=false
meson compile -C build
```
