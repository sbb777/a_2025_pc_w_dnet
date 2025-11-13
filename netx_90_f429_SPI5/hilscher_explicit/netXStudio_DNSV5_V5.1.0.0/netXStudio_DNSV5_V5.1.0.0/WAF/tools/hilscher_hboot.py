#! /usr/bin/env python
# vim:fileencoding=utf-8:sw=4:tw=4:et
########################################################################################
# Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.
########################################################################################
# $Id: hilscher_hboot.py 1096 2025-01-23 10:12:14Z AMesser $:
#
# Hilscher hboot compiler support code
########################################################################################
import os.path

from waflib import Errors, Task, Logs
from waflib.Context   import STDOUT, BOTH
from waflib.TaskGen import feature, after_method


def options(opt):
    opt.load('hilscher_hboot_cmds hilscher_firmware_netx90')

def configure(conf):
    conf.setenv('')
    conf.load('hilscher_hboot_cmds hilscher_firmware_netx90')

    # check if hboot_image_compiler is available
    cmd = conf.cmd_to_list(conf.find_program('hboot_image_compiler_com', var='HBOOT_COMPILER_COM'))
    conf.env.HBOOT_COMPILER_COM = cmd

    cmd = conf.cmd_to_list(conf.find_program('hboot_image_compiler_app', var='HBOOT_COMPILER_APP'))
    conf.env.HBOOT_COMPILER_APP = cmd

@feature('hboot')
@after_method('check_tgen_availability')
def build_hboot(self):
    '''
    Function to build a netX bootable image by performing an OBJCOPY and
    prepending a netX bootheader (64 byte).
    '''
    bld    = self.bld
    target = self.path.find_or_declare(self.target)

    hboot_xmls      = self.to_list(self.hboot_xml)

    if len(hboot_xmls) not in (1,2):
        bld.fatal('Unexpected number of HBoot xml description files defined (expected one or two xml files)')

    hboot_xml_nodes = list(self.path.find_resource(x) for x in hboot_xmls)

    for x,y in zip(hboot_xml_nodes, hboot_xmls):
        if not x:
            bld.fatal('HBoot xml description file "%s" not found' % (self.path.nice_path() + os.path.sep + y))

    elf_inputs = []
    platform   = ''
    toolchain  = getattr(self,'toolchain','')

    for x in self.to_list(getattr(self, 'use', None)):
        t = bld.get_tgen_by_name(x)

        platform  = platform or  t.platform.lower()
        toolchain = toolchain or t.toolchain.lower()

        if toolchain != t.toolchain:
            bld.fatal('HBoot toolchain of all source elfs must be identical')

        if platform != t.platform:
            bld.fatal('HBoot platforms of all source elfs must be identical')

        if not getattr(t, 'posted', None):
            t.post()

        elf_inputs.append(t.link_task.outputs[0])

    if self.netx_type == 'NETX90':
        Logs.pprint('YELLOW', "Warning: target %s using netx_type='NETX90' is netX90 rev. 0. (Most likely you want 'NETX90B')" % (self.name))

    self.platform  = platform
    self.toolchain = toolchain

    # setup environment according to toolchain of use references
    self.env = self.bld.all_envs['toolchain_%s' % toolchain].derive()

    # build the nxi file
    hboot_task_nxi = self.hboot_task_nxi = self.create_task('hboot',
        [hboot_xml_nodes[0]] + elf_inputs,
        [target])

    # register nxi for distribution
    self.add_dist_nodes([target])

    # build nxe file if required
    if len(hboot_xml_nodes) > 1:
        target_nxe = target.change_ext('.nxe')

        self.hboot_task_nxe = self.create_task('hboot',
            [hboot_xml_nodes[1]] + elf_inputs,
            [target_nxe])

        # register nxe for distribution
        self.add_dist_nodes(target_nxe)

class hboot(Task.Task):
    ''' Run objcopy on the target'''
    color     = 'PINK'
    inst_to   = None
    cmdline   = None
    log_str   = '[HBOOT] $TARGETS'

    def run(self):
        tgen = self.generator
        bld = tgen.bld

        env = self.env

        if not env.HBOOT_COMPILER_COM:
            bld.fatal('hboot compiler tool not available')

        hboot_xml    = self.inputs[0]
        elf_inputs   = self.inputs[1:]

        output_node, = self.outputs

        # build hboot compiler commandline
        cmd = env.HBOOT_COMPILER_COM + [
            '--verbose',
            '--netx-type=%s' % tgen.netx_type,
            '--include=%s'   % hboot_xml.parent.abspath(),
        ]

        output_elf_node = output_node.change_ext('.hboot.elf')
        cmd.append('--output-elf-file=%s' % output_elf_node.abspath())

        sniplib_node = getattr(self,'sniplib_node', None)
        if sniplib_node is not None:
            cmd.append('--sniplib=%s'   % sniplib_node.abspath())

        for i,x in enumerate(elf_inputs):
            cmd.append('--alias=tElf%d=%s' % (i, x.abspath()))

        cmd.append(hboot_xml.abspath())
        cmd.append(output_node.abspath())

        self.run_hboot_tool(cmd, extra_outputs=[output_elf_node])

    def run_hboot_tool(self, cmd, extra_outputs):
        bld = self.generator.bld

        output_nodes = self.outputs + extra_outputs

        for x in output_nodes:
            x.delete()

        # run boot image creator
        dct = dict(os.environ)
        dct['LANG']='C'

        try:
            out,err = bld.cmd_and_log(cmd, env = dct, output=BOTH, quiet=STDOUT)
        except Errors.WafError as e:
            out, err = e.stdout, e.stderr
            bld.fatal('Running hboot compiler failed: %s, %s' % (out,err))

        # hboot compiler does not set return code properly.
        for x in output_nodes:
            if not os.path.isfile(x.abspath()):
                bld.fatal('Running hboot compiler failed, file %s is missing: %s, %s' % (x.nice_path(),out,err))

class netx90_app_image(hboot):
    u''' Build application image for netX90'''
    log_str   = '[APPIMG] $TARGETS'

    def run(self):
        tgen = self.generator
        bld = tgen.bld

        env = self.env

        if not env.HBOOT_COMPILER_APP:
            bld.fatal('hboot compiler tool not available')

        app_image_xml = self.inputs[0]
        elf_inputs    = self.inputs[1:]
        hboot_nodes   = self.outputs

        cmd = env.HBOOT_COMPILER_APP + [
            '--verbose',
            '--netx-type=%s' % tgen.netx_type,
        ]

        extra_outputs = []

        # No elf generated for netx90_rev0
        if tgen.netx_type != 'netx90_rev0':
            output_elf_node = hboot_nodes[0].change_ext('.hboot.elf')

            cmd.append('--output-elf-file=%s' % output_elf_node.abspath())

            extra_outputs.append(output_elf_node)

        def append_segment_alias(alias, value):
            arg = '--alias=%s=' % alias

            if value is not None:
                if len(value) > 0:
                    cmd.append(arg + ','.join(value))
                else:
                    cmd.append(arg + ',')
            else:
                cmd.append(arg)

        if tgen.sdram_split_offset is not None:
            cmd.append('--sdram_split_offset=0x%x' % tgen.sdram_split_offset)

        append_segment_alias('segments_intflash', tgen.segments_intflash)
        append_segment_alias('segments_extflash', tgen.segments_extflash)

        if tgen.headeraddress_extflash is not None:
            cmd.append('--alias=headeraddress_extflash=0x%x' % tgen.headeraddress_extflash)

        for i,x in enumerate(elf_inputs):
            cmd.append('--alias=tElf%d=%s' % (i, x.abspath()))

        cmd.append(app_image_xml.abspath())

        for x in hboot_nodes:
            cmd.append(x.abspath())

        self.run_hboot_tool(cmd, extra_outputs=extra_outputs)

@feature('nai')
def dummy(self):
    ''' Helper to avoid waf complain about feature '''
    pass