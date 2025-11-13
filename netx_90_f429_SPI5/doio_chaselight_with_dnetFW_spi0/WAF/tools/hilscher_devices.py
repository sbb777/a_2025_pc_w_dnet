# encoding: utf-8
########################################################################################
# Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.
########################################################################################
# $Id: hilscher_devices.py 801 2023-03-17 10:03:31Z AMesser $:
#
# Waf module to declare common devices used at Hilscher
########################################################################################
from waflib.Configure import conf
from waflib.TaskGen import feature

def options(opt):
    opt.load('hilscher_toolchain')

def configure(conf):
    conf.load('hilscher_toolchain')

    conf.declare_platform('netx',     'hitex codesourcery arm-eabi gccarmemb', setup_device_netx,    target_triple_legacy)
    conf.declare_platform('netx50',   'hitex codesourcery arm-eabi gccarmemb', setup_device_netx50,  target_triple_legacy)
    conf.declare_platform('netx100',  'hitex codesourcery arm-eabi gccarmemb', setup_device_netx100, target_triple_legacy)
    conf.declare_platform('netx500',  'hitex codesourcery arm-eabi gccarmemb', setup_device_netx100, target_triple_legacy)

    conf.declare_platform('netx10',   'codesourcery arm-eabi gccarmemb', setup_device_netx10, target_triple_legacy)
    conf.declare_platform('netx51',   'codesourcery arm-eabi gccarmemb', setup_device_netx10, target_triple_legacy)
    conf.declare_platform('netx52',   'codesourcery arm-eabi gccarmemb', setup_device_netx10, target_triple_legacy)

    conf.declare_platform('netx90',   'gccarmemb ecoscentric', setup_device_netx90,   target_triple_armv7em)
    conf.declare_platform('netx4000', 'gccarmemb ecoscentric', setup_device_netx4000, target_triple_armv7r)

    conf.declare_platform('netx90_app',         'gccarmemb', setup_device_netx90_app, target_triple_armv7em)
    conf.declare_platform('netx90_app_softfp',  'gccarmemb', setup_device_netx90_app, target_triple_armv7em)
    conf.declare_platform('netx90_app_hardfp',  'gccarmemb', setup_device_netx90_app, target_triple_armv7em)

    conf.declare_platform('armv8a',     'ecoscentric', setup_platform_armv8a, target_triple_armv8a)
    conf.declare_platform('netxxxlmpw', 'ecoscentric', setup_platform_armv8a, target_triple_armv8a)

@conf
def target_triple_legacy(bld, platform):
    return 'arm-' + bld.env['TOOLCHAIN_TRIPLE'].partition('-')[2]

@conf
def setup_device_netx(bld):
    bld.gcc_netx_flags()

    for x in 'CFLAGS CXXFLAGS ASFLAGS LINKFLAGS'.split():
        bld.env.append_value(x, ['-march=armv5te'])
        if bld.env.TOOLCHAIN != 'hitex':
            # hitex does not fully support this
            bld.env.append_value(x, ['-msoft-float', '-mfpu=vfp', '-mfloat-abi=soft'])

@conf
def setup_device_netx50(bld):
    bld.gcc_netx_flags()

    if bld.cc_version()[0:2] < (4,1):
        mcpu = '-mcpu=arm9e'
    else:
        mcpu = '-mcpu=arm966e-s'

    for x in 'CFLAGS CXXFLAGS ASFLAGS'.split():
        bld.env.append_value(x, mcpu)

    if bld.env.TOOLCHAIN == 'hitex':
        # Workaround for hitex linking
        # hitex compiler chooses hardware fp library whenever something
        # else than arm926ej-s is specified
        bld.env.append_value('LINKFLAGS', '-mcpu=arm926ej-s')
    else:
        bld.env.append_value('LINKFLAGS', mcpu)

    for x in 'CFLAGS CXXFLAGS ASFLAGS LINKFLAGS'.split():
        if bld.env.TOOLCHAIN != 'hitex':
            # hitex does not fully support this
            bld.env.append_value(x, ['-msoft-float', '-mfpu=vfp', '-mfloat-abi=soft'])

@conf
def setup_device_netx10(bld):
    bld.gcc_netx_flags()

    if bld.cc_version()[0:2] < (4,1):
        mcpu = '-mcpu=arm9e'
    else:
        mcpu = '-mcpu=arm966e-s'

    for x in 'CFLAGS CXXFLAGS ASFLAGS'.split():
        bld.env.append_value(x, mcpu)

    if bld.env.TOOLCHAIN == 'hitex':
        # Workaround for hitex linking
        # hitex compiler chooses hardware fp library whenever something
        # else than arm926ej-s is specified
        bld.env.append_value('LINKFLAGS', '-mcpu=arm926ej-s')
    else:
        bld.env.append_value('LINKFLAGS', mcpu)

    for x in 'CFLAGS CXXFLAGS ASFLAGS LINKFLAGS'.split():
        bld.env.append_value(x, ['-msoft-float', '-mfpu=vfp', '-mfloat-abi=soft'])

@conf
def setup_device_netx100(bld):
    bld.gcc_netx_flags()

    for x in 'CFLAGS CXXFLAGS ASFLAGS LINKFLAGS'.split():
        bld.env.append_value(x, '-mcpu=arm926ej-s')
        if bld.env.TOOLCHAIN != 'hitex':
            # hitex does not fully support this
            bld.env.append_value(x, ['-msoft-float', '-mfpu=vfp', '-mfloat-abi=soft'])

@conf
def target_triple_armv7r(bld, platform):
    return 'armv7r-' + bld.env['TOOLCHAIN_TRIPLE'].partition('-')[2]

@conf
def setup_device_netx4000(bld):
    bld.gcc_flags()

    f = bld.env.append_value

    for x in 'CFLAGS CXXFLAGS'.split():
        f(x,[ '-mlong-calls',
              '-mapcs',
              '-fno-common',])

    f('ASFLAGS',[ '-mapcs',
                  '-c'])

    f('LINKFLAGS', ['-mthumb-interwork', '-nostdlib'])

    f('DEFINES', ['_NETX_'])
    f('STLIB_nxo_standardlibs',       ['gcc'])
    f('STLIB_default_standardlibs',   ['gcc'])

    for x in 'CFLAGS CXXFLAGS ASFLAGS LINKFLAGS'.split():
        # Floating point operations will invoke library functions
        # no hardware floating instructions will be enabled
        # see WAF-132 for details
        f(x, ['-march=armv7-r'])

        # software floating point
        f(x, ['-mfloat-abi=soft'])

        # hardware floating point with soft floating point api
        #f(x, ['-mfpu=vfpv3-d16', '-mfloat-abi=softfp'])

        # support hardware floating point with floating point calling convention
        # (fpu registers will be used to pass arguments)
        # WARNING: this produces incompatible code with above
        # option. Ether choose the softfp or this, not both
        #f(x, ['-mfpu=vfpv3-d16', '-mfloat-abi=hard'])

@conf
def setup_device_armv7em(bld):
    bld.gcc_flags()

    # Cortex-M4 onyl implements Thumb-2 instruction encoding but
    # no ARM instruction enconding
    bld.env['CFLAGS_compile_arm']      = []
    bld.env['CXXFLAGS_compile_arm']    = []
    bld.env['LINKFLAGS_compile_arm']   = []
    bld.env['CFLAGS_compile_thumb']    = []
    bld.env['CXXFLAGS_compile_thumb']  = []
    bld.env['LINKFLAGS_compile_thumb'] = []

    f = bld.env.append_value

    for x in 'CFLAGS CXXFLAGS'.split():
        f(x,[ '-mlong-calls',
              '-mapcs',
              '-fno-common',
              '-mthumb'])

    f('ASFLAGS',[ '-mapcs',
                  '-c',
                  '-mthumb'])

    # -mthumb is needed for linking to select correct libraries
    # see gcc -print-multi-lib output
    f('LINKFLAGS', [ '-nostdlib', '-mthumb'])

    f('DEFINES', ['_NETX_'])
    f('STLIB_nxo_standardlibs',       ['gcc'])
    f('STLIB_default_standardlibs',   ['gcc'])

    for x in 'CFLAGS CXXFLAGS ASFLAGS LINKFLAGS'.split():
        # Floating point operations will invoke library functions
        # no hardware floating instructions will be enabled
        # see WAF-132 for details
        f(x, ['-march=armv7e-m'])


@conf
def target_triple_armv7em(bld, platform):
    return 'armv7em-' + bld.env['TOOLCHAIN_TRIPLE'].partition('-')[2]

@conf
def setup_device_netx90(bld):
    bld.setup_device_armv7em()

    f = bld.env.append_value
    for x in 'CFLAGS CXXFLAGS ASFLAGS LINKFLAGS'.split():
        # software floating point
        f(x, ['-mfloat-abi=soft'])


@conf
def setup_device_netx90_app(bld):
    bld.setup_device_armv7em()

    float_flags = {
        'netx90_app'        : ['-mfloat-abi=soft'],
        'netx90_app_softfp' : ['-mfpu=fpv4-sp-d16', '-mfloat-abi=softfp'],
        'netx90_app_hardfp' : ['-mfpu=fpv4-sp-d16', '-mfloat-abi=hard'],
    }[bld.env['PLATFORM']]

    f = bld.env.append_value
    for x in 'CFLAGS CXXFLAGS ASFLAGS LINKFLAGS'.split():
        f(x, float_flags)

@conf
def target_triple_armv8a(bld, platform):
    return 'armv8a-' + bld.env['TOOLCHAIN_TRIPLE'].partition('-')[2]

@conf
def setup_platform_armv8a(bld):
    bld.gcc_flags()

    f = bld.env.append_value

    for x in 'CFLAGS CXXFLAGS'.split():
        f(x,[ '-mlong-calls',
              '-mapcs',
              '-fno-common',])

    f('ASFLAGS',[ '-mapcs',
                  '-c'])

    f('LINKFLAGS', ['-mthumb-interwork', '-nostdlib'])

    f('DEFINES', ['_NETX_'])
    f('STLIB_nxo_standardlibs',       ['gcc'])
    f('STLIB_default_standardlibs',   ['gcc'])

    for x in 'CFLAGS CXXFLAGS ASFLAGS LINKFLAGS'.split():
        # Floating point operations will invoke library functions
        # no hardware floating instructions will be enabled
        # see WAF-132 for details
        f(x, ['-march=armv8-a'])

        # software floating point
        f(x, ['-mfloat-abi=soft'])

        # hardware floating point with soft floating point api
        #f(x, ['-mfpu=vfpv3-d16', '-mfloat-abi=softfp'])

        # support hardware floating point with floating point calling convention
        # (fpu registers will be used to pass arguments)
        # WARNING: this produces incompatible code with above
        # option. Ether choose the softfp or this, not both
        #f(x, ['-mfpu=vfpv3-d16', '-mfloat-abi=hard'])
