#! /usr/bin/env python
# encoding: utf-8
########################################################################################
# Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.
########################################################################################
# $Id: objcopy.py 803 2023-03-17 11:16:38Z AMesser $:
#
# Provides tasks and functions for use with object copy
########################################################################################
import waflib.Task
import waflib.TaskGen
import os

hilscher_waf_dir = os.path.abspath(os.path.dirname(os.path.dirname(__file__)))

def configure(conf):
    conf.load('hilscher_dist', tooldir = [ hilscher_waf_dir ] )

class objcopy(waflib.Task.Task):
    u''' Run objcopy on a file '''
    color   = 'PINK'
    log_str = '[OBJCOPY] $SOURCES $TARGETS'
    run_str = '${OBJCOPY} -O ${TARGET_BFDNAME} ${OBJCOPYFLAGS} ${SRC} ${TGT}'


@waflib.TaskGen.taskgen_method
def create_objectcopy_task(self, source, target, bfdname, flags = []):
    tsk = self.create_task('objcopy', source, target)

    tsk.env.TARGET_BFDNAME = bfdname
    tsk.env.OBJCOPYFLAGS   = self.to_list(flags)

    return tsk

@waflib.TaskGen.taskgen_method
def binary_from_elf(self, source, target):
    return self.create_objectcopy_task(source, target, "binary")


@waflib.TaskGen.taskgen_method
def ihex_from_elf(self, source, target):
    return self.create_objectcopy_task(source, target, "ihex")

@waflib.TaskGen.feature('ihex')
@waflib.TaskGen.after_method('apply_link')
def build_rawimage(self):
    if not ('cprogram' in self.features or 'cxxprogram' in self.features):
        return

    elf_node = self.link_task.outputs[0]
    tsk = self.ihex_from_elf(elf_node, elf_node.change_ext('.ihex'))
    self.add_dist_nodes(tsk.outputs[0])

@waflib.TaskGen.feature('rawimage')
@waflib.TaskGen.after_method('apply_link')
def build_rawimage(self):
    if not ('cprogram' in self.features or 'cxxprogram' in self.features):
        return

    elf_node = self.link_task.outputs[0]
    tsk = self.binary_from_elf(elf_node , elf_node.change_ext('.bin'))
    self.add_dist_nodes(tsk.outputs[0])

# Also provide original method from waf for convience
@waflib.TaskGen.feature('objcopy')
@waflib.TaskGen.after_method('apply_link')
def objcopy(self):
    link_output = self.link_task.outputs[0]

    objcopy_target = link_output.change_ext('.' + self.objcopy_bfdname).name

    self.create_objectcopy_task( link_output,
                                 self.path.find_or_declare(objcopy_target),
                                 getattr(self, 'objcopy_flags', ""))

    self.add_dist_nodes(objcopy_target)

