#! /usr/bin/env python
# encoding: utf-8
########################################################################################
# Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.
########################################################################################
# $Id: hilscher_gencode.py 713 2022-08-11 13:24:46Z AMesser $:
#
# Provides tasks and functions for use when providing generated source files
########################################################################################
import waflib.Task
import waflib.TaskGen
import waflib.Configure

@waflib.TaskGen.taskgen_method
def generated_source(self, node):
    sources, headers = self.generated_code = getattr(self, "generated_code", ([],[]))
    sources.append(node)

@waflib.TaskGen.taskgen_method
def generated_header(self, node):
    sources, headers = self.generated_code = getattr(self, "generated_code", ([],[]))
    headers.append(node)

@waflib.TaskGen.feature("c", "cxx", "asm")
@waflib.TaskGen.before_method("process_source", "apply_incpaths")
def process_generated_source(self):
    u''' Scans any targets referenced by 'use_source' for generated
         source files to incoporate into current target '''

    includes = self.includes = self.to_list(getattr(self, 'includes', []))

    use_source = self.to_list(getattr(self, "use_source", []))

    for x in use_source:
        tgen = self.bld.get_tgen_by_name(x)

        if not getattr(tgen, 'posted', None):
            tgen.post()

        sources, headers = tgen.generated_code

        for node in sources:
            self.get_hook(node)(self,node)

        for node in headers:
            includes.append(node.parent)