#! /usr/bin/env python
# encoding: utf-8
########################################################################################
# Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.
########################################################################################
# $Id:  $:
#
# hilscher waf python3 support
########################################################################################
from waflib.Configure import conf
from waflib import Utils
import waflib.Context
import os

def options(opt):
    opt.load('python')

def configure(conf):
    conf.load('python')

    # change to parent environment object
    conf.setenv('')

    python_exe = conf.find_python('python3 python', var='PYTHON3', major_version=3,)


    # patch path for python
    conf.env['PYTHON3']            = python_exe
    conf.env['PYTHON3_PYTHONPATH'] = []

    conf.msg('Using program python3', conf.env['PYTHON3'])

@conf
def run_python3(ctx, cmd, pythonpath = None, env = None, quiet=waflib.Context.STDERR, **kw):
    cmd = [ctx.env['PYTHON3']] + Utils.to_list(cmd)
    pythonpath = Utils.to_list(pythonpath) + ctx.env['PYTHON3_PYTHONPATH']

    env = dict(env or os.environ)
    env['PYTHONPATH'] = os.pathsep.join(pythonpath)

    return ctx.cmd_and_log(cmd, output=waflib.Context.BOTH, quiet=quiet, env = env, **kw)
