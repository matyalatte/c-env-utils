project('c-env-utils', ['c'],
    meson_version: '>=0.58.0',
    default_options: [
        'buildtype=debug',              # build debug by default
        'default_library=shared',       # build shared libraries by default
        'warning_level=3',              # always max warnings
        'b_pch=false',                  # we don't want precompiled headers
        'b_staticpic=true',             # use PIC even for static libraries
        'c_std=c99',                    # strict C99
        'c_winlibs=',                   # we define our own Windows libraries
        'cpp_std=c++11',                # strict C++11
        'cpp_eh=sc',                    # shut the compiler up in some cases
        'cpp_winlibs=',                 # likewise as with c_winlibs
    ],
    version: '0.3.0')

envu_link_args = []
envu_c_args = []
envu_c_only_args = []
envu_OS = host_machine.system()
envu_compiler = meson.get_compiler('c').get_id()
envu_is_release = get_option('buildtype').startswith('release') or (get_option('buildtype').startswith('custom') and not get_option('debug'))

# Requires cpp on Windows and Haiku
if envu_OS == 'windows' or envu_OS == 'haiku'
    add_languages('cpp', native: false, required: true)
endif

# set compiler and linker options
if envu_OS == 'darwin' and not meson.is_subproject()
    languages = ['c']
    macosx_version_min = '-mmacosx-version-min=' + get_option('macosx_version_min')
    add_global_arguments(macosx_version_min, language: languages)
    add_global_link_arguments(macosx_version_min, language: languages)

    # Check if SDKs support universal binaries or not.
    arch = ['-arch', 'x86_64', '-arch', 'arm64']
    c_compiler = meson.get_compiler('c')
    result = c_compiler.run(
        'int main(void) { return 0; }',
        name : 'universal binary test',
        args: arch)
    if result.compiled()
        add_global_arguments(arch, language: languages)
        add_global_link_arguments(arch, language: languages)
    else
        warning('Universal build is disabled since your SDKs do not support it.')
    endif
endif

if envu_OS == 'freebsd'
    # Ignore some warnings on FreeBSD
    envu_c_args += ['-Wno-c11-extensions']
endif

if envu_compiler != 'msvc'
    # Show warnings for strict-prototypes
    envu_c_only_args += ['-Wstrict-prototypes']
endif

# set source files
envu_sources = ['src/common.c']
if envu_OS == 'windows'
    envu_sources += ['src/windows.c', 'src/wmi.cpp']
else
    envu_sources += ['src/unix.c']
endif
if envu_OS == 'haiku'
    envu_sources += ['src/haiku.cpp']
endif

# set dynamic linked libraries
envu_lib_deps = []
if envu_OS == 'windows'
    foreach lib : ['kernel32', 'advapi32', 'ole32', 'oleaut32', 'wbemuuid']
        envu_lib_deps += [
            meson.get_compiler('c').find_library(lib,
                required: true),
        ]
    endforeach
elif envu_OS == 'darwin'
    envu_lib_deps += [
        dependency('appleframeworks',
            modules : 'CoreFoundation',
            required: true),
    ]
elif envu_OS == 'haiku'
    envu_lib_deps += [
        meson.get_compiler('c').find_library('be',
            required: true),
    ]
endif

# main binary
env_utils = library('env_utils',
    envu_sources,
    dependencies: envu_lib_deps,
    c_args: envu_c_args + envu_c_only_args,
    cpp_args: envu_c_args,
    link_args: envu_link_args,
    install: true,
    include_directories: include_directories('./include'),
    gnu_symbol_visibility: 'hidden')
install_headers('include/env_utils.h')
if envu_OS == 'windows'
    install_headers('include/env_utils_windows.h')
endif

# dependency for other projects
env_utils_dep = declare_dependency(
    include_directories: include_directories('./include'),
    dependencies: envu_lib_deps,
    link_with : env_utils)

if get_option('cli')
    # cli
    executable('env_utils_cli',
        envu_sources + ['src/env_utils_cli.c'],
        dependencies: env_utils_dep,
        c_args: envu_c_args + envu_c_only_args,
        cpp_args: envu_c_args,
        install : false)
endif

# Build unit tests
if get_option('tests')
    add_languages('cpp', native:false, required: true)

    # get gtest
    gtest_proj = subproject('gtest')
    gtest_dep = gtest_proj.get_variable('gtest_dep')
    gmock_dep = gtest_proj.get_variable('gmock_dep')

    # build tests
    subdir('tests')
endif
