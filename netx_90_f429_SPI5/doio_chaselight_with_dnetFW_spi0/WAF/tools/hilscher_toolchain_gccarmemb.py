# encoding: utf-8
########################################################################################
# Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.
########################################################################################
# $Id: hilscher_toolchain_gccarmemb.py 796 2023-03-16 13:29:55Z AMesser $:
#
# ARM Embed GCC toolchain supports
########################################################################################
from waflib.Configure import conf
import os

def configure(conf):
    env = conf.declare_toolchain('gccarmemb')
    env['TOOLCHAIN_TRIPLE'] = '%s-none-eabi'

    # get path to toolchain
    path = None

    if 'GCC_ARM_PATH' in os.environ:
        path = [os.path.join(os.environ['GCC_ARM_PATH'],'bin')]

    with conf.ToolchainDetectContext():
        # as long as it is not defined how the gcc compiler V4.9 provided by arm fits into
        # the hilscher compiler versioning system building using codesourcery gcc V4.5.2
        # the arm compiler must be explicitly enabled by means of environment variable GCC_ARM_PATH.
        # If this variable is not set, the toolchain will not be available
        # Further its not clear, how waf shall distinguish codesourcery / arm compiler if they
        # are found via path (Actually there is no need to do so, but as for now the compiler version
        # in the target names breaks everything then.)
        if path:
            conf.setup_gnu_gcc_toolchain(prefix = 'arm-none-eabi-', path_list = path)
            conf.gcc_arm_flags()
            conf.gcc_optimize_flags()

            f = conf.env.append_value

            f('LINKFLAGS', ['-Wl,-gc-sections'])

            f('CFLAGS_enable_gc_sections',    ['-ffunction-sections', '-fdata-sections'])
            f('CXXFLAGS_enable_gc_sections',  ['-ffunction-sections', '-fdata-sections'])

            f('CFLAGS_disable_unaligned_access',   ['-mno-unaligned-access'])
            f('CXXFLAGS_disable_unaligned_access', ['-mno-unaligned-access'])
        else:
            conf.fatal('GCC_ARM_PATH environment variable must be set to enable ARM V4.9 toolchain')
