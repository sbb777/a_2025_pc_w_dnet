#! /usr/bin/env python
# vim:fileencoding=utf-8:sw=4:tw=4:et
########################################################################################
# Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.
########################################################################################
# $Id: hilscher_firmware_netx90.py 1140 2025-04-07 12:18:24Z AMesser $:
#
# netX90 application side firmware file generation
########################################################################################
from waflib import   Task
from waflib.TaskGen  import after_method,feature
import os

from netx_image_generator.builder     import generate_bootheader_checksums,\
                                             generate_commonheader_checksums,\
                                             generate_commonheader_md5_checksum,\
                                             generate_commonheader_headercrc32_checksum, \
                                             generate_bootheader_headerchecksum,\
                                             make_array32, array_to_bytes, update_hash

@feature('nxi', 'mxf')
@after_method('build_hboot')
def build_nxi_mxf(self):
    u''' Inject a new task which will patch the hboot image to become a
         nxi/nxe/mxf file.

         This code is currently always active for integrated hboot compiler and
         external hboot compiler'''
    hboot_task_nxi = self.hboot_task_nxi
    hboot_task_nxe = getattr(self, "hboot_task_nxe", None)

    # inject new name for intermediate hboot name
    target_nxi     = hboot_task_nxi.outputs[0]
    hboot_node_nxi = target_nxi.parent.find_or_declare('_%s.hboot' % target_nxi.name)
    hboot_task_nxi.outputs = [hboot_node_nxi]

    if hboot_task_nxe is not None:
        target_nxe     = hboot_task_nxe.outputs[0]
        hboot_node_nxe = target_nxe.parent.find_or_declare('_%s.hboot' % target_nxe.name)
        hboot_task_nxe.outputs = [hboot_node_nxe]

        self.nxi_task = self.create_task('generate_nxi_nxe',
            [hboot_node_nxi, hboot_node_nxe], [target_nxi, target_nxe])
    else:
        self.nxi_task = self.create_task('generate_nxi',
            [hboot_node_nxi], [target_nxi])


class HilscherTask(Task.Task):
    def unlink_if_exists(self, node_or_path):
        if not isinstance(node_or_path, str):
            node_or_path = node_or_path.abspath()

        if os.path.exists(node_or_path):
            os.unlink(node_or_path)

    def generate_commoncrc(self, filedata_iflash, filedata_eflash, offset_header_iflash = 0, offset_header_eflash = 0):
        import zlib

        offset_commonheader_md5       = (64 + 20) // 4
        offset_commonheader_commoncrc = (64 + 48) // 4

        offset_header_iflash_words = offset_header_iflash // 4
        offset_header_eflash_words = offset_header_eflash // 4

        concat_md5 = filedata_iflash[offset_header_iflash_words+offset_commonheader_md5:offset_header_iflash_words+offset_commonheader_md5+4] +\
                     filedata_eflash[offset_header_eflash_words+offset_commonheader_md5:offset_header_eflash_words+offset_commonheader_md5+4]


        # Calculate common header common CRC32
        commoncrc = zlib.crc32(array_to_bytes(concat_md5)) & 0xffffffff

        filedata_iflash[offset_header_iflash_words + offset_commonheader_commoncrc] = commoncrc
        filedata_eflash[offset_header_eflash_words + offset_commonheader_commoncrc] = commoncrc


class generate_nxi(HilscherTask):
    u''' Generate NXI file from hboot image'''
    color     = 'PINK'
    inst_to   = None
    cmdline   = None

    def get_nxi_hboot_headers(self, firmware_data):
        import struct

        boot_header = struct.unpack('<16L', firmware_data[0:64])

        # validate hboot content in header area
        if boot_header[0]  != 0x50494b53:
            raise ValueError(u'First chunk of hboot image for nxi file must be a skip chunk')

        if boot_header[1]  == 0x0000000B:
            # we expect a data chunk
            if boot_header[13] != 0x41544144:
                raise ValueError(u'Second chunk of hboot image for nxi file must be a data or XIP chunk')

            #extract max header data length from hboot chunk
            max_header_data_length = 64 + (boot_header[14]-1) * 4
        elif boot_header[1]  == 0x0000000C:
            # we expect a text chunk
            if boot_header[14] != 0x54584554:
                raise ValueError(u'Second chunk of hboot image for nxi file must be a data or XIP chunk')

            #extract max header data length from hboot chunk
            max_header_data_length = 64 + (boot_header[15]) * 4
        else:
            raise ValueError(u'Bad length of first chunk')

        common_header = struct.unpack('<%dL' % ((max_header_data_length - 64)/4),
                                      firmware_data[64:max_header_data_length])

        if common_header[0] != 0x00030000:
            raise ValueError(u'Unexpected header version of common header')

        if common_header[1] >= max_header_data_length:
            raise ValueError(u'Unexpected header length of common header')

        return boot_header, common_header

    def patch_nxi_header(self, boot_header, common_header, length_nxi):
        import struct

        header_length = (len(boot_header) + len(common_header)) * 4

        HIL_FILE_HEADER_NXI_COOKIE, = struct.unpack('<L', b'.NXI')
        HIL_FILE_HEADER_MXF_COOKIE, = struct.unpack('<L', b'.MXF')

        if "mxf" in self.generator.features:
            boot_header[0]    = HIL_FILE_HEADER_MXF_COOKIE
        else:
            boot_header[0]    = HIL_FILE_HEADER_NXI_COOKIE

        boot_header[1:16] = [0] * 15
        boot_header[4]    = int((length_nxi - 64) / 4)
        boot_header[6]    = struct.unpack('<L', b'NETX')[0]

        common_header[2]  = int(length_nxi - header_length)
        common_header[3]  = int(header_length)

    def dwords_to_bytes(self, dwords):
        import struct
        return struct.pack('<%dL' % len(dwords), *(dwords))

    def run(self):
        import struct
        tgen = self.generator
        netx_type = tgen.netx_type

        inputfile   = self.inputs[0].get_bld().abspath()
        outputfile  = self.outputs[0].get_bld().abspath()

        self.unlink_if_exists(outputfile)

        with open(inputfile, 'rb') as fh:
            firmware_data = fh.read()

        boot_header, common_header = self.get_nxi_hboot_headers(firmware_data)

        # patch binary hboot image to contain valid header structures
        boot_header = list(boot_header)
        common_header = list(common_header)

        self.patch_nxi_header(boot_header, common_header, len(firmware_data))

        header = self.dwords_to_bytes(boot_header + common_header)
        filedata = make_array32(header + firmware_data[len(header):])

        generate_commonheader_checksums(filedata)
        generate_bootheader_checksums(filedata)

        with open(outputfile, 'wb') as fh:
            fh.write(array_to_bytes(filedata))

class generate_nxi_nxe(generate_nxi):
    u'''Generate NXI/NXE file from hboot image'''
    color     = 'PINK'
    inst_to   = None
    cmdline   = None

    def patch_nxe_header(self, boot_header, common_header, length_nxe):
        import struct

        self.patch_nxi_header(boot_header, common_header, length_nxe)

        HIL_FILE_HEADER_NXE_COOKIE, = struct.unpack('<L', b".NXE")

        boot_header[0]    = HIL_FILE_HEADER_NXE_COOKIE

        # clear taglist information
        common_header[36 // 4] = 0
        common_header[40 // 4] = 0
        common_header[44 // 4] = 0

    header_length_nxe = 384

    def check_hboot_nxe(self, hboot_data_nxe):
        import struct

        hboot_header      = struct.unpack_from('<8L', hboot_data_nxe)

        # print list("0x{:08x}".format(x) for x in hboot_header)

        ulFlashOffsetBytes = hboot_header[2]
        ulFlashSelection   = hboot_header[5]

        if (ulFlashSelection & 0xFFFFFF00 ) != 0x00000100:
            raise ValueError( (u'Hboot image for nxe does not look like external SQI flash hboot image:\n'
                               u'Bad value 0x{:08x} of field ulFlashSelection.\n'
                               u'Please check NXE hboot xml to have \'<HBootImage ...  >\n'
                               u'                                       <Header set_flasher_parameters="true" />\n'
                               u'                                       <Chunks>\n'
                               u'                                       ...\'\n'
                              ).format(ulFlashSelection))

        if ulFlashOffsetBytes != self.header_length_nxe:
            raise ValueError( (u'Invalid hboot offset 0x{:x} found.\n'
                               u'Please check NXE hboot xml to have \'<HBootImage ... offset="0x{:x}" >\''
                              ).format(ulFlashOffsetBytes, self.header_length_nxe))


    def run(self):
        import struct
        tgen = self.generator
        netx_type = tgen.netx_type

        inputfile_nxi   = self.inputs[0].get_bld().abspath()
        outputfile_nxi  = self.outputs[0].get_bld().abspath()

        inputfile_nxe   = self.inputs[1].get_bld().abspath()
        outputfile_nxe  = self.outputs[1].get_bld().abspath()

        self.unlink_if_exists(outputfile_nxi)
        self.unlink_if_exists(outputfile_nxe)

        with open(inputfile_nxi, 'rb') as fh:
            firmware_data_nxi = fh.read()

        with open(inputfile_nxe, 'rb') as fh:
            hboot_data_nxe    = fh.read()

        boot_header, common_header = self.get_nxi_hboot_headers(firmware_data_nxi)

        # patch binary hboot image to contain valid header structures
        boot_header   = list(boot_header)
        common_header = list(common_header)

        self.patch_nxi_header(boot_header, common_header, len(firmware_data_nxi))

        header_nxi   = self.dwords_to_bytes(boot_header + common_header)
        filedata_nxi = make_array32(header_nxi + firmware_data_nxi[len(header_nxi):])

        header_nxe   = self.dwords_to_bytes(boot_header + common_header)

        self.check_hboot_nxe(hboot_data_nxe)

        # according to definition, header length is fixed
        common_header = common_header[0:(self.header_length_nxe-64) // 4]

        self.patch_nxe_header(boot_header, common_header, self.header_length_nxe + len(hboot_data_nxe))

        header_nxe   = self.dwords_to_bytes(boot_header + common_header)
        filedata_nxe = make_array32(header_nxe + hboot_data_nxe)

        generate_commonheader_md5_checksum(filedata_nxi)
        generate_commonheader_md5_checksum(filedata_nxe)

        self.generate_commoncrc(filedata_nxi, filedata_nxe)

        generate_commonheader_headercrc32_checksum(filedata_nxi)
        generate_commonheader_headercrc32_checksum(filedata_nxe)

        generate_bootheader_checksums(filedata_nxi)
        generate_bootheader_checksums(filedata_nxe)

        with open(outputfile_nxi, 'wb') as fh_nxi:
            with open(outputfile_nxe, 'wb') as fh_nxe:
                fh_nxi.write(array_to_bytes(filedata_nxi))
                fh_nxe.write(array_to_bytes(filedata_nxe))

def generate_netx90_bootheader_checksums(filedata_iflash, filedata_eflash = None):
    u''' Update the checksum in the netx90 app side bootheader of nai image'''
    import hashlib

    offset_boot_header       = 448
    offset_boot_header_words = offset_boot_header // 4

    h = hashlib.sha384()

    h.update(array_to_bytes(filedata_iflash[0:112]))
    h.update(array_to_bytes(filedata_iflash[128:]))

    if filedata_eflash is not None:
        h.update(array_to_bytes(filedata_eflash))

    hash_value = make_array32(h.digest())

    # Write the first 7 DWORDs of the hash to the HBOOT header.
    filedata_iflash[offset_boot_header_words + 8: offset_boot_header_words + 15] = hash_value[0:7]

    generate_bootheader_headerchecksum(filedata_iflash, offset_boot_header)

class generate_nai(HilscherTask):
    ''' Generate NAI file from hboot image'''
    color     = 'PINK'
    inst_to   = None
    cmdline   = None

    @property
    def targets_pretty(self):
        return ", ".join(x.name for x in self.outputs)

    nai_offset_fh3_boot_header   = 512
    nai_offset_fh3_common_header = nai_offset_fh3_boot_header + 64

    def get_nai_file_headers(self, firmware_data):
        import struct

        default_header   = make_array32(firmware_data[self.nai_offset_fh3_boot_header:self.nai_offset_fh3_boot_header+64])

        HIL_FILE_HEADER_NAI_COOKIE, = struct.unpack('<L', b'.NAI')
        HIL_FILE_HEADER_NETX_SIGNATURE, = struct.unpack('<L', b'NETX')

        if default_header[0] != HIL_FILE_HEADER_NAI_COOKIE:
            raise ValueError(u'Unexpected cookie in default header for NAI image when building %s' % self.targets_pretty)

        if default_header[6] != HIL_FILE_HEADER_NETX_SIGNATURE:
            raise ValueError(u'Unexpected signature in default header for NAI image when building %s' % self.targets_pretty)

        common_header = make_array32( firmware_data[self.nai_offset_fh3_common_header:self.nai_offset_fh3_common_header+64])

        if common_header[0] != 0x00030000:
            raise ValueError(u'Unexpected header version of common header for NAI when building %s' % self.targets_pretty)

        if common_header[3] != 704:
            raise ValueError(u'Unexpected value %d in field ulDataStartOffset of common file header for NAE when building %s. (Should be 704)' % (common_header[3], self.targets_pretty))

        return default_header, common_header

    def patch_nai_header(self, default_header, common_header, length_nai):
        default_header[4]  = (length_nai - self.nai_offset_fh3_common_header) // 4
        # Set FHv3 Common Header ulDataSize field according file size & ulDataStartOffset
        common_header[2]   = (length_nai - common_header[3])

    def run(self):
        inputfile   = self.inputs[0].get_bld().abspath()
        outputfile  = self.outputs[0].get_bld().abspath()

        self.unlink_if_exists(outputfile)

        with open(inputfile, 'rb') as fh:
            firmware_data = fh.read()

        default_header, common_header = self.get_nai_file_headers(firmware_data)

        # patch binary hboot image to contain valid header structures
        self.patch_nai_header(default_header, common_header, len(firmware_data))

        header = make_array32(firmware_data[0:512]) + default_header + common_header

        filedata = header + make_array32(firmware_data[(len(header) * 4):])

        generate_commonheader_checksums(filedata, header_offset = 512)
        generate_bootheader_checksums(filedata  , header_offset = 512)
        generate_netx90_bootheader_checksums(filedata)

        with open(outputfile, 'wb') as fh:
            fh.write(array_to_bytes(filedata))

class generate_nai_nae(generate_nai):
    u''' Generate NAI/NAE file from app image'''
    color     = 'PINK'
    inst_to   = None
    cmdline   = None

    nae_offset_fh3_boot_header   = 64
    nae_offset_fh3_common_header = nae_offset_fh3_boot_header + 64

    def get_nae_file_headers(self, firmware_data):
        import struct

        default_header   = make_array32( firmware_data[self.nae_offset_fh3_boot_header:self.nae_offset_fh3_boot_header+64])

        HIL_FILE_HEADER_NAE_COOKIE,     = struct.unpack('<L', b'.NAE')
        HIL_FILE_HEADER_NETX_SIGNATURE, = struct.unpack('<L', b'NETX')

        if default_header[0] != HIL_FILE_HEADER_NAE_COOKIE:
            raise ValueError(u'Unexpected cookie in default header for NAE image when building %s' % self.targets_pretty)

        if default_header[6] != HIL_FILE_HEADER_NETX_SIGNATURE:
            raise ValueError(u'Unexpected signature in default header for NAE image when building %s' % self.targets_pretty)

        common_header = make_array32(firmware_data[self.nae_offset_fh3_common_header:self.nae_offset_fh3_common_header+64])

        if common_header[0] != 0x00030000:
            raise ValueError(u'Unexpected header version of common header for NAE image when building %s' % self.targets_pretty)

        if common_header[3] != 256:
            raise ValueError(u'Unexpected value %d in field ulDataStartOffset of common file header for NAE image when building %s. (Should be 256)' % (common_header[3], self.targets_pretty))

        return default_header, common_header

    def patch_nae_header(self, default_header, common_header, length_nae):
        default_header[4]  = (length_nae - self.nae_offset_fh3_common_header) // 4
        # Set FHv3 Common Header ulDataSize field according file size & ulDataStartOffset
        common_header[2]   = (length_nae - common_header[3])

        if common_header[2] < 1:
            raise ValueError(u'%s: MFW will refuse to flash NAE file with zero application length' % self.generator.name)

    def run(self):
        inputfile_nai_image  = self.inputs[0].get_bld().abspath()
        outputfile_nai       = self.outputs[0].get_bld().abspath()

        inputfile_nae_image  = self.inputs[1].get_bld().abspath()
        outputfile_nae       = self.outputs[1].get_bld().abspath()

        self.unlink_if_exists(outputfile_nai)
        self.unlink_if_exists(outputfile_nae)

        with open(inputfile_nai_image, 'rb') as fh:
            nai_data = fh.read()

        with open(inputfile_nae_image, 'rb') as fh:
            nae_data = fh.read()

        default_header_nai, common_header_nai = self.get_nai_file_headers(nai_data)
        default_header_nae, common_header_nae = self.get_nae_file_headers(nae_data)

        # patch binary hboot image to contain valid header structures
        self.patch_nai_header(default_header_nai, common_header_nai, len(nai_data))
        self.patch_nae_header(default_header_nae, common_header_nae, len(nae_data))

        header_nai   = make_array32(nai_data[0:512]) + default_header_nai + common_header_nai
        filedata_nai = header_nai + make_array32(nai_data[(len(header_nai) * 4):])

        header_nae   = make_array32(nae_data[0:64])  + default_header_nae + common_header_nae
        filedata_nae = header_nae + make_array32(nae_data[(len(header_nae) * 4):])

        generate_commonheader_md5_checksum(filedata_nai, header_offset = 512)
        generate_commonheader_md5_checksum(filedata_nae, header_offset =  64)

        self.generate_commoncrc(filedata_nai, filedata_nae, 512, 64)

        generate_commonheader_headercrc32_checksum(filedata_nai, header_offset = 512)
        generate_commonheader_headercrc32_checksum(filedata_nae, header_offset =  64)

        generate_bootheader_checksums(filedata_nai, header_offset = 512)
        generate_bootheader_checksums(filedata_nae, header_offset =  64)

        generate_netx90_bootheader_checksums(filedata_nai, filedata_nae)

        with open(outputfile_nai, 'wb') as fh_nai:
            with open(outputfile_nae, 'wb') as fh_nae:
                fh_nai.write(array_to_bytes(filedata_nai))
                fh_nae.write(array_to_bytes(filedata_nae))
