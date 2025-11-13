# encoding: utf-8
########################################################################################
# Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.
########################################################################################
# $Id: hilscher_toolchain_gccarm.py 796 2023-03-16 13:29:55Z AMesser $:
#
# Generic gcc arm toolchain
########################################################################################
from waflib.Configure import conf

def configure(conf):
    env = conf.declare_toolchain('arm-eabi')
    env['TOOLCHAIN_TRIPLE'] = 'arm-none-eabi'

    with conf.ToolchainDetectContext():
        exc = None
        for x in 'arm-none-eabi- arm-eabi-'.split():
            try:
                conf.setup_gnu_gcc_toolchain(prefix = x)
                break
            except conf.errors.ConfigurationError as e:
                exc = exc or e
        else:
            raise exc

        conf.gcc_arm_flags()
        conf.gcc_optimize_flags()

        f = conf.env.append_value

        f('CFLAGS',    ['-ffunction-sections', '-fdata-sections'])
        f('CXXFLAGS',  ['-ffunction-sections', '-fdata-sections'])
        f('LINKFLAGS', ['-Wl,-gc-sections'])
