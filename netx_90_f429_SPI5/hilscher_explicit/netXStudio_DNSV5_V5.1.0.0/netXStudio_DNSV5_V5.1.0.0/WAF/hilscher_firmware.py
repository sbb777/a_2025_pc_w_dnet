#! /usr/bin/env python
# vim:fileencoding=utf-8:sw=4:tw=4:et
########################################################################################
# Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.
########################################################################################
# $Id: hilscher_firmware.py 1096 2025-01-23 10:12:14Z AMesser $:
#
# Hilscher firmware processing stuff
########################################################################################
from waflib import Task, Utils
from waflib.Configure import conf
from waflib.TaskGen import feature, after_method, taskgen_method
from waflib import Logs
import re
import os.path
from netx_image_generator.builder  import NxoBuilder, nxupdate_fn


hilscher_waf_dir = os.path.abspath(os.path.dirname(__file__))

# waf buildsystem related hooks to initialize this module
def options(opt):
    global hilscher_waf_dir
    opt.load('hilscher_libsused', tooldir = [ hilscher_waf_dir ] )

def configure(conf):
    global hilscher_waf_dir
    conf.load('hilscher_libsused', tooldir = [ hilscher_waf_dir ] )
    conf.load('objcopy')

def display_findings(bld):
    findings = bld.findings

    separatorline = '{0:=^80}'.format

    if findings:
        Logs.warn(separatorline(' Findings '))

        for node in sorted(findings, key= lambda x : x.nice_path()):
            title = '%s' % (node.nice_path())

            Logs.warn(title)
            Logs.warn('-' * len(title))

            for msg in sorted(findings[node]):
                Logs.warn('- %s' % msg)

            Logs.warn('')

        Logs.warn(separatorline(''))

@taskgen_method
def report_finding(self, node, msg):
  bld = self.bld

  if display_findings not in getattr(bld, 'post_funs', []):
      bld.findings = {}
      bld.add_post_fun(display_findings)

  try:
      bld.findings[node].add(msg)
  except KeyError:
      bld.findings[node] = set([msg])


''' ------------------------------------------
   NXO/Loadable Module
   ------------------------------------------ '''
@feature('remove_librcx')
@after_method('apply_link', 'process_use')
def process_removelibrcx(self):
  # features are also applied to Objects, so we will receive
  # a call for thumb_objs / arm_objs which don't have link task
  if not getattr(self, 'link_task', None):
      return

  tmp_list = list(self.link_task.env['STLIB'])
  for lib_to_remove in tmp_list:
      # find archives the name starts with 'rcx'
      # do not remove 'rcx_module*' and 'rcx_netxNNN_2portswitch'
      m = re.match('rcx(_(vol|bas|mid|midshm|hif_cifx|netx\d+(_physhif|_hif)?))?$', lib_to_remove)
      if m:
          # remove and warn, because there is a wrong use-component in wscript for this target
          Logs.pprint('YELLOW', "Warning: use-component '{0}' removed from build of target '{1}'".format(lib_to_remove, self.name))
          self.link_task.env['STLIB'].remove(lib_to_remove)

@conf
def module(bld, target, fileheader_source, platform = None, toolchain = None,**kw):

  kw['target']    = target
  kw['platform']  = platform
  kw['toolchain'] = toolchain

  bld.handle_toolchain(kw)

  description      = kw.pop('description', '')
  use              = kw.pop('use',        [])
  source           = kw.pop('source',     [])
  source_arm       = kw.pop('source_arm', [])

  kw.pop('target', None)
  name = kw.pop('name', target)

  tempelf_target = bld.path.find_or_declare(target)
  tempelf_target = str(tempelf_target.change_ext(''))
  tempelf_name   = target + '.nxo'

  install_path = kw.pop('install_path', None)

  features_program = Utils.to_list(kw.pop('features', [])) + ['remove_libc', 'remove_librcx', 'buildstamp']
  features_nxo     = []

  if 'libsused' in features_program:
      features_program.remove('libsused')
      features_nxo = ['libsused']

  # programm
  bld.program(
          target      = tempelf_target,
          name        = tempelf_name,
          features    = features_program,
          source      = source,
          source_arm  = source_arm,
          use         = use,
          linkflags   = Utils.to_list(kw.pop('linkflags', [])) + ['-Wl,-r'],
          hidden_from_list = "Internal",
          **kw)

  nxo_sources = Utils.to_list(fileheader_source) + Utils.to_list(kw.get('taglist_source', []))

  bld(features      = features_nxo + ['c', 'nxo'],
      source        = nxo_sources,
      target        = target,
      name          = name,
      moduleelf     = tempelf_name,
      fileheader_source = fileheader_source,
      use           = Utils.to_list(use),
      description   = description,
      install_path  = install_path,
      **kw)

@feature('nxo')
@after_method('apply_link', 'process_source')
def build_nxo(self):
    '''
    Function to build a netX loadable module.
    '''
    if not getattr(self, 'use', None):
      self.bld.fatal("Building a NXO requires 'use' parameter")

    inputs =  [None,None,None]
    outputs = [self.path.find_or_declare(self.target)]

    # Search our elf file to convert
    try:
        tg = self.bld.hil_get_tgen_by_name(self.moduleelf)
        # is already posted
        if not getattr(tg,'posted',None):
          tg.post()
    except KeyError:
        Logs.pprint('YELLOW',"Skipping nxo-file \'%s\' because no matching tgen found. (Probably the source elf was not build because of missing toolchain)" % (target))
        return

    if getattr(tg, 'link_task', None):
      # Only use .elf files as inputs
      for x in tg.link_task.outputs:
          if x.suffix().lower() == '.elf':
              inputs[0] = x
    else:
        self.bld.fatal('Input tgen has no link task')

    fileheader_node = self.path.find_node(self.fileheader_source)

    if getattr(self, 'taglist_source', None):
        taglist_node = self.path.find_node(self.taglist_source)
    else:
        taglist_node = None

    for x in self.compiled_tasks:
        if (x.inputs[0] == fileheader_node):
            inputs[1] = x.outputs[0]
        elif (x.inputs[0] == taglist_node):
            inputs[2] = x.outputs[0]

    # create task and adapt its env
    self.nxobuilder_task = self.create_task('NxoBuilderTask', inputs,outputs)
    self.nxobuilder_task.env  = tg.env.derive()

    self.add_dist_nodes(outputs)

class NxoBuilderTask(Task.Task):
    color   = 'PINK'
    cmdline = None
    inst_to = None

    def run(self):
        inputfile  = self.inputs[0].abspath()
        outputfile = self.outputs[0].get_bld().abspath()
        fileheader = self.inputs[1]
        taglist    = self.inputs[2]

        if not NxoBuilder(self, outputfile, inputfile, fileheader, taglist):
          self.generator.bld.fatal("NXO building failed")

''' ------------------------------------------
   Firmware
   ------------------------------------------ '''
@conf
def firmware(bld, target, linkerscript, platform, toolchain,**kw):
  """
  Function to build a firmware (nxf/nxo[todo]) from sources and libraries.
  <b>  Mandatory parameters  </b>
  @param target    \b string: file name of the target (e.g. netX.nxf)
  @param platform  \b string: type of platform, that specified compiler parameter set. Values: \n\e netx - common parameters for all netx platforms \n\e netx500 - parameters specific for netX500 \n\e netx100 - parameters specific for netX100 \n\e netx50  - parameters specific for netX50 \n\e netx51  - parameters specific for netX51 \n\e netx10  - parameters specific for netX10
  @param toolchain \b string: identification of toolchain to use in build. \n For example: \e 'hitex' or \e 'codesourcery'. See 'hilscher_toolchains.py' for more toolchains or define therein a new one. \n
  @param linkerscript \b string: linker script
  @param BOOTBLOCKERFLAGS <b> list of strings: </b>parameters for bootblocker
  \n\n\n
  <b> Optional parameters: </b>\n\n
  @param description \b string: description of this target, will be listed in "waf list"
  @param sources       <b> list of strings: </b> list of source files (*.c/*.s) in absolute or relative path, that compiler translates in 'thumb' mode
  @param sources_thumb <b> list of strings: </b> (equivalent to \e sources) list of source files (*.c/*.s) in absolute or relative path, that compiler translates in 'thumb' mode
  @param sources_arm   <b> list of strings: </b> list of source files (*.c/*.s) in absolute or relative path, that compiler translates in 'arm' mode
  @param includes <b> list of strings: </b> list of include directories to include in build of source files
  @param use <b> list of strings: </b> list of libraries to use in compile stage (their 'export_defines' and 'export_includes' are used for build too)
  @param defines <b> list of strings: </b> list of define directives to use in build of source files
  \n\n\n
  <b> Additional parameters for compiler, assembler and linker: </b>\n\n
  @param stlib <b> list of strings: </b> used static libraries
  @param stlibpath <b> list of strings: </b> path to static libraries
  @param linkflags <b> list of strings: </b> options, forwarded to the linker
  @param cflags    <b> list of strings: </b> options, forwarded to the C-Compiler
  @param cxxflags  <b> list of strings: </b> options, forwarded to the C++-Compiler
  \n\n\n
  <b> Installation: </b>\n\n
  @param install_path \b string:  (relative) path to install current component while "waf install"
  \n\n\n
  """
  # mandatory parameters
  kw['target']           = target
  kw['platform']         = platform
  kw['toolchain']        = toolchain
  kw['linkerscript']     = linkerscript

  bld.handle_toolchain(kw)

  install_path     = kw.pop('install_path', None)
  BOOTBLOCKERFLAGS = kw.pop('BOOTBLOCKERFLAGS', None)
  hboot_xml        = kw.pop('hboot_xml', None)
  netx_type        = kw.pop('netx_type', None)

  # nai support
  segments_intflash      = kw.pop('segments_intflash', None)
  segments_extflash      = kw.pop('segments_extflash', None)
  headeraddress_extflash = kw.pop('headeraddress_extflash', None)
  sdram_split_offset     = kw.pop('sdram_split_offset', None)

  tgt = bld.path.find_or_declare(target)

  kw.pop('target', None)
  name = kw.pop('name', target)

  # generate unique, identifiers for intermediate build product "elf file"

  # the target name (the name of the elf file) will be generate from base name
  # of the firmware to build (everything up to the file extension). The extension
  # '.elf' will be automatically append by bld.program below
  prog_target, _ = os.path.splitext(target)

  # The name of the elf file generator is just the name of the firmware file generator
  # prefixed with an underscore
  prog_name = '_' + name

  features = Utils.to_list(kw.pop('features', []))
  features_program  = list(set(features) | set(['buildstamp']))
  features_firmware = ['inherit_version']

  # extract firmware kind if defined
  kind = None
  if 'nxi' in features_program:
      kind = 'nxi'
      features_program.remove('nxi')
  if 'mxf' in features_program:
      kind = 'mxf'
      features_program.remove('nxi')
  if 'nxf' in features_program:
      kind = 'nxf'
      features_program.remove('nxf')
  if 'nai' in features_program:
      kind = 'nai'
      features_program.remove('nai')

  if 'libsused' in features_program:
      features_program.remove('libsused')
      features_firmware.append('libsused')

  # generate the elf file
  bld._program_internal(target           = prog_target,
                        name             = prog_name,
                        install_path     = [],
                        hidden_from_list = "Internal",
                        features         = features_program,
                        **kw)

  # check kind of firmware to build
  if kind is None:
      _, ext = os.path.splitext(target)

      kind = {
          '.nxi' : 'nxi',
          '.mxf' : 'mxf',
          '.nai' : 'nai',
      }.get(ext.lower(), 'nxf')

  kw_fw = {}
  kw_fw['description']  = kw.get('description', u' ')
  kw_fw['displaygroup'] = kw.get('displaygroup', u'default')
  kw_fw['version']      = kw.get('version', None)
  kw_fw['install_path'] = install_path

  if kind == 'nxf':
      if BOOTBLOCKERFLAGS is None:
          bld.fatal(u'Parameter "BOOTBLOCKERFLAGS" not defined for target %r' % name)

      # generate .nxf from elf file
      buildnxf(bld,
               target           = target,
               name             = name,
               use              = prog_name,
               features         = features_firmware,
               BOOTBLOCKERFLAGS = BOOTBLOCKERFLAGS,
               **kw_fw)
  elif kind in ('nxi', 'mxf'):
      features_firmware.append(kind)

      if hboot_xml is None:
          bld.fatal(u'Parameter "hboot_xml" not defined for target %r' % name)

      if netx_type is None:
          bld.fatal(u'Parameter "netx_type" not defined for target %r' % name)

      # generate .nxi / .nxe from elf file
      bld.generate_hboot_image(
               target           = target,
               name             = name,
               use              = prog_name,
               netx_type        = netx_type,
               hboot_xml        = hboot_xml,
               features         = features_firmware,
               **kw_fw)
  elif kind == 'nai':
      features_firmware.append(kind)

      if netx_type is None:
          bld.fatal(u'Parameter "netx_type" not defined for target %r' % name)

      # generate .nai from elf file
      bld.generate_netx90_app_image(
               target           = target,
               name             = name,
               use              = prog_name,
               netx_type        = netx_type,
               features         = features_firmware,
               segments_intflash      = segments_intflash,
               segments_extflash      = segments_extflash,
               headeraddress_extflash = headeraddress_extflash,
               sdram_split_offset     = sdram_split_offset,
               **kw_fw)

''' ------------------------------------------
   NXF Update support
   ------------------------------------------ '''
@conf
def buildnxf(bld, target, *k, **kw):
    """
    Function to build a firmware (nxf) from executable (elf).
    <b>  Mandatory parameters  </b>
    @param target    \b string: file name of firmware
    @param use <b> list of strings: </b> target, from binary will be constructed
    @param BOOTBLOCKERFLAGS <b> list of strings: </b>parameters for bootblocker
    \n\n\n
    <b> Optional parameters: </b>\n\n
    @param description \b string: description of this target, will be listed in "waf list"
    \n\n\n
    <b> Installation: </b>\n\n
    @param install_path \b string:  (relative) path to install current component while "waf install"
    \n\n\n
    """
    if (not 'BOOTBLOCKERFLAGS' in kw) and (not 'BOOTBLOCKERDATA' in kw):
        bld.fatal("Bootblocker flags are missing")

    kw['target']    = target
    kw['name_patch']= '~'+target+'.bin'
    if not 'name' in kw:
      kw['name']    = target

    features = Utils.to_list(kw.pop('features', [])) + ['bootimage', 'nxf']

    bld(features = features, **kw)

class nxupdate(Task.Task):
    ''' Run objcopy on the target'''
    color     = 'PINK'
    inst_to   = None
    cmdline   = None

    def run(self):
      inputfile   = self.inputs[0].get_bld().abspath()
      outputfile  = self.outputs[0].get_bld().abspath()
      nxupdate_fn(inputfile, outputfile)

@feature('nxf')
@after_method('build_bootimage')
def build_nxf(self):
    '''
    Function to build a netX bootable image by performing an OBJCOPY and
    prepending a netX bootheader (64 byte).
    '''
    target = self.path.find_or_declare(self.target)
    #target = self.path.make_node(self.name)

    input = self.to_list(getattr(self, 'use', None))

    self.nxf_task = self.create_task('nxupdate')

    #target = self.path.find_or_declare(self.name_real)

    self.nxf_task.outputs     = [target]

    self.nxf_task.inputs.extend(self.bootblocker_task.outputs)
    self.nxf_task.set_run_after(self.bootblocker_task)

    self.add_dist_nodes([target])

""" HELPER functions """


@conf
def SpecFWFileName(self, DeviceFamily, DeviceForm, netXType,
                   PrimaryProtocol, SecondaryProtocol, HWCompInd):
    """
    Function constructs the name of firmware according to document "Specification Firmware File Names" (revision 3, 2013-04-03) [H:\Manual netX Architecture\System\Firmware Names]

    @param DeviceFamily \b string: device family
    @param DeviceForm \b string: device form
    @param netXType \b string: netX type
    @param PrimaryProtocol \b string: primary protocol on stack
    @param SecondaryProtocol \b string: secondary protocol on stack
    @param HWCompInd \b string: hardware compatibility index
    @return  \b string: Firmware file name
    """
    #Table 3:
    device_family = {'netbrick'          :{None :'B0', '' :'B0',},
                     'cifx'              :{None :'C0', '' :'C0',},
                     'netdimm'           :{None :'D0', '' :'D0',},
                     'eu5c'              :{None :'E0', '' :'E0',},
                     'nethost'           :{'t100' :'FT'},
                     'netsmart'          :{None :'G0', '' :'G0',},
                     'nethmi'            :{'j500' :'HJ', 'b500' :'HB',},
                     'netic'             :{None :'I0', '' :'I0',},
                     'netjack'           :{None :'J0', '' :'J0',},
                     'netlink'           :{'usb':'LU', 'ethernet':'LN'},
                     'comx'              :{None :'M0', '' :'M0',},
                     'netpac'            :{None :'P0', '' :'P0',},
                     'netrapid'          :{None :'R0', '' :'R0',},
                     'netplc'            :{'c100':'SC', 'd100':'SD', 'j100':'SJ', 'j500':'SJ', 'm100':'SM', 't100':'ST'},
                     'nettap'            :{None :'T0', '' :'T0',},
                     'netx'              :{None :'X0', '' :'X0',},
                     'rcx'               :{None :'X0', '' :'X0',},
                     'rcx base firmware' :{None :'X0', '' :'X0',},
                    }
    #Table 3, remark:
    file_extention = ".nxf"
    if DeviceFamily.lower() in ['rcx', 'rcx base firmware']:
        file_extention = ".nxo"
    # Table 4:
    netx_type    =  {'netx5'   : '4',
                     'netx6'   : '8',
                     'netx10'  : '5',
                     'netx50'  : '3',
                     'netx51'  : '6',
                     'netx52'  : '7',
                     'netx100' : '2',
                     'netx500' : '1',
                    }
    #Table 5:
    protocol_table = { None                                   : '00',
                ''                                      : '00',
                'rcx'                                   : '00',
                'profibus-dp master'                    : '01',
                'profibus dp master'                    : '01',
                'profibus-dp slave'                     : '02',
                'profibus dp slave'                     : '02',
                'profibus-mpi'                          : '03',
                'profibus mpi'                          : '03',
                'canopen master'                        : '04',
                'canopen slave'                         : '05',
                'devicenet master'                      : '06',
                'devicenet slave'                       : '07',
                'as-interface master'                   : '08',
                'cc-link slave'                         : '09',
                'componet slave'                        : '0A',
                'io-link master'                        : '0B',
                'profinet io controller'                : '0C',
                'pnm'                                   : '0C',
                'profinet io device'                    : '0D',
                'pns'                                   : '0D',
                'ethercat master'                       : '0E',
                'ecm'                                   : '0E',
                'ethercat slave'                        : '0F',
                'ecs'                                   : '0F',
                'ethernet/ip scanner/master'            : '0G',
                'ethernet/ip adapter/slave'             : '0H',
                'sercos iii master'                     : '0I',
                's3m'                                   : '0I',
                'sercos iii slave'                      : '0J',
                's3s'                                   : '0J',
                'powerlink controlled node'             : '0K',
                'open modbus/tcp'                       : '0L',
                'rfc 1006'                              : '0M',
                'df1'                                   : '0N',
                '3964r'                                 : '0P',
                'ascii'                                 : '0Q',
                'modbus rtu (master/slave)'             : '0R',
                'modbus rtu'                            : '0R',
                'netscript (programmable serial)'       : '0S',
                'netscript'                             : '0S',
                'varan client (server)'                 : '0T',
                'varan client'                          : '0T',
                'varan server'                          : '0T',
                'varan'                                 : '0T',
                'smartwire-dt master'                   : '0U',
                'marshaller'                            : '0V',
                'atvise (ethernet)'                     : '10',
                'atvise'                                : '10',
                'profibus-dp master and codesys'        : '11',
                'profibus dp master and codesys'        : '11',
                'devicenet master and codesys'          : '16',
                'profinet io controller and codesys'    : '1C',
                'pnm and codesys'                       : '1C',
                'ethernet/ip scanner/master and codesys': '1G',
                'eip and codesys'                       : '1G',
                'codesys and local i/o'                 : '1Z',
                'profibus-dp master and proconos eclr'  : '21',
                'profibus dp master and proconos eclr'  : '21',
                'canopen master and proconos eclr'      : '24',
                'devicenet master and proconos eclr'    : '26',
                'proconos eclr and local i/o'           : '2Z',
                'profibus-dp master and ibh s7'         : '31',
                'profibus dp master and ibh s7'         : '31',
                'canopen master and ibh s7'             : '34',
                'devicenet master and ibh s7'           : '36',
                'ibh s7 and local i/o'                  : '3Z',
                }

    def get_protocol_id(protocol):
        ltd = False

        if isinstance(protocol, str):
            protocol = protocol.lower()

            if protocol.endswith(" limited"):
                protocol = protocol[:-8]
                ltd = True

            id = protocol_table[protocol]
        else:
            id = protocol_table[protocol]

        if ltd:
            if id[0] == '0':
                id = 'Z' + id[1:]
            else:
                self.fatal("Can not generate limited firmware name for protocol {protocol}".format(**locals()))

        return id

    error_msg = "error: wrong parameter specified in \"SpecFWFileName\" on argument "
    out = ''
    # 1st letter
    try:
        loc_device_form = device_family[DeviceFamily.lower()]
    except KeyError:
        self.fatal(error_msg + "\"DeviceFamily = %s\"" % DeviceFamily)
    # 2nd letter
    try:
        if DeviceForm:
            DeviceForm = DeviceForm.lower()
        out = loc_device_form[ DeviceForm ]
    except KeyError:
        self.fatal(error_msg + "\"DeviceForm = %s\". Available \"DeviceForm\": %s" % (DeviceForm, ', '.join([str(var) for var,value in loc_device_form.iteritems()]) ) )
    #3rd letter
    try:
        out += netx_type[ netXType.replace(' ','').lower() ]
    except KeyError:
        self.fatal(error_msg + "\"netXType = %s\"" % netXType)
    #4th and 5th letters
    try:
        out += get_protocol_id (PrimaryProtocol)
    except KeyError:
        self.fatal(error_msg + "\"PrimaryProtocol = %s\"" % PrimaryProtocol)
    #6th and 7th letters
    try:
        out += get_protocol_id (SecondaryProtocol)
    except KeyError:
        self.fatal(error_msg + "\"SecondaryProtocol = %s\"" % SecondaryProtocol)
    #8th letter and firmware extention
    out += str(HWCompInd) + file_extention

    return out
