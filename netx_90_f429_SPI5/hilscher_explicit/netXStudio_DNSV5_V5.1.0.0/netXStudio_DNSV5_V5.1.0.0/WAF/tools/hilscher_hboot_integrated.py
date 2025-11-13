#! /usr/bin/env python
# vim:fileencoding=utf-8:sw=4:tw=4:et
########################################################################################
# Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.
########################################################################################
# $Id: hilscher_hboot_integrated.py 1096 2025-01-23 10:12:14Z AMesser $:
#
# Support code for hboot compiler integrated into waf
########################################################################################
import os

from waflib import Logs, Task, Utils
from waflib.Configure import conf
from waflib.TaskGen   import after_method,before_method,feature
from waflib.Context   import STDOUT, BOTH

hboot_image_compiler_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), '..' , 'hboot_image_compiler'))

def options(opt):
    opt.load('hilscher_hboot_cmds hilscher_firmware_netx90 python')

def configure(conf):
    conf.setenv('')
    conf.load('hilscher_hboot_cmds hilscher_firmware_netx90 python')

    #check python version
    hboot_python_version = conf.cmd_and_log([conf.env.get_flat('PYTHON'), '-c', 'import sys; sys.stdout.write(str(tuple(sys.version_info)))'], quiet=BOTH)
    hboot_python_version = eval(hboot_python_version, {})

    if hboot_python_version[0:3] < (2,7,9):
        Logs.pprint('YELLOW', "Warning: hboot tools might not work with python version %d.%d.%d" % (hboot_python_version[0],hboot_python_version[1], hboot_python_version[2]))

    # check if hboot_image_compiler is available
    p = os.path.abspath( os.path.join(hboot_image_compiler_dir, 'com'))
    if not os.path.isdir(p):
        p = ''

    conf.msg("Checking for hboot image compiler", p or False)
    conf.env['HBOOT_IMAGE_COMPILER'] = p

    # check if netx90_app_iflash_image.py is available
    p = os.path.abspath( os.path.join(hboot_image_compiler_dir, 'com', 'netx90_app_iflash_image.py'))
    if not os.path.isfile(p):
        p = ''

    conf.msg("Checking for app intflash image update tool", p or False)
    conf.env['HBOOT_APPFLASH_IMAGE'] = p

    # check if netx90_app_image.py is available
    p = os.path.abspath( os.path.join(hboot_image_compiler_dir, 'app', 'netx90_app_image.py'))
    if not os.path.isfile(p):
        p = ''

    conf.msg("Checking for app image update tool", p or False)
    conf.env['HBOOT_APP_IMAGE'] = p


@conf
def generate_netx90_intflash2_image(bld, target, use, **kw):
    u""" build an application image for application flash 2 for netx90.
       The image must be compiled with proper linkerscript in order to be usefull
    """
    features = Utils.to_list(kw.pop('features', [])) + ['nx90_intflash2_image']
    bld(target = target, features = features,  use=use, **kw)

default_hboot_patch_tables = {
  'NETX56'          : 'hboot_netx56_patch_table.xml',
  'NETX90'          : 'hboot_netx90_patch_table.xml',
  'NETX90B'         : 'hboot_netx90b_patch_table.xml',
  'NETX90_MPW'      : 'hboot_netx90_mpw_patch_table.xml',
  'NETX4000_RELAXED': 'hboot_netx4000_relaxed_patch_table.xml',
  'NETX4000'        : 'hboot_netx4000_patch_table.xml',
  'NETX4100'        : 'hboot_netx4000_patch_table.xml',
}

@feature('hboot')
@after_method('check_tgen_availability')
def build_hboot(self):
    '''
    Function to build a netX bootable image by performing an OBJCOPY and
    prepending a netX bootheader (64 byte).
    '''
    global default_hboot_patch_tables

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

    if self.netx_type not in default_hboot_patch_tables:
        bld.fatal('Unsupported netx type %s when building hboot image' % self.netx_type)

    self.platform  = platform
    self.toolchain = toolchain

    # setup environment according to toolchain of use references
    self.env = self.bld.all_envs['toolchain_%s' % toolchain].derive()

    patch_table_path = self.env.get_flat('HBOOT_IMAGE_COMPILER') +\
                           '/../patch_tables/%s' % default_hboot_patch_tables[self.netx_type]

    patch_table_node = bld.root.find_node(patch_table_path)

    if not patch_table_node:
        bld.fatal('Patch table %r not found' % patch_table_path)

    # build the nxi file
    hboot_task_nxi = self.hboot_task_nxi = self.create_task('hboot',
        [hboot_xml_nodes[0], patch_table_node] + elf_inputs,
        [target])

    # register nxi for distribution
    self.add_dist_nodes([target])

    # build nxe file if required
    if len(hboot_xml_nodes) > 1:
        target_nxe = target.change_ext('.nxe')

        self.hboot_task_nxe = self.create_task('hboot',
            [hboot_xml_nodes[1], patch_table_node] + elf_inputs,
            [target_nxe])

        # register nxe for distribution
        self.add_dist_nodes(target_nxe)


@feature('nx90_intflash2_image')
@after_method('check_tgen_availability')
def build_nx90_intflash2_image(self):
    '''
    Function to build a netX 90 APP bootable image for flash 2.
    '''
    bld = self.bld
    target     = self.path.find_or_declare(self.target)

    target_tmp = target.parent.find_or_declare(target.name + '.tmp')

    elf_inputs = []
    platform   = ''
    toolchain  = ''

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

    self.binary_from_elf(elf_inputs, [target_tmp])
    self.image_task = self.create_task('appflash_update', [target_tmp], [target])


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

        cmd = [env.get_flat('PYTHON'), env.get_flat('HBOOT_IMAGE_COMPILER'), '--verbose']

        if not all(bool(x) for x in cmd):
            bld.fatal('Either Python or HBOOT image compiler not available')

        hboot_xml, patch_table_node = self.inputs[0:2]
        elf_inputs  = self.inputs[2:]

        # build hboot compiler commandline
        cmd.append('--include=%s'   % hboot_xml.parent.abspath())
        cmd.append('--netx-type=%s' % tgen.netx_type)
        cmd.append('--objdump=%s'   % env.get_flat('OBJDUMP'))
        cmd.append('--objcopy=%s'   % env.get_flat('OBJCOPY'))
        cmd.append('--readelf=%s'   % env.get_flat('READELF'))


        cmd.append('--patch-table=%s' % patch_table_node.abspath())

        sniplib_node = getattr(self,'sniplib_node', None)
        if sniplib_node is not None:
            cmd.append('--sniplib=%s'   % sniplib_node.abspath())

        for i,x in enumerate(elf_inputs):
            cmd.append('--alias=tElf%d=%s' % (i, x.abspath()))

        cmd.append(hboot_xml.abspath())
        cmd.append(self.outputs[0].abspath())

        # run boot image creator
        dct = dict(os.environ)
        dct['LANG']='C'

        out,err = bld.cmd_and_log(cmd, env = dct, output=BOTH, quiet=STDOUT)

        # hboot_image_compiler -v --netx-type 4000 --objcopy %GCC_ARM_PATH%/bin/arm-none-eabi-objcopy --objdump %GCC_ARM_PATH%/bin/arm-none-eabi-objdump --readelf %GCC_ARM_PATH%/bin/arm-none-eabi-readelf --alias tElfCR7=netx4000.elf CR7_DDR600.xml CR7_DDR600.bin

class netx90_app_image(Task.Task):
    u''' Build application image for netX90'''
    color     = 'PINK'
    inst_to   = None
    cmdline   = None
    log_str   = '[APPIMG] $TARGETS'

    def run(self):
        tgen = self.generator
        bld = tgen.bld

        env = self.env

        cmd = [env.get_flat('PYTHON'), env.get_flat('HBOOT_APP_IMAGE'), '--verbose']

        if not all(bool(x) for x in cmd):
            bld.fatal('Either Python or app image compiler not available')

        app_image_xml = self.inputs[0]
        elf_inputs    = self.inputs[1:]
        hboot_nodes   = self.outputs

        def append_segment_alias(alias, value):
            arg = '--alias=%s=' % alias

            if value is not None:
                if len(value) > 0:
                    cmd.append(arg + ','.join(value))
                else:
                    cmd.append(arg + ',')
            else:
                cmd.append(arg)

        # build app image compiler commandline
        cmd.append('--netx-type=%s' % tgen.netx_type)
        cmd.append('--objdump=%s'   % env.get_flat('OBJDUMP'))
        cmd.append('--objcopy=%s'   % env.get_flat('OBJCOPY'))
        cmd.append('--readelf=%s'   % env.get_flat('READELF'))

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

        # run app image creator
        dct = dict(os.environ)
        dct['LANG']='C'

        out,err = bld.cmd_and_log(cmd, env = dct, output=BOTH, quiet=STDOUT)

class appflash_update(Task.Task):
    u''' Update application binary flash file'''
    color     = 'PINK'
    inst_to   = None
    cmdline   = None
    log_str   = '[APPFLASH] $TARGETS'

    def run(self):
        tgen = self.generator
        bld = tgen.bld

        env = self.env
        cmd = [env.get_flat('PYTHON'), env.get_flat('HBOOT_APPFLASH_IMAGE'), '--verbose']

        if not all(bool(x) for x in cmd):
            bld.fatal('Either Python or HBOOT image compiler not available')

        cmd.append(self.inputs[0].abspath())
        cmd.append(self.outputs[0].abspath())

        out,err = bld.cmd_and_log(cmd, output=BOTH, quiet=STDOUT)

@feature('nai')
@after_method('build_netx90_app_image')
def build_nai(self):
    u''' Inject a new task which will patch the app hboot image to become a
         nai/nae file.

         This code is used only for integrated hboot compiler
         since external hboot compiler does this on its own now'''

    ''' Generate nai header checksums when running with integrated hboot compiler'''
    target_nodes = self.app_image_task.outputs

    # use a different name for hboot compiler output
    app_image_nodes = [x.parent.find_or_declare('%s.unpatched' % x.name) for x in target_nodes]
    self.app_image_task.outputs = app_image_nodes

        # patch & update the headers
    if len(target_nodes) == 1:
        self.nai_task = self.create_task('generate_nai',
            app_image_nodes, target_nodes)
    else:
        self.nai_nae_task = self.create_task('generate_nai_nae',
            app_image_nodes, target_nodes)
