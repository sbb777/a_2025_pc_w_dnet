# vim:fileencoding=utf-8:sw=4:tw=4:et
########################################################################################
# Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.
########################################################################################
# $Id: hilscher_toolchain.py 874 2023-10-19 10:14:01Z AMesser $:
#
# Waf module to handle toolchains
########################################################################################
from waflib import Build, Options, Utils
from waflib.Configure import conf
from waflib.TaskGen import feature, before_method

def options(opt):
    opt.load('asm gcc gxx hilscher_ccroot')

    opt.add_option('--custom-toolchain', action='store_true', dest='use_custom_toolchain', default=False,
                    help=u'Use a custom toolchain for all builds')

    opt.add_option('--compiler-prefix', action='store', dest='custom_toolchain_prefix', default='arm-none-eabi',
                    help='Select a custom compiler prefix (e.g. "arm-none-eabi" [default])')

    opt.add_option('--compiler-path', action='store', dest='custom_toolchain_path', default=None,
                    help=r'Select a custom compiler path (e.g. "C:\Programme\Codesourcery\Sourcery G++ Lite\bin")')

    opt.add_option('--compiler-driver', action='store', dest='custom_toolchain_driver', default='gcc',
                    help=r'Select a custom compiler driver (e.g. "gcc" [default])')

    opt.add_option('--cxxcompiler-driver', action='store', dest='custom_toolchain_cxxdriver', default='g++',
                    help=r'Select a custom C++ compiler driver (e.g. "g++" [default])')

def configure(conf):
    conf.setenv('')

    conf.load('hilscher_ccroot')

    if not conf.env['CONDITIONS']:
        raise conf.errors.WafError(u'Must load hilscher_netx before hilscher_toolchains in %s' % conf.cur_script.abspath())

    if conf.options.use_custom_toolchain:
        configure_toolchain_custom(conf)


@feature('*')
@before_method('propagate_uselib_vars')
def apply_conditions(self):
    ''' Apply build conditions '''

    _vars = self.get_uselib_vars()
    env = self.env

    for var in _vars:
        env.append_value(var, env[var + '__optimize'])

    # For legacy wscripts, set feature to include feature vars as
    # previous. legacy scripts should be reworked at some time
    self.features = ['compile_' + self.env.CONDITIONS] + Utils.to_list(getattr(self, 'features', []))

@conf
def get_platform_funs(bld, platform):
    funs = bld.env['PLATFORM_%s_funs' % platform]

    if not funs:
        if bld.env['DISABLED_WHY']:
            bld.warn('Toolchain \'%s\' notavailable: \'%s\'.' % (bld.env['TOOLCHAIN'], bld.env['DISABLED_WHY']))
            return None, None
        else:
            bld.wscript_error('Toolchain \'%s\' does not support platform \'%s\'.' % (bld.env['TOOLCHAIN'], platform))

    return funs

@conf
def get_target_triple(bld, platform):
    toolchain = bld.env['TOOLCHAIN']

    if bld.env['DISABLED_WHY']:
        # Quirk to properly handly historic build script where get_name_prefix was called without platform
        if toolchain == 'codesourcery':
            platform = 'netx'

        # compute a fake target triplet since we dont know the toolchain
        target_triple = ('%s-disabled-%s' % (platform or 'unkown',toolchain))
    else:
        if platform is None:
            target_triple = bld.env['TOOLCHAIN_TRIPLE']

            if '%s' in target_triple:
                bld.wscript_error('Toolchain \'%s\' requires platform argument' % toolchain)
        else:
            _, target_triplet_fun_name = bld.get_platform_funs(platform)

            if target_triplet_fun_name:
                target_triple_fun = getattr(bld, target_triplet_fun_name)
                target_triple = target_triple_fun(platform=platform)
            else:
                target_triple = None

        if not target_triple:
            bld.wscript_error('Can not get target triplet for toolchain \'%s\' platform \'%s\'.' % (toolchain, platform))

    return target_triple

@conf
def get_name_prefix(bld, toolchain = None, suffix = None, platform = None):
    bld.select_toolchain_env(toolchain)

    target_triple = bld.get_target_triple(platform)

    version = '.'.join(bld.env['CC_VERSION']) or 'x.x.x'

    return "/".join(x for x in (target_triple, version, suffix, '') if x is not None)

@conf
def select_toolchain_env(ctx, name):
    envname = 'toolchain_%s' % name

    if envname in ctx.all_envs:
        ctx.setenv(envname)
    elif (name != 'custom') and ctx.options.use_custom_toolchain:
        env = ctx.select_toolchain_env('custom')
        ctx.setenv(envname,env)
    else:
        ctx.setenv('')
        ctx.setenv(envname,ctx.env)
        ctx.env['TOOLCHAIN']    = name
        ctx.env['DISABLED_WHY'] = 'Toolchain \'%s\' not initialized' % (name)

    return ctx.env

@conf
def setup_platform_env(ctx, platform, toolchain):
    envname = 'device_%s_%s' % (platform, toolchain)

    if envname in ctx.all_envs:
        ctx.setenv(envname)
    else:
        ctx.select_toolchain_env(toolchain)
        ctx.setenv(envname,ctx.env)

        ctx.env['PLATFORM'] = platform

        if platform.startswith('netx'):
            ctx.env.append_value('DEFINES', '__' + platform.upper())

        if not ctx.env['DISABLED_WHY']:
            setup_fn_name, _ = ctx.get_platform_funs(platform)

            if setup_fn_name:
                getattr(ctx, setup_fn_name)()
            else:
                ctx.env['DISABLED_WHY'] = 'Platform \'%s\' not available in toolchain \'%s\'' % (platform, toolchain)

    return ctx.env

@conf
def declare_toolchain(conf, name):
    return conf.select_toolchain_env(name)

@conf
def get_toolchain_funs(ctx):
    funs = ctx.env['TOOLCHAIN_funs']

    if not funs:
        ctx.wscript_error('Toolchain \'%s\' not available.' % (ctx.env['TOOLCHAIN']))

    return funs

@conf
class ToolchainDetectContext(object):
    def __init__(self, ctx):
        self.ctx = ctx

    def __enter__(self):
        if self.ctx.options.use_custom_toolchain and self.ctx.envname != 'toolchain_custom':
            raise self.ctx.errors.ConfigurationError('Custom toolchain requested')

        self.ctx.env.stash()

    def __exit__(self, type, value, traceback):
        ctx = self.ctx

        if value:
            ctx.env.revert()

            ctx.env['DISABLED_WHY'] = 'Toolchain \'%s\' not available: \'%s\'' % (ctx.env.TOOLCHAIN, str(value))

            ctx.start_msg("Toolchain '%s'" % ctx.env.TOOLCHAIN)
            ctx.end_msg("not available (Some projects may not be available for building)", 'YELLOW')

            return True
        else:
            del ctx.env['DISABLED_WHY']

            ctx.start_msg("Toolchain '%s'" % ctx.env.TOOLCHAIN)
            ctx.end_msg("%s %s" %(ctx.env.CC_NAME, ".".join(ctx.env.CC_VERSION)))

        return False

@conf
def declare_platform(conf, platform, toolchains, setup_env_fun, triple_fun):
    for t in Utils.to_list(toolchains):
        conf.select_toolchain_env(t)
        conf.env['PLATFORM_%s_funs' % platform] = setup_env_fun.__name__, triple_fun.__name__

def set_optimize_flags(env, conditions, **kwargs):
    if conditions != env.CONDITIONS:
        return

    for var,value in kwargs.items():
        if value is not None:
            env[var.upper() + '__optimize'] = Utils.to_list(value)

@conf
def set_toolchain_optimize_flags(conf, toolchain, conditions, cflags = None, cxxflags = None, defines = None, linkflags = None):
    u''' Set/override optimization flags for particular condition'''

    if conf.cmd != 'configure':
        conf.fatal(u'Function set_toolchain_optimize_flags() only allowed in configure()')

    env = conf.select_toolchain_env(toolchain)
    set_optimize_flags(env,conditions, cflags = cflags, cxxflags = cxxflags, defines=defines, linkflags=linkflags)


@conf
def set_target_optimize_flags(bld, name, conditions, cflags = None, cxxflags = None, defines = None, linkflags = None):
    u''' Set/override optimization flags for particular condition'''

    if not isinstance(bld,Build.BuildContext):
        bld.fatal(u'Function set_target_optimize_flags() only allowed in build()')

    try:
        env = bld.all_envs['target_' + name]
    except KeyError:
        raise bld.errors.WafError(u'Environment for target \'%s\' not found. (Target not yet declared?)' % name)

    set_optimize_flags(env, conditions, cflags = cflags, cxxflags = cxxflags, defines=defines, linkflags=linkflags)


###############################################################################
###############################################################################
# Definition of setup functions for common compiler types
###############################################################################
###############################################################################
@conf
def setup_gnu_binutils(conf, path_list = None):
    u'''Setup GNU binutils variables for current environment'''
    env = conf.env

    prefix = env['GNU_BINUTILS_PREFIX'] or ''

    for tool in 'ar readelf objdump objcopy strip nm'.split():
        executable_name = prefix + tool
        conf.find_program(executable_name, var=tool.upper(), path_list=path_list)

    env.ARFLAGS = 'rcs'

@conf
def gcc_optimize_flags(conf):
    f = conf.env.append_value

    if conf.env.CONDITIONS == 'debug':
        f('CFLAGS__optimize',   ['-O0'])
        f('CXXFLAGS__optimize', ['-O0'])
    else:
        f('CFLAGS__optimize',   ['-Os'])
        f('CXXFLAGS__optimize', ['-Os'])
        f('DEFINES__optimize',  ['NDEBUG=1'])

@conf
def setup_gnu_gcc_toolchain(conf, prefix, compiler = None, cxxcompiler = None, path_list=None):
    env = conf.env

    env.stash()
    try:
        env['GNU_GCC_PREFIX']      = prefix
        env['GNU_BINUTILS_PREFIX'] = prefix

        conf.setup_gnu_binutils(path_list)

        compiler    = compiler or (env['GNU_GCC_PREFIX'] + 'gcc')
        cxxcompiler = cxxcompiler or (env['GNU_GCC_PREFIX'] + 'g++')

        cc = conf.find_program(compiler, var='CC', path_list=path_list)
        cc = conf.cmd_to_list(cc)

        cxx = conf.find_program(cxxcompiler, var='CXX', mandatory=False, path_list=path_list)
        cxx = conf.cmd_to_list(cxx)

        conf.env.CC  = cc
        conf.env.AS  = cc

        env['CC_NAME']    = 'gcc'
        env['CC_VERSION'] = conf.cmd_and_log(env.CC + ["-dumpversion"]).strip().split('.')
        env['MACHINE']    = env['MACHINE'] or conf.cmd_and_log(env.CC +["-dumpmachine"]).strip()
        env['DEST_OS']    = 'Unknown'

        # backwards compat. Since we support clang now as well,
        # there is no common CC_PREFIX anymore. (CLANG is not prefixed)
        # please use 'MACHINE' for future developments
        env['CC_PREFIX']  = prefix.strip('-')

        conf.cc_load_tools()
        conf.gcc_common_flags()
        conf.cc_add_flags()

        conf.env.AS_TGT_F = '-o'

        if(cxx):
            env['CXX_NAME'] = 'gcc'
            env['CXX'] = cxx

            conf.cxx_load_tools()
            conf.gxx_common_flags()
            conf.cxx_add_flags()

        conf.link_add_flags()

        f = env.append_value

        # we have to use gnu99 because we use some non standard gnu extensions conditionally
        # when __GNUC__ is defined
        f('CFLAGS_standard_c99', [ "-std=gnu99" ])

        # Always generate debug symbols
        f('CFLAGS',   ['-g'])
        f('CXXFLAGS', ['-g'])
        f('ASFLAGS',  ['-Wa,-g'])

        gcov = env['GNU_GCC_PREFIX'] + 'gcov'
        gcov = conf.find_program(gcov, var='GCOV', mandatory=False, path_list=path_list)
    except conf.errors.ConfigurationError:
        env.revert()
        raise

def configure_toolchain_custom(conf):
    ''' Used when overriding all toolchains with the one provided at command line'''
    opt = Options.options

    conf.setup_gnu_gcc_toolchain(
        prefix = opt.custom_toolchain_prefix + '-',
        compiler=opt.custom_toolchain_driver,
        cxxcompiler=opt.custom_toolchain_cxxdriver,
        path_list=[opt.custom_toolchain_path]
    )

    conf.gcc_arm_flags()
    conf.gcc_optimize_flags()

    f = conf.env.append_value

    f('CFLAGS',    ['-ffunction-sections', '-fdata-sections'])
    f('CXXFLAGS',  ['-ffunction-sections', '-fdata-sections'])
    f('LINKFLAGS', ['-Wl,-gc-sections'])

@conf
def cc_version(conf):
    a,b,c = conf.env['CC_VERSION'] or ('0', '0', '0')

    c = c.rstrip('abcdefghijklmnopqrstuvwxyz')
    return (int(a), int(b), int(c))

@conf
def gcc_flags(conf):
    f = conf.env.append_value

    for x in 'CFLAGS CXXFLAGS'.split():
        f(x,[ '-Wall',
              '-Wredundant-decls',
              '-Wno-inline',
              '-Winit-self'])

    f('ASFLAGS',[ '-Wall',
                  '-Wredundant-decls',
                  '-Wno-inline',
                  '-c'])

    for x in 'CFLAGS CXXFLAGS'.split():
        f(x + '_check_ansi', ['-ansi',
                              '-pedantic'])

    for x in 'CFLAGS CXXFLAGS'.split():
        f(x + '_check_extra', ['-Wconversion',
                               '-Wmissing-field-initializers',
                               '-Wsign-compare',
                               '-Wpointer-arith',
                               '-Wcast-qual'])

    if conf.cc_version() >= (4,1,2):
        f('CFLAGS_check_extra', ['-Wc++-compat'])

    # define CFLAGS_warninglevel1 and CXXFLAGS_warninglevel2
    for x in 'CFLAGS CXXFLAGS'.split():
        f(x + '_warninglevel1', [ "-Wsystem-headers",
                                  "-Wbad-function-cast",
                                  "-Wsign-compare",
                                  "-Waggregate-return",
                                  "-Wswitch-default",
                                  "-Wstrict-prototypes",
                                  '-Wpointer-arith',
                                  ])

@feature("check_extra", "check_ansi", "warninglevel1", "standard_c99")
def compile_check_dummy(self):
    # make waf happy
    pass

@conf
def gcc_arm_flags(conf):
    f = conf.env.append_value

    f('CFLAGS_compile_arm',   ['-marm'])
    f('CXXFLAGS_compile_arm',   ['-marm'])
    f('LINKFLAGS_compile_arm',   ['-marm'])
    f('CFLAGS_compile_thumb', ['-mthumb'])
    f('CXXFLAGS_compile_thumb', ['-mthumb'])
    f('LINKFLAGS_compile_thumb', ['-mthumb'])

    conf.env['cprogram_PATTERN']   = '%s.elf'
    conf.env['cxxprogram_PATTERN'] = '%s.elf'

    # we prefer gdwarf-2 output for use with lauterbach t32
    f('CFLAGS',   ['-gdwarf-2'])
    f('CXXFLAGS', ['-gdwarf-2'])
    f('ASFLAGS',  ['-Wa,-gdwarf2'])

@conf
def gcc_netx_flags(conf):
    conf.gcc_flags()

    f = conf.env.append_value

    for x in 'CFLAGS CXXFLAGS'.split():
        f(x,[ '-mlong-calls',
              '-mapcs',
              '-mthumb-interwork',
              '-fshort-enums',
              '-fno-common'])

    f('ASFLAGS',[ '-mapcs',
                  '-mthumb-interwork',
                  '-fshort-enums',
                  '-c'])

    f('LINKFLAGS', [ '-mthumb-interwork', '-nostdlib'])

    f('DEFINES', ['_NETX_'])
    f('STLIB_nxo_standardlibs', ['m', 'gcc'])
    f('STLIB_default_standardlibs',   ['m', 'c', 'gcc'])
