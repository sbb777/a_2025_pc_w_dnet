# encoding: utf-8
########################################################################################
# Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.
########################################################################################
# $Id: hilscher_toolchain_codesourcery.py 796 2023-03-16 13:29:55Z AMesser $:
#
# Codesourcery Toolchain support
########################################################################################
from waflib.Configure import conf
import os
import sys

if sys.platform.startswith("win"):
    try:
        import _winreg as winreg
    except:
        import winreg

    def get_registry_string_value(key, value_name):
        value = None

        with  winreg.ConnectRegistry(None, winreg.HKEY_LOCAL_MACHINE) as aReg:
            try:
                with winreg.OpenKey(aReg, key) as aKey:
                    value, typ = winreg.QueryValueEx(aKey, value_name)
            except WindowsError:
                pass

            if value is None and hasattr(winreg, 'KEY_WOW64_32KEY'):
                try:
                    with winreg.OpenKey(aReg, key, 0, winreg.KEY_READ | winreg.KEY_WOW64_32KEY) as aKey:
                        value, typ = winreg.QueryValueEx(aKey, value_name)
                except WindowsError:
                    pass

            if value is not None:
                assert typ is winreg.REG_SZ

                if sys.version_info[0] == 2:
                    value = value.encode('ascii', 'ignore')

        return value
else:
    def get_registry_string_value(key, value_name):
        return None

def configure(conf):
    env = conf.declare_toolchain('codesourcery')
    env['TOOLCHAIN_TRIPLE'] = 'arm-none-eabi'

    with conf.ToolchainDetectContext():
        # get path to toolchain
        path = None

        if 'CS_PATH' in os.environ:
            path = [os.path.join(os.environ['CS_PATH'],'bin')]
        else:
            if sys.platform.startswith("win"):
                install_loc = get_registry_string_value(r"SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\Sourcery G++ Lite for ARM EABI", "InstallLocation")

            if install_loc is not None:
                path = [os.path.join(install_loc, 'bin')]

        conf.setup_gnu_gcc_toolchain(prefix = 'arm-none-eabi-', path_list=path)
        conf.gcc_arm_flags()
        conf.gcc_optimize_flags()

        f = conf.env.append_value

        f('LINKFLAGS', ['-Wl,-gc-sections'])

        f('CFLAGS_enable_gc_sections',    ['-ffunction-sections', '-fdata-sections'])
        f('CXXFLAGS_enable_gc_sections',  ['-ffunction-sections', '-fdata-sections'])



