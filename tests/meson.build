if meson.is_cross_build()
    error('unit tests does NOT support cross build.')
endif

true_build_dir = meson.current_build_dir().replace('\\', '\\\\')
true_source_root = meson.project_source_root().replace('\\', '\\\\')
true_exe_name = 'env_utils_test'
if envu_OS == 'windows'
    true_exe_name += '.exe'
endif

test_conf = configuration_data()
test_conf.set('BUILD_DIR', true_build_dir)
test_conf.set('SOURCE_ROOT', true_source_root)
test_conf.set('EXE_NAME', true_exe_name)
test_conf.set('OS', envu_OS)
configure_file(input : 'true_env_info.h.in',
               output : 'true_env_info.h',
               configuration : test_conf)

test_exe = executable('env_utils_test',
    'main.cpp',
    dependencies : [env_utils_dep, gtest_dep, gmock_dep],
    install : false)

test('env_utils_test', test_exe, workdir: true_source_root)
