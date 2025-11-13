#! /usr/bin/env python
# encoding: utf-8
########################################################################################
# Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.
########################################################################################
# $Id: javaw.py 980 2024-09-09 12:00:56Z AMesser $:
#
# hilscher waf python3 support
########################################################################################
import waflib.Context
import os

def options(opt):
    pass

def configure(conf):
    # change to parent environment object
    conf.setenv('')

    if 'JAVA_HOME' in os.environ:
        conf.env['JAVA_HOME'] = os.environ['JAVA_HOME']
        JAVA = conf.cmd_to_list(conf.find_program('java', var = 'JAVA', path_list=[conf.env['JAVA_HOME'] + os.sep + 'bin']))
    else:
        JAVA = conf.cmd_to_list(conf.find_program('java', var = 'JAVA'))

        if len(JAVA) != 1:
            conf.fatal('Can not determine java home path')

        conf.env['JAVA_HOME'] = os.path.normpath(os.path.join(JAVA[0], '..', '..'))

    (out,err) = conf.cmd_and_log(JAVA + ["--version"], output=waflib.Context.BOTH)

