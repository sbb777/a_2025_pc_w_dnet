# vim:fileencoding=utf-8:sw=4:tw=4:et
########################################################################################
# Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.
########################################################################################
# $Id: hilscher_toolchain_host.py 1036 2024-10-31 07:12:36Z AMesser $:
#
# A toolchain to build host applications
########################################################################################
from waflib.Configure import conf
import sys

def configure(conf):
    # setup defaults some toolchain gcc & native
    env = conf.declare_toolchain('gcc')
    env['TOOLCHAIN_TRIPLE'] = 'none-none-gnu'

    conf.alias_toolchain('gcc', 'native')

    # prepare  linux gcc toolchain
    env = conf.declare_toolchain('gcc-linux')
    env['TOOLCHAIN_TRIPLE'] = 'none-linux-gnu'

    # default aliases & platforms for gcc-linux toolchain
    conf.alias_toolchain('gcc-linux', 'linux posix')
    conf.declare_platform('native', 'gcc-linux', setup_device_linux, target_triple_machine)
    conf.declare_platform('linux',  'gcc-linux', setup_device_linux, target_triple_machine)

    with conf.ToolchainDetectContext():
        configure_toolchain_gcclinux(conf)

        # on success alias gcc-linux as "native, gcc"
        conf.alias_toolchain('gcc-linux', 'native gcc')

    # prepare mingw32 gcc toolchain
    env = conf.declare_toolchain('mingw32')
    env['TOOLCHAIN_TRIPLE'] = 'none-pc-mingw32'

    # default aliases & platforms for mingw32 toolchain
    conf.alias_toolchain('mingw32', 'win mingw')
    conf.declare_platform('win32',  'mingw32', setup_device_mingw, target_triple_machine)
    conf.declare_platform('native', 'mingw32', setup_device_mingw, target_triple_machine)

    with conf.ToolchainDetectContext():
        configure_toolchain_mingw32(conf)

        # on success, alias mingw32 as "native, gcc"
        conf.alias_toolchain('mingw32', 'native gcc')

    # prepare mingw64 gcc toolchain
    env = conf.declare_toolchain('mingw64')
    env['TOOLCHAIN_TRIPLE'] = 'none-pc-mingw64'

    # default aliases & platforms for mingw64 toolchain
    conf.declare_platform('win64',  'mingw64', setup_device_mingw, target_triple_machine)
    conf.declare_platform('native', 'mingw64', setup_device_mingw, target_triple_machine)

    with conf.ToolchainDetectContext():
        configure_toolchain_mingw64(conf)

        # on success, alias mingw64 as "native, gcc, win, mingw"
        conf.alias_toolchain('mingw64', 'native gcc win mingw')

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

        conf.env['DEST_OS'] = 'win32'
        conf.gcc_modifier_platform()
        conf.env.append_value('LINKFLAGS', ['-Wl,-gc-sections'])
    else:
        conf.fatal('MinGW toolchain not available none windows os')

def configure_toolchain_mingw64(conf):
    if sys.platform in ("win64"):
        conf.setup_gnu_gcc_toolchain(prefix = 'mingw64')

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
    bld.gcc_host_optimize_flags()

    f = bld.env.append_value

    f('STLIB',   ['m', 'c', 'gcc'])
    f('TOOL_OPTIONS', ["linkerscript_optional"])

@conf
def setup_device_mingw(bld):
    bld.gcc_flags()
    bld.gcc_host_optimize_flags()

    f = bld.env.append_value

    f('TOOL_OPTIONS', ["linkerscript_optional"])

@conf
def gcc_host_optimize_flags(conf):
    f = conf.env.append_value

    if conf.env.CONDITIONS == 'debug':
        f('CFLAGS__optimize',   ['-O0'])
        f('CXXFLAGS__optimize', ['-O0'])
    else:
        f('CFLAGS__optimize',   ['-O3'])
        f('CXXFLAGS__optimize', ['-O3'])
        f('DEFINES__optimize',  ['NDEBUG=1'])
