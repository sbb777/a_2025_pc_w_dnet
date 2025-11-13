# vim:fileencoding=utf-8:sw=4:tw=4:et
########################################################################################
# Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.
########################################################################################
# $Id: hilscher_toolchain_llvm.py 848 2023-10-02 07:12:39Z AMesser $:
#
# Provides common helpers for use with llvm toolchains
########################################################################################
from waflib.Configure import conf

@conf
def setup_llvm_toolchain(conf, target, path_list=None):
    env = conf.env

    compiler = 'clang'

    env.stash()
    try:
        env['MACHINE'] = target

        if target:
            env['GNU_BINUTILS_PREFIX'] = "%s-" % target
        else:
            env['GNU_BINUTILS_PREFIX'] = ""

        conf.setup_gnu_binutils(path_list)

        config = conf.find_program('llvm-config', var='LLVM_CONFIG', path_list=path_list)
        env['LLVM_CONFIG'] = conf.cmd_to_list(config)

        cc = conf.find_program(compiler, var='CC', path_list=path_list)
        cc = conf.cmd_to_list(cc) + ['-target', target]

        env['CC_NAME']    = compiler
        env['CXX_NAME']   = compiler

        env['CC_VERSION'] = conf.cmd_and_log(env.LLVM_CONFIG + ["--version"]).strip().split('.')

        env.CC     = cc
        env['CXX'] = cc
        env.AS     = cc
        env.ARFLAGS = 'rcs'

        env['LINK_CC']  = env['CC']
        env['LINK_CXX'] = env['CC']

        conf.clang_common_flags()

        f = env.append_value
        f('INCLUDES', conf.cmd_and_log(env.LLVM_CONFIG + ["--includedir"]).splitlines())
        f('LIBPATH',  conf.cmd_and_log(env.LLVM_CONFIG + ["--libdir"]).splitlines())

        conf.env.AS_TGT_F = ['-c', '-o']
        conf.cc_add_flags()
        conf.link_add_flags()

        # we have to use gnu99 because we use some non standard gnu extensions conditionally
        # when __GNUC__ is defined
        env.append_value('CFLAGS_standard_c99', [ "-std=gnu99" ])

        # Always generate debug symbols
        f('CFLAGS',   ['-g'])
        f('CXXFLAGS', ['-g'])
        f('ASFLAGS',  ['-Wa,-g'])

        #env.stash()
        #try:
        #    conf.env.prepend_value('LINKFLAGS', ['-v'])
        #    conf.env.prepend_value('CFLAGS', ['-v'])
        #    conf.check_cc(lib='c', msg = 'Checking if clang supports target %s' % target, okmsg='Yes', errmsg='No')
        #finally:
        #    env.revert()
    except conf.errors.ConfigurationError:
        env.revert()
        raise

@conf
def clang_common_flags(conf):
    """
    Common flags for g++ on nearly all platforms
    """
    v = conf.env

    v['CC_SRC_F']           = []
    v['CC_TGT_F']           = ['-c', '-o']

    v['CXX_SRC_F']           = []
    v['CXX_TGT_F']           = ['-c', '-o']


    v['CCLNK_SRC_F']         = []
    v['CCLNK_TGT_F']         = ['-o']
    v['CXXLNK_SRC_F']        = []
    v['CXXLNK_TGT_F']        = ['-o']
    v['CPPPATH_ST']          = '-I%s'
    v['DEFINES_ST']          = '-D%s'

    v['LIB_ST']              = '-l%s' # template for adding libs
    v['LIBPATH_ST']          = '-L%s' # template for adding libpaths
    v['STLIB_ST']            = '-l%s'
    v['STLIBPATH_ST']        = '-L%s'
    v['RPATH_ST']            = '-Wl,-rpath,%s'

    v['SONAME_ST']           = '-Wl,-h,%s'
    v['SHLIB_MARKER']        = '-Wl,-Bdynamic'
    v['STLIB_MARKER']        = '-Wl,-Bstatic'

    # program
    v['cprogram_PATTERN']    = '%s'
    v['cxxprogram_PATTERN']  = '%s'

    # shared library
    v['CXXFLAGS_cshlib']   = ['-fPIC']
    v['LINKFLAGS_cshlib']  = ['-shared']
    v['cshlib_PATTERN']    = 'lib%s.so'

    v['CXXFLAGS_cxxshlib']   = ['-fPIC']
    v['LINKFLAGS_cxxshlib']  = ['-shared']
    v['cxxshlib_PATTERN']    = 'lib%s.so'

    # static lib
    v['LINKFLAGS_cstlib']  = ['-Wl,-Bstatic']
    v['cstlib_PATTERN']    = 'lib%s.a'

    v['LINKFLAGS_cxxstlib']  = ['-Wl,-Bstatic']
    v['cxxstlib_PATTERN']    = 'lib%s.a'
