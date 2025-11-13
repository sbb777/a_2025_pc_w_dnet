#! /usr/bin/env python
# encoding: utf-8
########################################################################################
# Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.
########################################################################################
# $Id: python.py 1038 2024-11-05 13:59:25Z AMesser $:
#
# hilscher waf python support
########################################################################################
from waflib.Configure import conf
from waflib import Utils
import waflib.Context
import os

def options(opt):
    pass

@conf
def get_python_version(conf, python_exe):
    try:
        # Ensure PYTHONPATH variable is not set to avoid obstacles caused by wrong python paths
        environ = dict(os.environ)
        environ.pop('PYTHONPATH', None)

        (_,err) = conf.cmd_and_log(python_exe + ["-E", "-c", r"import sys; import os; sys.stderr.write('\n'.join([sys.exec_prefix,sys.executable, str(sys.version_info.major)]));"], env = environ, output=waflib.Context.BOTH)
    except conf.errors.WafError as e:
        conf.fatal('%s: %s' % (e, e.stderr))
    else:
        (python_base, python_executable, python_version) = err.splitlines()

    return python_executable, int(python_version,0)


@conf
def find_python(conf, filenames, var, major_version = None):
    # Use build tools if available
    if os.environ['PATH_BUILDTOOLS']:
        root, dirs, files = next(os.walk(os.path.join(os.environ['PATH_BUILDTOOLS'], 'python')))
        path_list = [root + os.sep + d for d in sorted(dirs, key = lambda d : tuple(map(int,d.split('.')[0:3])))]

        for p in path_list:
            conf.env.stash()
            EXE = conf.cmd_to_list(conf.find_program('python', path_list = [p], var = var))

            python_executable, python_version = conf.get_python_version(EXE)

            if python_version == (major_version or python_version):
                return python_executable

            conf.env.revert()
        else:
            conf.fatal('Failed to locate python version %s in build tools %s' % (major_version or 'any', path_list))
    else:
        for exe in Utils.to_list(filenames):
            conf.env.stash()
            try:
                EXE = conf.cmd_to_list(conf.find_program(exe, var = var))
            except conf.errors.ConfigurationError as e:
                pass # program not found
            else:
                python_executable, python_version = conf.get_python_version(EXE)

                if python_version == (major_version or python_version):
                    return python_executable
            conf.env.revert()
        else:
            conf.fatal('Failed to locate python executable %s version %s in %s' % (filenames, major_version or 'any', path_list) )


def configure(conf):
    # change to parent environment object
    conf.setenv('')

    python_exe = conf.find_python('python', var='PYTHON')

    conf.env['PYTHON']            = python_exe
    conf.env['PYTHON_PYTHONPATH'] = []

    conf.msg('Using program python', conf.env['PYTHON'])

@conf
def run_python(ctx, cmd, pythonpath = None, env = None, quiet=waflib.Context.STDERR, **kw):
    cmd = [ctx.env['PYTHON']] + Utils.to_list(cmd)
    pythonpath = Utils.to_list(pythonpath) + ctx.env['PYTHON_PYTHONPATH']

    env = dict(env or os.environ)
    env['PYTHONPATH'] = os.pathsep.join(pythonpath)

    return ctx.cmd_and_log(cmd, output=waflib.Context.BOTH, quiet=quiet, env = env, **kw)
