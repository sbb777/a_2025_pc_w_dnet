# encoding: utf-8
########################################################################################
# Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.
########################################################################################
# $Id: hilscher_toolchain_hitex.py 796 2023-03-16 13:29:55Z AMesser $:
#
# Hitex toolchain support
########################################################################################
from waflib.Configure import conf
import os

def configure(conf):
    env = conf.declare_toolchain('hitex')
    env['TOOLCHAIN_TRIPLE'] = 'arm-hitex-elf'

    path = None

    if 'PATH_GNU_ARM' in os.environ:
        path = [os.path.join(os.environ['PATH_GNU_ARM'], 'bin')]

    with conf.ToolchainDetectContext():
        conf.setup_gnu_gcc_toolchain(prefix = 'arm-hitex-elf-', path_list = path)
        conf.gcc_arm_flags()
        conf.gcc_optimize_flags()
