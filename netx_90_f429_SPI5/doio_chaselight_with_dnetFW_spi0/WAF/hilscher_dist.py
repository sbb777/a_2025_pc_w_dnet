#! /usr/bin/env python
# encoding: utf-8
########################################################################################
# Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.
########################################################################################
# $Id: hilscher_dist.py 883 2023-10-24 07:37:36Z AMesser $:
#
# Description:
# Distribution generation functions
########################################################################################
import waflib.Task
import waflib.TaskGen
import waflib.Build
from waflib import Node, Logs, Utils
from waflib.Configure import conf
from waflib.TaskGen import feature, after_method, before_method, taskgen_method
import os, string
import time

template_wscript_header = string.Template('''\
def configure(conf):
    ${EXTRA_CONFIGURE}
    pass

def build(bld):
''')

template_wscript_sdk = string.Template('''\
    bld(
        name            = '${NAME}',
        description     = '${DESCRIPTION}',
        displaygroup    = '${DISPLAYGROUP}',
        version         = '${VERSION}',

        SDK             = True,
        export_includes = [${INCLUDE_PATHS}],
        export_defines  = [${EXPORT_DEFINES}],
    )
''')

template_wscript_extlib = string.Template('''\
    bld.externalcomponent(
        name            = '${NAME}',
        description     = '${DESCRIPTION}',
        displaygroup    = '${DISPLAYGROUP}',
        version         = '${VERSION}',

        target          = '${TARGET}',
        path            = '${PATH}',
        export_includes = [${INCLUDE_PATHS}],
        export_defines  = [${EXPORT_DEFINES}],
        uselib          = [${USELIB}],
    )
''')

class strip(waflib.Task.Task):
    u''' Strip sections from a file '''
    color   = 'PINK'
    log_str = '[STRIP] $SOURCES $TARGETS'
    run_str = '${STRIP} -o ${TGT} ${STRIPFLAGS} ${SRC}'


@waflib.TaskGen.taskgen_method
def create_strip_debug_task(self, tgen, source, target):
    ''' Creates a task which will strip debug information from a compiled file '''
    tsk = tgen.create_task('strip', source, target)
    tsk.env.STRIPFLAGS = ['--strip-debug']
    return tsk

@conf
def distribute_lib(bld, install_path, use , **kw):
    if not getattr(bld,'is_install', False):
        return

    kw.setdefault('wscript_name', 'wscript')
    kw.setdefault('dist_includes', [])

    # mandatory parameters:
    kw['use']          = Utils.to_list(use)[:]
    kw['install_path'] = install_path
    kw['target']       = install_path + '/' + kw['wscript_name']
    kw['name']         = kw['target']

    bld.check_args(**kw)

    use_ltd =  Utils.to_list(kw.get('use_ltd', []))[:]
    use_usedlibs = Utils.to_list(use)[:] + use_ltd

    # generate distribution folder
    bld(features = 'distribute distribute_lib',
        hidden_from_list = "Internal",
        **kw)

    # generate libsused file
    bld(name     = '%s/libsused' % install_path,
        target   = 'libsused.txt',
        features = 'libsused_explicit',
        use      = use_usedlibs,
        include_SDK = True,
        hidden_from_list = "Internal",
        install_path = '%s/ReleaseNotes/' % install_path)

@conf
def distribute_firmware(bld, install_path, use , **kw):
    if not getattr(bld,'is_install', False):
        return

    kw.setdefault('dist_includes', [])

    # mandatory parameters:
    kw['use']          = use[:]
    kw['install_path'] = install_path
    kw['name']         = install_path

    bld.check_args(**kw)

    use_ltd =  kw.get('usr_ltd', [])[:]
    use_usedlibs = list(x for x,p in (use + use_ltd))

    # generate distribution folder
    bld(features = 'distribute distribute_firmware',
        hidden_from_list = "Internal",
        **kw)

    # generate libsused file
    bld(name     = '%s/libsused' % install_path,
        target   = 'libsused.txt',
        features = 'libsused_explicit',
        use      = use_usedlibs,
        hidden_from_list = "Internal",
        install_path = '%s/ReleaseNotes/' % install_path)

@conf
def distribute_doc(bld, install_path, use_doc , **kw):
    if not getattr(bld,'is_install', False):
        return

    kw.setdefault('dist_includes', [])

    # mandatory parameters:
    kw['use_doc']      = use_doc[:]
    kw['install_path'] = install_path
    kw['name']         = install_path

    bld.check_args(**kw)

    # Do not use Documentation subfolder by default
    kw.setdefault('doc_folder_name', '')

    # generate distribution folder
    bld(features = 'distribute',
        hidden_from_list = "Internal",
        **kw)

@conf
def distribute_source(bld, install_path, use_source , **kw):
    if not getattr(bld,'is_install', False):
        return

    kw.setdefault('dist_includes', [])

    # mandatory parameters:
    kw['use_source']   = use_source[:]
    kw['install_path'] = install_path
    kw['name']         = install_path

    bld.check_args(**kw)

    # generate distribution folder
    bld(features = 'distribute_source',
        hidden_from_list = "Internal",
        **kw)
@conf
def distribute_debug(bld, install_path, use_source , **kw):
    if not getattr(bld,'is_install', False):
        return

    kw.setdefault('dist_includes', [])

    # mandatory parameters:
    kw['use_source']   = use_source[:]
    kw['install_path'] = install_path
    kw['name']         = install_path

    bld.check_args(**kw)

    features = ['distribute_source']

    if 'use_debug' in kw:
        features.append('distribute_debug')

    # generate distribution folder
    bld(features = features, hidden_from_list = "Internal", **kw)

@feature('distribute', 'distribute_lib', 'distribute_firmware', 'distribute_source')
def set_default_dist_folders(self):
    ''' Configure default values for all subfolder names if not provided '''
    dct = self.__dict__

    dct.setdefault('doc_folder_name',          'Documentation')
    dct.setdefault('include_folder_name',      'Includes')
    dct.setdefault('lib_folder_name',          'Lib')
    dct.setdefault('lib_ltd_folder_name',      'Lib_Ltd')
    dct.setdefault('firmware_folder_name',     'Firmware')
    dct.setdefault('firmware_ltd_folder_name', 'Firmware_Ltd')
    dct.setdefault('source_archive_name',      'Source.zip')
    dct.setdefault('debug_archive_name',       'Debug.zip')

known_flags = set(Utils.to_list('no_debug_info'))

@taskgen_method
def dist_iterate_use(self, use):
    for x in use:
        subdir_path = None
        flags = ""

        if isinstance(x,(tuple,list)):
            try:
                name, subdir_path, flags = tuple(x)
            except ValueError:
                name, subdir_path = tuple(x)
        else:
            name = x

        flags = Utils.to_list(flags)
        tgen = self.bld.get_tgen_by_name(name)

        if not getattr(tgen, 'posted', None):
            tgen.post()

        unknown_flags = set(flags) - known_flags
        if unknown_flags:
            self.wscript_error('Unkown dist flags %r' % (tuple(unknown_flags)))

        yield x, name, subdir_path, tgen, flags

@taskgen_method
def add_dist_nodes(self, nodes):
    try:
        iter(nodes)
    except TypeError:
        nodes = [nodes]

    dist_nodes = getattr(self, 'dist_nodes', [])
    dist_nodes.extend(nodes)
    self.dist_nodes = dist_nodes

@feature('cprogram', 'cxxprogram')
@before_method('dist_binary')
def distribute_program(self):
    self.add_dist_nodes(self.link_task.outputs[0])

@feature('distribute_lib', 'distribute_firmware')
@after_method('set_default_dist_folders')
@before_method('check_tgen_availability')
def dist_binary(self):
    bld = self.bld

    install_path = Utils.split_path(self.install_path)

    include_dist_paths   = set()

    tsk_debug_archive = None

    if 'distribute_lib' in self.features:
        name = 'generated_' + '_' .join(Utils.split_path(self.install_path) + [self.wscript_name])

        wscript_node = self.path.find_or_declare(name)
        tsk = self.wscript_task = self.create_task('distribute_generate_wscript', [], [wscript_node])
        tsk.install_dirs  = {}
        tsk.generators     = []

        bld.install_as('/'.join(install_path + [self.wscript_name]), wscript_node)
        tsk.install_dirs[wscript_node] = '/'.join(install_path)

        include_dist_paths.add(tuple(install_path[:]))

    install_debug_path = getattr(self, 'install_debug_path', None)

    if install_debug_path:
        name = 'generated_' + '_' .join(Utils.split_path(self.install_path) + Utils.split_path(self.install_debug_path))

        debug_archive_node = self.path.find_or_declare(name)

        tsk_debug_archive  = self.debug_archive_task = self.create_task('distribute_generate_debug', [], [debug_archive_node])
        tsk_debug_archive.install_paths = {}

        bld.install_as(install_debug_path, debug_archive_node)

    use     = self.to_list(self.use)
    use_ltd = self.to_list(getattr(self,'use_ltd', []))

    include_search_paths = []

    for x, name, subdir_path, tgen, flags in self.dist_iterate_use(use + use_ltd):
        ltd = x in use_ltd

        tgen_disabled = getattr(tgen,'tgen_disabled', None)

        if tgen_disabled:
            bld.fatal(u'Target %r not available for distribution: %r' % (tgen.name, tgen_disabled))

        # build list of used task generators
        include_tgen     = [tgen]

        for y in Utils.to_list(getattr(tgen,'use',[])):
            include_tgen.append(self.bld.get_tgen_by_name(y))

        # additional nodes to distribute for debug purposes
        dist_debug_nodes = []

        for y in include_tgen:
            # collect include search paths
            include_search_paths.extend(getattr(y, "include_nodes", [])[:])

            # collect exported include search paths
            for z in getattr(y, "export_includes", []):
                include_search_paths.append(y.path.find_node(z))

            # collect debug nodes for distribution
            dist_debug_nodes.extend(getattr(y, 'dist_debug_nodes', []))

        parts    = tgen.name.split('/')

        # get nodes to distribute
        dist_nodes   = getattr(tgen,'dist_nodes', [])
        # init list of destination file names
        dist_targets = dict( ((x, x.name) for x in dist_nodes))

        if getattr(tgen,'SDK', False):
            dst_subdir_parts       = None
        elif set(tgen.features) & set(['fake_lib', 'shlib', 'stlib', 'cshlib', 'cstlib']):
            link_task = tgen.link_task
            lib_node = link_task.outputs[0]

            if 'no_debug_info' in flags:
                _, ext = os.path.splitext(lib_node.name)
                lib_node_stripped = lib_node.change_ext('.stripped' + ext)

                dist_nodes = [lib_node_stripped]
                dist_targets[lib_node_stripped] = lib_node.name

                self.create_strip_debug_task(tgen, lib_node, lib_node_stripped)
            elif not dist_nodes:
                dist_nodes.append(lib_node)
                dist_targets[lib_node] = lib_node.name

            if ltd:
                dst_subdir_parts = install_path + [self.lib_ltd_folder_name]
            else:
                dst_subdir_parts = install_path + [self.lib_folder_name]
        else:
            if not hasattr(tgen, 'dist_nodes'):
                Logs.warn('hilscher: trying to distribute tgen %s without dist_nodes' % tgen.name)
                tgen_target_node = tgen.path.find_or_declare(tgen.target)

                dist_nodes.append(tgen_target_node)
                dist_targets[tgen_target_node] = tgen_target_node.name

            if ltd:
                dst_subdir_parts = install_path + [self.firmware_ltd_folder_name]
            else:
                dst_subdir_parts = install_path + [self.firmware_folder_name]

            include_dist_paths.add(tuple(dst_subdir_parts[:]))

        if 'fake_lib' in tgen.features:
            # an external library is to be distributed with this distribution, extract
            # original path from library and reuse it if possible
            prefix = ('%s/%s/' % (parts[0], parts[1])).lower()
            orig_path = '/'.join(Utils.split_path(dist_nodes[0].path_from(tgen.path)))

            i = orig_path.lower().find(prefix)

            if i >= 0:
                p = Utils.split_path(prefix + orig_path[ i + len(prefix):])
            else:
                Logs.warn(u'Unable to extract prefix for %s' % tgen.name)
                p = Utils.split_path(orig_path)

            # Last item in p is the name of the library file itself
            # we just want the prefix path
            dst_subdir_parts.extend(p[0:-1])

        elif len(dist_nodes) > 0:
            if subdir_path is not None:
                if subdir_path.endswith('/') or subdir_path.endswith('\\'):
                    # subdir_path is a directory:
                    dst_subdir_parts.extend(Utils.split_path(subdir_path))
                    dst_name = dist_targets[dist_nodes[0]]
                else:
                    # subdir_path is a file

                    if len(dist_nodes) > 1:
                        bld.fatal(u'Can not distribute multiple targets to single filename for %r' % tgen.name)

                    p = Utils.split_path(subdir_path)

                    dst_subdir_parts.extend(p[0:-1])
                    dst_name = dist_targets[dist_nodes[0]] = p[-1]

                    _, ext = os.path.splitext(dst_name.lower())

                    if ext not in '.nxi .nxf .nxo .rom'.split():
                        Logs.warn(u'Missing file extension for distribution %s. (Check for missing trailing slash sub-path?)' % ('/'.join(p)))

                # also install libsused file if available
                libsused_task = getattr(tgen, 'libsused_task', None)

                if libsused_task is not None:
                    bld.install_as('/'.join( dst_subdir_parts + ['%s_usedlibs.txt' % dst_name]),
                                   libsused_task.outputs[0])

            elif 'distribute_lib' in self.features:
                target_triple         = tgen.env['TARGET_TRIPLE'] or 'unkown-none-unknown'
                toolchain_version = '.'.join(tgen.env['CC_VERSION']) or 'x.x.x'

                if len(parts) not in (3,4):
                    bld.fatal(u'Invalid generator name %s' % tgen.name)

                if parts[0] != target_triple:
                    Logs.warn(u'Unexpected toolchain in generator %s' % tgen.name)

                if parts[1] != toolchain_version:
                    Logs.warn(u'Unexpected toolchain version in generator %s' % tgen.name)

                dst_subdir_parts.extend([target_triple, toolchain_version])

                if len(parts) == 4:
                    os_label = parts[2]
                    dst_subdir_parts.append(os_label)

                platform_label = tgen.platform.lower()

                if platform_label not in ('netx'):
                    dst_subdir_parts.append(platform_label)

            else:
                bld.fatal(u'Unable to determine install path for %s' % tgen.name)

        dst_subdir = '/'.join(dst_subdir_parts or [])

        for node in dist_nodes:
            bld.install_as(dst_subdir + '/' + dist_targets[node], node)

        if 'distribute_lib' in self.features:
            tsk.set_inputs(dist_nodes)

            for x in dist_nodes:
                tsk.install_dirs[x] = '/'.join(dst_subdir_parts)

            tsk.generators.append((tgen, x))

        if tsk_debug_archive:
            for node in dist_debug_nodes:
                tsk_debug_archive.set_inputs(node)
                tsk_debug_archive.install_paths[node] = dst_subdir + '/' + node.name

    for x in self.dist_includes:
        if isinstance(x,(tuple,list)):
            src_path, subdir_path = x
        else:
            src_path, subdir_path = x, None

        for x in include_search_paths:
            include_node = x.find_node(src_path)

            if include_node:
                if subdir_path is None:
                    subdir_path = include_node.parent.path_from(x)
                break
        else:
            bld.fatal(u'Include file %s not found in use/use_ltd targets' % src_path)

        for y in include_dist_paths:
            dst_path_parts = list(y) +  [self.include_folder_name] + Utils.split_path(subdir_path)
            bld.install_files('/'.join(dst_path_parts), [include_node])

@feature('distribute')
@after_method('set_default_dist_folders')
@before_method('check_tgen_availability')
def dist_doc(self):
    bld = self.bld
    use_doc = self.to_list(getattr(self, 'use_doc', []))

    install_path = Utils.split_path(self.install_path)

    for x, name, subdir_path, tgen, flags in self.dist_iterate_use(use_doc):
        dist_nodes = getattr(tgen,'dist_nodes', [])

        dst_subdir_parts = install_path + [self.doc_folder_name]

        # init list of destination file names
        dst_names   = list(x.name for x in dist_nodes)

        if subdir_path is not None:
            if subdir_path.endswith('/') or subdir_path.endswith('\\'):
                # subdir_path is a directory:
                dst_subdir_parts.extend(Utils.split_path(subdir_path))
            else:
                # subdir_path is a file
                if len(dist_nodes) > 1:
                    bld.fatal(u'Can not distribute multiple targets to single filename for %r' % tgen.name)

                p = Utils.split_path(subdir_path)

                dst_subdir_parts.extend(p[0:-1])
                dst_names[0] = p[-1]

        dst_subdir = '/'.join(dst_subdir_parts or [])

        for node, n in zip(dist_nodes, dst_names):
            bld.install_as(dst_subdir + '/' + n, node)

@feature('distribute_source')
@after_method('set_default_dist_folders')
@before_method('check_tgen_availability')
def dist_source(self):
    bld = self.bld

    use_source = self.to_list(self.use_source)

    install_path = Utils.split_path(self.install_path)

    source_nodes = []

    scanned_deps = []

    for x, name, subdir_path, tgen, flags in self.dist_iterate_use(use_source):
        compiled_tasks = getattr(tgen,'compiled_tasks',[])[:]

        for _, _, _, tgen2, _ in self.dist_iterate_use(tgen.use):
            compiled_tasks.extend(getattr(tgen2,'compiled_tasks',[]))

        # Add c-files to zip
        source_nodes.extend(t.inputs[0] for t in compiled_tasks)

        # remind the tasks to scan later for dependencies
        # of the c files
        scanned_deps.append((tgen, compiled_tasks))


    source_paths = dict((n, n.path_from(bld.srcnode)) for n in source_nodes)

    tsk = self.install_zip(self.source_archive_name, source_paths)

    if tsk:
        def scan_deps_late(scanned_deps = scanned_deps):
            ''' Scan for header files after the compiled tasks had been run'''
            source_nodes = []

            for tgen, compiled_tasks in scanned_deps:
                deps = set()

                # Get a list of all dependencies of compiled objects (e.g. header files)
                for x in compiled_tasks:
                    deps.update(set(bld.node_deps[x.uid()]))

                # Add headers in component to zip
                source_nodes.extend([n for n in deps if n.is_child_of(tgen.path)])

            tsk.inputs.extend(source_nodes)

            for n in source_nodes:
                tsk.install_paths[n] = n.path_from(bld.srcnode)

            return (source_nodes, time.time())

        tsk.scan = scan_deps_late

@feature('distribute_debug')
@after_method('set_default_dist_folders')
@before_method('check_tgen_availability')
def dist_debug(self):
    bld = self.bld

    use_debug = self.to_list(self.use_debug)

    debug_nodes = []

    for _, _, _, tgen, _ in self.dist_iterate_use(use_debug):
        debug_tgen     = [tgen]

        for y in Utils.to_list(getattr(tgen,'use',[])):
            debug_tgen.append(bld.get_tgen_by_name(y))

        for y in debug_tgen:
            # collect debug nodes for distribution
            debug_nodes.extend(getattr(y, 'dist_debug_nodes', []))

    debug_paths = dict((n, n.path_from(bld.bldnode)) for n in debug_nodes)

    self.install_zip(self.debug_archive_name, debug_paths)

@taskgen_method
def install_zip(self, dest, srcmap):
    install_path = Utils.split_path(self.install_path)

    ''' Install a zip file generated from provided source mapping '''
    if self.bld.is_install:
        tsk = self.create_task('inst_zip', srcmap.keys(), [])
        tsk.dest = '/'.join(install_path + [dest])
        tsk.install_paths = srcmap
        return tsk

class distribute_generate_wscript(waflib.Task.Task):
    u''' Generate wscript in distribution folder '''
    log_str   = "[WSCRIPT] $TARGET"
    color     = 'PINK'
    inst_to   = None
    cmdline   = None

    def run(self):
        output_node = self.outputs[0]

        output_node.parent.mkdir()

        with open(output_node.abspath(), 'wt') as fh:
            dct = {}

            extra_configure = getattr(self.generator, 'extra_configure', '').splitlines()

            indent = ' ' * 4
            dct['EXTRA_CONFIGURE'] = ('\n' + indent).join(x.lstrip() for x in extra_configure)

            wscript_header = template_wscript_header.substitute(dct)
            fh.write(''.join((x.rstrip() + '\n') for x in wscript_header.splitlines()))
            separator = ''

            remove_prefix = self.install_dirs[output_node] + '/'
            for tgen, node in self.generators:
                dct = {
                    'NAME'           : tgen.name,
                    'DESCRIPTION'    : tgen.description,
                    'DISPLAYGROUP'   : tgen.displaygroup,
                    'VERSION'        : tgen.version,
                    'INCLUDE_PATHS'  : "'%s'" % self.generator.include_folder_name,
                }

                if getattr(tgen, 'SDK', False):
                    tmpl = template_wscript_sdk
                else:
                    tmpl = template_wscript_extlib

                    dct['TARGET'] = Utils.split_path(tgen.target)[-1]
                    dct['PATH']   = self.install_dirs[node][len(remove_prefix):]

                # If a component defines public_defines we will use them,
                # otherwise fall back to export defines
                export_defines = Utils.to_list(getattr(tgen, 'public_defines', [])) +\
                                Utils.to_list(getattr(tgen, 'export_defines', []))

                uselib = Utils.to_list(getattr(tgen, 'uselib', []))

                indent = ' ' * 28
                dct['EXPORT_DEFINES'] = (',\n' + indent).join("'%s'" % x for x in export_defines)
                dct['USELIB']         = (',\n' + indent).join("'%s'" % x for x in uselib)

                fh.write(separator + tmpl.substitute(dct))
                separator = '\n'

class inst_zip(waflib.Build.inst):
    u''' Generate a zip file during install '''
    color     = 'PINK'
    inst_to   = None
    cmdline   = None

    def run(self):
        import zipfile

        input_nodes   = self.inputs
        install_paths = self.install_paths

        zip_path = self.get_install_path()

        d = os.path.dirname(zip_path)
        try:
            os.makedirs(d)
        except:
            pass

        Logs.info('+ install_zip %s' % (os.path.relpath(zip_path, self.generator.bld.srcnode.abspath())))

        with zipfile.ZipFile(self.get_install_path(), mode = 'w', compression=zipfile.ZIP_DEFLATED) as zfile:
            for i in input_nodes:
                zfile.write(i.abspath(), install_paths[i])

class distribute_generate_debug(waflib.Task.Task):
    u''' Generate debug information in distribution folder '''
    log_str   = "[DEBUGZIP] $TARGETS"
    color     = 'PINK'
    inst_to   = None
    cmdline   = None

    def run(self):
        import zipfile

        output_node   = self.outputs[0]
        input_nodes   = self.inputs
        install_paths = self.install_paths

        output_node.parent.mkdir()

        with zipfile.ZipFile(output_node.abspath(), mode = 'w', compression=zipfile.ZIP_DEFLATED) as zfile:
            for i in input_nodes:
                zfile.write(i.abspath(), install_paths[i])

class HilscherInstallContext(waflib.Build.InstallContext):
    def install_files(self, dest, files, env=None, chmod=Utils.O644, relative_trick=False, cwd=None, add=True, postpone=True):
        """
        Create a task to install files on the system::

            def build(bld):
                bld.install_files('${DATADIR}', self.path.find_resource('wscript'))

        :param dest: absolute path of the destination directory
        :type dest: string
        :param files: input files
        :type files: list of strings or list of nodes
        :param env: configuration set for performing substitutions in dest
        :type env: Configuration set
        :param relative_trick: preserve the folder hierarchy when installing whole folders
        :type relative_trick: bool
        :param cwd: parent node for searching srcfile, when srcfile is not a :py:class:`waflib.Node.Node`
        :type cwd: :py:class:`waflib.Node.Node`
        :param add: add the task created to a build group - set ``False`` only if the installation task is created after the build has started
        :type add: bool
        :param postpone: execute the task immediately to perform the installation
        :type postpone: bool
        """
        tsk = waflib.Build.inst(env=env or self.env)
        tsk.bld = self
        tsk.path = cwd or self.path
        tsk.chmod = chmod

        if isinstance(files, Node.Node):
            files =  [files]
        else:
            files = Utils.to_list(files)

            for f in files:
                if not isinstance(f, Node.Node):
                    node = self.path.find_node(f)

                    if not node:
                        self.fatal("File %s does not exist (%s/wscript)" % (f,self.path.relpath()));

            files = list((f if isinstance(f, Node.Node) else self.path.find_node(f)) for f in files)

        tsk.source = files
        tsk.dest = dest
        tsk.exec_task = tsk.exec_install_files
        tsk.relative_trick = relative_trick
        if add: self.add_to_group(tsk)
        self.run_task_now(tsk, postpone)
        return tsk

