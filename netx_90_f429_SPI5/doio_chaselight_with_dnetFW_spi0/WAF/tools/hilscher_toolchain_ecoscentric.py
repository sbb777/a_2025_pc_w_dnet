# encoding: utf-8
########################################################################################
# Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.
########################################################################################
# $Id: hilscher_toolchain_ecoscentric.py 796 2023-03-16 13:29:55Z AMesser $:
#
# Ecoscentrics toolchain support
########################################################################################
from waflib.Configure import conf
import os
import re

match_ecoscentric_version = re.compile(r'.* release (?P<version>[0-9a-z]+\.[0-9a-z]+\.[0-9a-z]+)\W').match

def configure(conf):
    env = conf.declare_toolchain('ecoscentric')
    env['TOOLCHAIN_TRIPLE'] = '%s-ecoscentric-ecospro-eabi'

    # get path to toolchain
    path = os.environ.get('PATH_GCC_ARM_ECOSCENTRIC', None)

    with conf.ToolchainDetectContext():
        if path:
            conf.setup_gnu_gcc_toolchain(prefix = 'arm-eabi-', path_list = [ path + os.sep + 'bin'])
            conf.gcc_arm_flags()
            conf.gcc_optimize_flags()

            f = conf.env.append_value

            f('LINKFLAGS', ['-Wl,-n', # Disable alignment by default
                            '-Wl,-gc-sections'
            ])

            #f('CFLAGS_enable_gc_sections',    ['-ffunction-sections', '-fdata-sections'])
            #f('CXXFLAGS_enable_gc_sections',  ['-ffunction-sections', '-fdata-sections'])

            f('CFLAGS_disable_unaligned_access',   ['-mno-unaligned-access'])
            f('CXXFLAGS_disable_unaligned_access', ['-mno-unaligned-access'])

            try:
                readme_node = conf.root.find_resource(path + os.sep + 'README.txt')
                content = readme_node.read()

                first_line = content.splitlines()[0]
                m = match_ecoscentric_version(first_line)
                conf.env['CC_VERSION'] = m.group('version').split('.')
            except Exception as e:
                conf.fatal(u'Unable to determine eCosCentric toolchain version: %s' % (str(e)))
        else:
            conf.fatal('PATH_GCC_ARM_ECOSCENTRIC environment variable must be set to enable eCosCentric toolchain')


