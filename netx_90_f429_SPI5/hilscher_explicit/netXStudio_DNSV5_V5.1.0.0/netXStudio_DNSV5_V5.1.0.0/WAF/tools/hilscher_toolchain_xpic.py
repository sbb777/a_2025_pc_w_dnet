# vim:fileencoding=utf-8:sw=4:tw=4:et
########################################################################################
# Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.
########################################################################################
# $Id: hilscher_toolchain_xpic.py 848 2023-10-02 07:12:39Z AMesser $:
#
# Provides tasks and functions for use with object copy
########################################################################################
from waflib.Configure import conf
import waflib.Task
import waflib.TaskGen
import waflib.Configure
import struct
import textwrap
import os

def configure(conf):
    env = conf.declare_toolchain('llvm-xpic')
    env['TOOLCHAIN_TRIPLE'] = 'xpic-hilscher-elf'

    conf.declare_platform('xpic', 'llvm-xpic',  setup_device_xpic, target_triple_xpic)
    conf.declare_platform('xpic2', 'llvm-xpic', setup_device_xpic, target_triple_xpic)

    conf.load('objcopy hilscher_gencode hilscher_toolchain_llvm')

    path = None

    if 'PATH_LLVM_XPIC' in os.environ:
        # new version llvm
        path = [os.path.join(os.environ['PATH_LLVM_XPIC'], 'bin')]
    else:
        if 'PATH_GNU_XPIC' in os.environ:
            # old version llvm
            path = [os.path.join(os.environ['PATH_GNU_XPIC'], 'bin')]

    with conf.ToolchainDetectContext():
        conf.setup_llvm_toolchain(target='xpic', path_list=path)

        # overide compiler name with xpic specific name
        env['CC_NAME']    = 'xpic-llvm'
        env['CXX_NAME']   = 'xpic-llvm'

        env['cprogram_PATTERN']   = '%s.elf'
        env['cxxprogram_PATTERN'] = '%s.elf'

        try:
            env['CC_VERSION'] = conf.get_xpic_llvm_version(path_list=path)
        except:
            conf.msg('Extracting xpic backend version', result = 'fail', color = 'YELLOW')

        f = env.append_value

        # Add dwarf2 debugging symbols
        f('CFLAGS',   ['-gdwarf-2'])
        f('CXXFLAGS', ['-gdwarf-2'])

        f('DEFINES', ['__XPIC__'])

        conf.xpic_optimize_flags()

        # fix wrong directories in xpic compiler
        libdir_list = conf.cmd_and_log(conf.env['LLVM_CONFIG'] + ["--libdir"]).splitlines()
        f('LIBPATH',  [x+"/gcc/xpic-hilscher-elf/0.1.1" for x in libdir_list])

        f('STLIB_default_standardlibs',   ['m', 'c', 'gcc'])

@conf
def target_triple_xpic(bld, platform):
    return platform + '-' + bld.env['TOOLCHAIN_TRIPLE'].partition('-')[2]

@conf
def setup_device_xpic(bld):
    mcu = bld.env['PLATFORM']

    for f in 'CFLAGS CXXFLAGS ASFLAGS'.split():
        bld.env.prepend_value(f,  ['-Wa,-mmcu=%s' % mcu])

    bld.env.prepend_value('LINKFLAGS', '-mcpu=%s' % mcu)

@conf
def xpic_optimize_flags(conf):
    f = conf.env.append_value

    if conf.env.CONDITIONS == 'debug':
        f('CFLAGS__optimize',   ['-O0'])
        f('CXXFLAGS__optimize', ['-O0'])
    else:
        f('CFLAGS__optimize',   ['-Os'])
        f('CXXFLAGS__optimize', ['-Os'])
        f('DEFINES__optimize',  ['NDEBUG=1'])

@conf
def get_xpic_llvm_version(conf, path_list=None):
    env = conf.env

    if not env['LLC']:
        llc = conf.find_program('llc', var='llc', path_list=path_list)
        env['LLC'] = conf.cmd_to_list(llc)

    # we try to get the xpic llvm version running 'bin/llc.exec --version'
    # when ran with '--version' attribute the llc.exe behaves unexpectedly
    # and exits with error code '1', interrupting the build process;
    # to avoid that, we enclose the 'cmd_to_log' command in a try/catch block
    # and parse the xpic llvm version from the exception message
    # which contains the actual process output

    try:
        out = conf.cmd_and_log(env.LLC  + ["--version"])
    except Exception as e:
        out = e.stdout

    version = out.split('version: V')[1].split('Git')[0].strip().split('.')

    return version

class IHexReader(object):
    def __init__(self, node):
        self.path = node.abspath()

    def __iter__(self):
        with open(self.path, "rt") as f:
            for nr, line in enumerate(f):
                _, _, line = line.strip().partition(':')

                if (sum(int(line[x:x+2],16) for x in range(0, len(line), 2)) % 256) != 0:
                    raise ValueError('Bad checksum in %s line %d' % (self.path, nr))

                l    = int(line[0:2], 16)
                addr = int(line[2:6], 16)
                typ  = int(line[6:8], 16)
                data = line[8:-2].decode('hex')

                if len(data) != l:
                    raise ValueError('Bad length %u in %s line %d' % (l, self.path, nr))

                yield addr,typ,data

class xpic_to_c(waflib.Task.Task):
    u''' Convert xpic ihex file to  .c / .h file '''
    color   = 'PINK'
    log_str = '[XPIC2C] $SOURCES $TARGETS'


    template_c = textwrap.dedent('''
        #include "{include_name}"

        uint32_t {name}_dram[{dram_cnt}] = {{
          {dram_data}
        }};

        uint32_t {name}_pram[{pram_cnt}] = {{
          {pram_data}
        }};
    ''')

    template_h = textwrap.dedent('''
        #ifndef {name}_H_
        #define {name}_H_

        #include <stdint.h>

        extern uint32_t {name}_dram[{dram_cnt}];
        extern uint32_t {name}_pram[{pram_cnt}];

        #endif
    ''')

    def run(self):
        ihex_node, = self.inputs
        c_node, h_node = self.outputs

        c_node.delete()
        h_node.delete();

        base_addr = 0
        dram_data = b''
        pram_data = b''

        for addr, typ, data in IHexReader(ihex_node):
            if typ == 0:
                #print('0x%08X: %s' % (base_addr + addr, data.encode('hex')))
                addr = base_addr + addr

                if (addr >= 0xFF880000) and (addr < 0xFF882000):
                    # data for dram segment
                    offset = addr - 0xFF880000

                    assert((addr + len(data)) <= 0xFF882000)

                    if len(dram_data) < offset:
                        dram_data = dram_data + (b'\0' * (offset - len(dram_data)))

                    dram_data = dram_data[0:offset] + data + dram_data[offset + len(data):]

                elif (addr >= 0xFF882000) and (addr < 0xFF884000):
                    # data for pram segment
                    offset = addr - 0xFF882000

                    assert((addr + len(data)) <= 0xFF884000)

                    if len(pram_data) < offset:
                        pram_data = pram_data + (b'\0' * (offset - len(pram_data)))

                    pram_data = pram_data[0:offset] + data + pram_data[offset + len(data):]

            elif typ == 1:
                break
            elif typ == 2:
                base_addr = struct.unpack('>H',data)[0] * 16
            elif typ == 4:
                base_addr = struct.unpack('>H',data)[0] << 16
            else:
                raise ValueError('Unsupported type %d' % typ)

        def convert_to_uint32_values(data):
            uint32_cnt = len(data) / 4
            return ("0x%08X" % x for x in struct.unpack('<%uL' % uint32_cnt, data))

        def group_by_eight(data):
            stack = []
            for val in convert_to_uint32_values(data):
                stack.append(val)

                if len(stack) >= 8:
                    yield ",".join(stack)
                    stack = []

            if len(stack):
                yield ",".join(stack)

        def format_data(data):
            return ",\n  ".join(group_by_eight(data))

        vars = dict(
            name = self.inputs[0].name.rpartition('.')[0],
            dram_cnt  = len(dram_data) / 4,
            dram_data = format_data(dram_data),
            pram_cnt  = len(pram_data) / 4,
            pram_data = format_data(pram_data),

            include_name = h_node.name
        )

        c_node.write(self.template_c.format(**vars))
        h_node.write(self.template_h.format(**vars))


@waflib.TaskGen.feature("xpic_program")
@waflib.TaskGen.after_method("apply_link")
def extract_xpic_binaries(self):
    link_task = getattr(self, "link_task", None)

    if link_task:
        elf_node = self.link_task.outputs[0]

        tsk = self.create_objectcopy_task(elf_node , elf_node.change_ext('.ihex'), "ihex")

        c_node = elf_node.change_ext(".c")
        h_node = elf_node.change_ext(".h")

        self.create_task("xpic_to_c", tsk.outputs[0], [c_node, h_node])

        # register with source code management
        self.generated_source(c_node)
        self.generated_header(h_node)

@waflib.Configure.conf
def xpic_program(self, *args, **kw):
    features = waflib.Utils.to_list(kw.pop('features',[]))
    self.program( *args, features = features + ["xpic_program"], **kw)

