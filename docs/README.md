# c-env-utils

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

c-env-utils is a cross-platform C library to get environment information such as executable path, user name, and OS version.

## Documentation

Most of the functions are here.  
[c-env-utils: include/env_utils.h File Reference](https://matyalatte.github.io/c-env-utils/env__utils_8h.html)  

c-env-utils only supports utf8 strings.  
You should use `env_utils_windows.h` if you want to use utf16 strings on Windows.  
[c-env-utils: include/env_utils_windows.h File Reference](https://matyalatte.github.io/c-env-utils/env__utils__windows_8h.html)  

## Platforms

c-env-utils supports most of the desktop operating systems.

- Windows
- macOS
- Linux
- BSD
- Haiku
- Solaris
- Other unix variants (maybe)

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

    // OS
    char *prodname = envuGetOSProductName();
    printf("OS: %s\n", prodname);
    envuFree(prodname);

    // Parse the PATH variable
    int count;
    char **paths = envuGetEnvPaths(&count);
    printf("%s", "PATH:\n");
    for (char **p = paths; p < paths + count; p++) {
        printf("  %s\n", *p);
    }
    envuFreeEnvPaths(paths);
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
meson test -C build
```

### Build Library Only

```bash
meson setup build -Dcli=false -Dtests=false
meson compile -C build
```

### Build as Subproject

You don't need to clone the git repo if you build your project with meson.  
Save the following text as `subprojects/env_utils.wrap`.  

```ini
[wrap-git]
url = https://github.com/matyalatte/c-env-utils.git
revision = head
depth = 1

[provide]
env_utils = env_utils_dep
```

Then, you can use c-env-utils in your meson project.

```python
env_utils_dep = dependency('env_utils', fallback : ['env_utils', 'env_utils_dep'])
executable('your_exe_name', ['your_code.cpp'], dependencies : [env_utils_dep])
```

```bash
meson setup build -Denv_utils:cli=false -Denv_utils:tests=false
meson compile -C build
```

## Projects Which Use c-env-utils

- [Tuw](https://github.com/matyalatte/tuw): A tiny GUI wrapper for command-line tools.
