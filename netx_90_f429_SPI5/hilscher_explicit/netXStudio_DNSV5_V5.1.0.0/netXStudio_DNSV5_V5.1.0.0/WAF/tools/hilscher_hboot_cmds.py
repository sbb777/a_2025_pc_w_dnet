#! /usr/bin/env python
# vim:fileencoding=utf-8:sw=4:tw=4:et
########################################################################################
# Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.
########################################################################################
# $Id: hilscher_hboot_cmds.py 1096 2025-01-23 10:12:14Z AMesser $:
#
# Common commands to build hboot based firmwares
########################################################################################
import os.path

from waflib import Utils
from waflib.Configure import conf
from waflib.TaskGen   import after_method,feature

def configure(conf):
    pass

@conf
def generate_hboot_image(bld, target, netx_type, hboot_xml, use, **kw):
    """
    """

    features = Utils.to_list(kw.pop('features', [])) + ['hboot']
    bld(target = target,
        features = features,
        netx_type = netx_type,
        hboot_xml = hboot_xml, use=use, **kw)

allowed_sdram_split_offset_values = (
    0, # No split
    0x00400000, # Enable Split for 8 MB SDRAM
    0x00800000, # Enable Split for 16 MB SDRAM
    0x01000000, # Enable Split for 32 MB SDRAM
    0x02000000, # Enable Split for 64 MB SDRAM
    0x04000000, # Enable Split for 128 MB SDRAM
    0x08000000, # Enable Split for 256 MB SDRAM
)

@conf
def generate_netx90_app_image(bld, target, netx_type, use,
                              segments_intflash = None, segments_extflash = None,
                              headeraddress_extflash = None,
                              sdram_split_offset = None, **kw):
    u""" build an nai/nae flash image to contain the application
         firmware """

    global allowed_sdram_split_offset_values

    name = target

    features = Utils.to_list(kw.pop('features', [])) + ['netx90_app_image']

    if segments_intflash is not None:
        segments_intflash = Utils.to_list(segments_intflash)[:]

        if not segments_intflash:
            bld.fatal(u'Empty argument segments_intflash specified for target %r' % name)

    if segments_extflash is not None:
        segments_extflash = Utils.to_list(segments_extflash)[:]

        if not segments_extflash:
            bld.fatal(u'Empty argument segments_extflash specified for target %r' % name)

        if not segments_intflash:
            bld.fatal(u'Empty argument segments_intflash while using extflash specified for target %r' % name)

    if sdram_split_offset is not None:
        if not isinstance(sdram_split_offset,int):
            bld.fatal(u'Argument sdram_split_offset for target %r must be a number' % (name))

        if sdram_split_offset not in allowed_sdram_split_offset_values:
            bld.fatal(u'Argument sdram_split_offset for target %r set to unsupported value 0x%08x' % (name,sdram_split_offset))

    if headeraddress_extflash is not None:
        if not isinstance(headeraddress_extflash,int):
            bld.fatal(u'Argument headeraddress_extflash for target %r must be a number' % (name))

    if ((segments_extflash is not None) != (sdram_split_offset is not None)) or\
       ((segments_extflash is not None) != (headeraddress_extflash is not None)):
        bld.fatal(u'Either all or none of arguments segments_extflash,sdram_split_offset and headeraddress_extflash must be set for %r' % name)

    bld(target = target,
        features = features,
        netx_type = netx_type,
        segments_intflash      = segments_intflash,
        segments_extflash      = segments_extflash,
        headeraddress_extflash = headeraddress_extflash,
        sdram_split_offset     = sdram_split_offset,
        use=use, **kw)

module_path = os.path.dirname(os.path.abspath(__file__))

@feature('netx90_app_image')
@after_method('check_tgen_availability')
def build_netx90_app_image(self):
    bld    = self.bld

    # build list of target nodes (output files)
    target_nai_node = self.path.find_or_declare(self.target)
    target_nodes = [target_nai_node]

    if self.segments_extflash:
        target_nodes.append(target_nai_node.change_ext('.nae'))

    # locate pre-defined xml file
    app_image_xml_node = bld.root.find_resource(module_path + os.sep + 'netx90_app_image.xml')

    if app_image_xml_node is None:
        bld.fatal(u'APP image xml file %r not found in %r' %('netx90_app_image.xml', module_path) )

    elf_inputs = []
    platform   = ''
    toolchain  = getattr(self,'toolchain','')

    for x in self.to_list(getattr(self, 'use', None)):
        t = bld.get_tgen_by_name(x)

        platform  = platform or  t.platform.lower()
        toolchain = toolchain or t.toolchain.lower()

        if toolchain != t.toolchain:
            bld.fatal('Toolchain of all source elfs must be identical')

        if platform != t.platform:
            bld.fatal('Platforms of all source elfs must be identical')

        if not getattr(t, 'posted', None):
            t.post()

        elf_inputs.append(t.link_task.outputs[0])

    self.platform  = platform
    self.toolchain = toolchain

    # setup environment according to toolchain of use references
    self.env = self.bld.all_envs['toolchain_%s' % toolchain].derive()

    # build the app image file
    self.app_image_task = self.create_task('netx90_app_image',
        [app_image_xml_node] + elf_inputs, target_nodes)

    self.add_dist_nodes(target_nodes)

@feature('hboot')
@after_method('build_hboot')
def add_hboot_sniplib(self):
    ''' Add custom sniplib to hboot '''
    bld = self.bld

    sniplib = getattr(self,'sniplib', None)

    if sniplib is not None:
        if isinstance(sniplib, bld.node_class):
            sniplib_node = sniplib
        else:
            sniplib_node = self.path.find_dir(sniplib)

        if sniplib_node is None:
            bld.fatal('Sniplib not found in %r' % sniplib)

        if not os.path.isdir(sniplib_node.abspath()):
            bld.fatal('Sniplib not found in %r' % sniplib)

        self.hboot_task_nxi.sniplib_node = sniplib_node

        hboot_task_nxe = getattr(self, 'hboot_task_nxe', None)
        if hboot_task_nxe is not None:
            hboot_task_nxe.sniplib_node = sniplib_node
