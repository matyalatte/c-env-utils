if meson.is_cross_build()
    error('unit tests does NOT support cross build.')
endif

fs = import('fs')

# get expected results of unit tests
true_build_dir = meson.current_build_dir().replace('\\', '\\\\')
true_build_dir_name = fs.name(true_build_dir)
true_source_root = meson.project_source_root().replace('\\', '\\\\')
true_exe_name = 'env_utils_test'
true_home = fs.expanduser('~').replace('\\', '\\\\')
r = run_command('whoami', check: true)
true_username = r.stdout().strip().split('\\')[-1]

if envu_OS == 'windows'
    true_exe_name += '.exe'
    true_os = 'Windows'
    # get os version via wmi
    r = run_command('powershell', 'Get-WmiObject -Query "SELECT Version FROM Win32_OperatingSystem"'
                    + ' | Select-Object -ExpandProperty Version',
                    check: true)
    true_os_version = r.stdout().strip()
    # get os name via wmi
    r = run_command('powershell','Get-WmiObject -Query "SELECT Caption FROM Win32_OperatingSystem"'
                    + ' | Select-Object -ExpandProperty Caption',
                    check: true)
    true_os_product_name = r.stdout().strip()
else
    # get os with uname
    r = run_command('uname', '-s', check: true)
    true_os = r.stdout().strip()
    # get os version with uname
    r = run_command('uname', '-r', check: true)
    true_os_version = r.stdout().strip().split('-')[0]
    if envu_OS == 'darwin'
        # get os name from SystemVersion.plist
        r = run_command('sw_vers', '-productName', check: true)
        prod_name = r.stdout().strip()
        r = run_command('sw_vers', '-productVersion', check: true)
        prod_version = r.stdout().strip()
        true_os_product_name = prod_name + ' ' + prod_version
    elif envu_OS == 'linux'
        # get os name from /etc/os-release
        r = run_command('grep', 'PRETTY_NAME', '/etc/os-release', check: true)
        true_os_product_name = r.stdout().strip().split('=')[1].strip('"')
    elif envu_OS == 'haiku'
        true_os_product_name = 'failed'
    elif envu_OS == 'sunos'
        # get os name from /etc/release
        r = run_command('head', '-1', '/etc/release', check: true)
        true_os_product_name = r.stdout().split('(')[0].strip()
    else
        true_os_product_name = true_os + ' ' + true_os_version
    endif
endif

# write results in true_env_info.h
test_conf = configuration_data()
test_conf.set('BUILD_DIR', true_build_dir)
test_conf.set('BUILD_DIR_NAME', true_build_dir_name)
test_conf.set('SOURCE_ROOT', true_source_root)
test_conf.set('EXE_NAME', true_exe_name)
test_conf.set('OS', true_os)
test_conf.set('HOME', true_home)
test_conf.set('USERNAME', true_username)
test_conf.set('OS_VERSION', true_os_version)
test_conf.set('OS_PRODUCT_NAME', true_os_product_name)
if envu_OS == 'windows'
    drive_name = true_source_root.split(':')[0]
    test_conf.set('WIN_DRIVE', drive_name)
else
    test_conf.set('WIN_DRIVE', '')
endif
configure_file(input : 'true_env_info.h.in',
               output : 'true_env_info.h',
               configuration : test_conf)

if envu_OS != 'windows'
    # unit tests use 'ln' and 'rm' commands.
    find_program('ln', required: true)
    find_program('rm', required: true)

    # unit tests use another executable.
    subdir('test_cli')
endif

test_exe = executable('env_utils_test',
    'main.cpp',
    dependencies : [env_utils_dep, gtest_dep, gmock_dep],
    install : false)

test('env_utils_test', test_exe, workdir: true_source_root)
