# vim:fileencoding=utf-8:sw=4:tw=4:et
########################################################################################
# Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.
########################################################################################
# $Id: hilscher_toolchain_host.py 848 2023-10-02 07:12:39Z AMesser $:
#
# A toolchain to build host applications
########################################################################################
from waflib.Configure import conf
import sys

def configure(conf):
    env = conf.declare_toolchain('gcc-linux')
    env['TOOLCHAIN_TRIPLE'] = 'none-linux-gnu'

    with conf.ToolchainDetectContext():
        configure_toolchain_gcclinux(conf)

        conf.setenv('toolchain_native',env)
        conf.setenv('toolchain_gcc',env)

        conf.declare_platform('native', 'gcc-linux', setup_device_linux, target_triple_machine)

    # make some aliases
    conf.setenv('toolchain_linux',env)
    conf.setenv('toolchain_posix',env)

    env = conf.declare_toolchain('mingw32')
    env['TOOLCHAIN_TRIPLE'] = 'none-pc-mingw32'

    with conf.ToolchainDetectContext():
        configure_toolchain_mingw32(conf)

        conf.declare_platform('native', 'mingw32', setup_device_mingw, target_triple_machine)

        conf.setenv('toolchain_native',env)
        conf.setenv('toolchain_gcc',env)
        conf.setenv('toolchain_win',env)

    conf.setenv('toolchain_mingw',env)

    env = conf.declare_toolchain('mingw64')
    env['TOOLCHAIN_TRIPLE'] = 'none-pc-mingw64'

    with conf.ToolchainDetectContext():
        configure_toolchain_mingw64(conf)

        conf.declare_platform('native', 'mingw64', setup_device_mingw, target_triple_machine)

        if 'mingw' in conf.all_envs:
            del conf.all_envs['mingw']

        if 'mative' in conf.all_envs:
            del conf.all_envs['native']

        conf.setenv('toolchain_mingw',env)
        conf.setenv('toolchain_native',env)
        conf.setenv('toolchain_gcc',env)
        conf.setenv('toolchain_win',env)


    conf.declare_platform('linux', 'gcc-linux', setup_device_linux, target_triple_machine)
    conf.declare_platform('win32', 'mingw32',   setup_device_mingw, target_triple_machine)
    conf.declare_platform('win64', 'mingw64',   setup_device_mingw, target_triple_machine)

def configure_toolchain_gcclinux(conf):
    if sys.platform in ("linux","linux2"):
        conf.setup_gnu_gcc_toolchain(prefix='')
        conf.gcc_optimize_flags()
        conf.env.append_value('LINKFLAGS', ['-Wl,-gc-sections'])
    else:
        conf.fatal('GCC Linux toolchain not available none linux os')

def configure_toolchain_mingw32(conf):
    if sys.platform in ("win32","win64"):
        conf.setup_gnu_gcc_toolchain(prefix = 'mingw32-')
        conf.gcc_optimize_flags()

        conf.env['DEST_OS'] = 'win32'
        conf.gcc_modifier_platform()
        conf.env.append_value('LINKFLAGS', ['-Wl,-gc-sections'])
    else:
        conf.fatal('MinGW toolchain not available none windows os')

def configure_toolchain_mingw64(conf):
    if sys.platform in ("win64"):
        conf.setup_gnu_gcc_toolchain(prefix = 'mingw64')
        conf.gcc_optimize_flags()

        conf.env['DEST_OS'] = 'win32'
        conf.gcc_modifier_platform()
        conf.env.append_value('LINKFLAGS', ['-Wl,-gc-sections'])
    else:
        conf.fatal('MinGW 64 toolchain not available none 64 bit windows os')

@conf
def target_triple_machine(bld, platform):
    return bld.env['MACHINE'] or bld.env['TOOLCHAIN_TRIPLE']

@conf
def setup_device_linux(bld):
    bld.gcc_flags()
    bld.host_optimize_flags()

    f = bld.env.append_value

    f('STLIB',   ['m', 'c', 'gcc'])
    f('TOOL_OPTIONS', ["linkerscript_optional"])

@conf
def setup_device_mingw(bld):
    bld.gcc_flags()
    bld.host_optimize_flags()

    f = bld.env.append_value

    f('TOOL_OPTIONS', ["linkerscript_optional"])

@conf
def host_optimize_flags(conf):
    f = conf.env.append_value

    if conf.env.CONDITIONS == 'debug':
        f('CFLAGS__optimize',   ['-O0'])
        f('CXXFLAGS__optimize', ['-O0'])
    else:
        f('CFLAGS__optimize',   ['-O3'])
        f('CXXFLAGS__optimize', ['-O3'])
        f('DEFINES__optimize',  ['NDEBUG=1'])
