# encoding: utf-8
########################################################################################
# Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.
########################################################################################
# $Id: hilscher_waf_console.py 1108 2025-01-30 11:05:03Z AMesser $:
#
# Adjust the waf printouts to console
########################################################################################
import waflib.Context
import waflib.Task

import traceback
from string import Template


def options(opt):
    newstyle = waflib.Context.HEXVERSION >= waflib.Utils.num2ver('2.0.0')

    waflib.Task.Task.keyword = keyword

    if newstyle:
        waflib.Task.Task.__str__ = task_str
    else:
        waflib.Task.Task.__str__ = lambda self : '%s %s\n' % (self.keyword(), task_str(self))

def configure(conf):
    ''' Dummy function to make import happy '''
    pass

task_log_defaults = {
    'c'               : '[CC]        $SOURCES',
    'cxx'             : '[C++]       $SOURCES',
    'asm'             : '[ASM]       $SOURCES',
    'cstlib'          : '[LIB]       $TARGETS',
    'cprogram'        : '[ELF]       $OUTPUT',
    'cxxprogram'      : '[ELF]       $OUTPUT',
    'patch'           : '[PATCH]     $CWD',
    'insight_autogen' : '[AUTOG]     $SOURCES',
    'NxoBuilderTask'  : '[NXO]       $TARGETS',
    'nxupdate'        : '[NXF]       $TARGETS',
    'rst2html'        : '[RST2HTML]  $TARGETS',
    'svnexport'       : '[SVNEXPORT] $TARGETS',
    'netx_bootimage'  : '[BOOTIMAGE] $TARGETS',
    'dumpenv'         : '[DUMPENV]   $TARGET',
}

def get_log_string(task):
    name = task.__name__.replace('_task', "")

    logstring = "[%s] $SOURCES $TARGETS" % name.upper()
    logstring = task_log_defaults.get(name, logstring)
    logstring = getattr(task, "log_str", logstring)

    return logstring

def keyword(self):
    ''' Implements/overrides the keyword function '''
    w = get_log_string(type(self)).split(None,1)[0].lstrip('[').rstrip(']')

    return '[%s]' % w

def task_str(self):
    global task_log_defaults

    try:
        self.log_tmpl
    except AttributeError:
        for x in waflib.Task.classes.values():
            logstring = get_log_string(x)
            fields = logstring.split(None,1)
            x.log_tmpl = Template(fields[1])

    env = self.env

    kw = dict()
    kw['SOURCES'] = ' '.join([x.nice_path(env) for x in self.inputs])
    kw['TARGETS'] = ' '.join([x.nice_path(env) for x in self.outputs])

    if self.outputs:
        kw['OUTPUT'] = self.outputs[0].nice_path(env)

    if self.inputs:
        kw['INPUT'] = self.inputs[0].nice_path(env)

    # Some task generators don't have a 'target' attribute
    kw['TARGET']  = getattr(self.generator, "target", "<Unknown>")

    bld = self.generator.bld
    cwdnode = bld.root.find_node(getattr(self, 'cwd', getattr(bld, "cwd", bld.variant_dir)))

    kw['CWD'] = cwdnode.nice_path(env)
    kw['CLASS'] = self.__class__.__name__.replace('_task', '')

    # backwards compat for old tasks, need to adjust old tasks to get rid of this at some timepoint
    kw['name'] = getattr(self,'name',None)
    kw['out']  = getattr(self,'out', None)

    # Handle exception in logging gracefully. Otherwise way may become stuck due to implementation
    # of function run() in waflib.Runner.Spawner class
    try:
        log = self.log_tmpl.substitute(kw)
    except KeyError as e:
        traceback.print_exc()
        log = str(e)

    line_length = len(self.keyword()) + len(log)

    if line_length > 80:
        log = "..." + log[line_length - 77:]

    return log

