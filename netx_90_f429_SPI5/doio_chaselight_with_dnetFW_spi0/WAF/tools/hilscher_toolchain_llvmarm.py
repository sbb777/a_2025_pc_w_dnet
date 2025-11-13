# vim:fileencoding=utf-8:sw=4:tw=4:et
########################################################################################
# Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.
########################################################################################
# $Id: hilscher_toolchain_llvmarm.py 848 2023-10-02 07:12:39Z AMesser $:
#
# Generic gcc arm toolchain
########################################################################################
from waflib.Configure import conf

def configure(conf):
    conf.load('hilscher_toolchain_llvm')
    env = conf.declare_toolchain('llvm-arm')
    env['TOOLCHAIN_TRIPLE'] = 'arm-none-eabi'

    path = None

    with conf.ToolchainDetectContext():
        conf.setup_llvm_toolchain(target='arm-none-eabi', path_list=path)

        conf.gcc_arm_flags()
        conf.gcc_optimize_flags()

        f = conf.env.append_value

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
