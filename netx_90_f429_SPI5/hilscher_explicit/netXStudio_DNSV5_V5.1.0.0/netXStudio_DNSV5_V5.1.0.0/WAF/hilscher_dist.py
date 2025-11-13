#! /usr/bin/env python
# vim:fileencoding=utf-8:sw=4:tw=4:et
########################################################################################
# Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.
########################################################################################
# $Id: hilscher_dist.py 1143 2025-05-06 13:22:59Z AMesser $:
#
# Description:
# Distribution generation functions
########################################################################################
import waflib.Task
import waflib.TaskGen
import waflib.Build
from waflib import Logs, Utils, Options, Errors
from waflib.Configure import conf
from waflib.TaskGen import feature, after_method, before_method, taskgen_method
import os, stat, string, time

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

    use_ltd =  kw.get('use_ltd', [])[:]
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



@feature('cprogram', 'cxxprogram')
@before_method('dist_binary')
@after_method('apply_link')
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
        wscript_tsk = self.wscript_task = self.create_task('distribute_generate_wscript', [], [wscript_node])
        wscript_tsk.install_dirs  = {}
        wscript_tsk.generators     = []

        bld.install_as('/'.join(install_path + [self.wscript_name]), wscript_node)
        wscript_tsk.install_dirs[wscript_node] = '/'.join(install_path)

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

    installed_nodes = self.installed_nodes = getattr(self, 'installed_nodes', [])

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

        if tgen.is_sdk():
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
            dst = dst_subdir + '/' + dist_targets[node]
            installed_nodes.append((tgen, node, dst))

            bld.install_as(dst, node)

        if 'distribute_lib' in self.features:
            wscript_tsk.set_inputs(dist_nodes)

            y = None
            for y in dist_nodes:
                wscript_tsk.install_dirs[y] = '/'.join(dst_subdir_parts)

            wscript_tsk.generators.append((tgen, y))

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

    compiled_tasks = []
    base_nodes     = set()

    for x, name, subdir_path, tgen, flags in self.dist_iterate_use(use_source):
        compiled_tasks.extend(getattr(tgen,'compiled_tasks',[]))
        use = self.to_list(tgen.use)

        # for shared libraries, add subtasks
        if 'cstlib' in self.to_list(tgen.features):
            for _, _, _, tgen2, _ in self.dist_iterate_use(use):
                compiled_tasks.extend(getattr(tgen2,'compiled_tasks',[]))

        # try to detect firmware task generators
        elif len(use) == 1:
            _, _, _, tgen2, _ = next(self.dist_iterate_use(use))

            if 'cprogram' in self.to_list(tgen2.features):
                compiled_tasks.extend(getattr(tgen2,'compiled_tasks',[]))

                for _, _, _, tgen3, _ in self.dist_iterate_use(self.to_list(tgen2.use)):
                    compiled_tasks.extend(getattr(tgen3,'compiled_tasks',[]))

        # remind the base paths to include the source for when
        # scanning dependencies later on
        base_nodes.add(tgen.path)

    # Make a collection of all inputs of the compiled tasks
    source_nodes = set(t.inputs[0] for t in compiled_tasks)

    # map source nodes to zip paths
    source_paths = dict((n, n.path_from(bld.srcnode)) for n in source_nodes)

    tsk = self.install_zip(self.source_archive_name, source_paths)


    if tsk:
        # We must run the zip task after running the compiled tasks in order to
        # extract the header files used
        for t in compiled_tasks:
            tsk.set_run_after(t)

        # Install a scanner method which will extract the paths of used header files
        # from compile tasks after these tasks had been run
        def scan_deps_late(bld = bld, compiled_tasks = compiled_tasks, base_nodes = base_nodes):
            ''' Scan for header files after the compiled tasks had been run '''
            deps = set()

            # add dependencies of compiled objects (e.g. header files)
            for x in compiled_tasks:
                # dependencies are only build for tasks which have a scan method
                if x.scan:
                    try:
                        deps.update(set(bld.node_deps[x.uid()]))
                    except KeyError as e:
                        # make a nice error istead of crappy exception
                        raise bld.errors.WafError('Failed to get deps for %s' % x)

            # include only header files which are below the wscript path
            # of at least one of the task generators we're inlduing the source for
            # This is to avoid including header files of components not referenced
            # in use_source
            deps = set(d for d in deps if any(d.is_child_of(b) for b in base_nodes))

            # Add dependency nodes to sources
            tsk.install_paths.update((n,n.path_from(bld.srcnode)) for n in deps)

            return (list(deps), time.time())

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
        tsk = self.create_task('inst_zip', [], [])
        # waf 1.6 uses .dest, waf 2 uses tsk.install_to
        tsk.dest = tsk.install_to = '/'.join(install_path + [dest])
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

            def default_include_folder_names(tgen):
                return [self.generator.include_folder_name]

            get_include_folder_names = getattr(self.generator,'override_export_includes', default_include_folder_names)

            for tgen, node in self.generators:
                dist_includes = get_include_folder_names(tgen)

                dct = {
                    'NAME'           : tgen.name,
                    'DESCRIPTION'    : tgen.description,
                    'DISPLAYGROUP'   : tgen.displaygroup,
                    'VERSION'        : tgen.version,
                    'INCLUDE_PATHS'  : ",".join("'%s'" % x for x in dist_includes),
                }

                if tgen.is_sdk():
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

    # make waf2 happy
    link = ''

    @property
    def inputs(self):
        ''' inst_zip task computes inputs from mapping source_node -> archive_path'''
        return sorted(self.install_paths.keys(), key = lambda x : self.install_paths[x])

    @inputs.setter
    def inputs(self, val):
        ''' Report an error wehn someone tries to set inputs of inst_zip task '''
        if val:
            raise ValueError('Can not set inputs os inst_zip task')

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
    def install_files(self, dest, files, **kw):
        tmp = []
        for x in Utils.to_list(files):
            if isinstance(x, str):
                node = self.path.find_node(x)
            else:
                node = x

            if not node:
                self.wscript_error('Install file {} not found'.format(x))

            tmp.append(node)

        super(HilscherInstallContext, self).install_files(dest, files=tmp, **kw)

class inst(waflib.Build.inst):
    ''' Override of install task to make a nicer output '''
    def do_install(self,src,tgt,lbl,**kw):
        launch_node = self.generator.bld.launch_node()

        lbl_tgt = os.path.relpath(tgt, launch_node.abspath())

        ''' Install a file, this function will be only relevant for waf 2 or newer '''
        if not Options.options.force:
            try:
                st1=os.stat(tgt)
                st2=os.stat(src)
            except OSError:
                pass
            else:
                if st1.st_mtime+2>=st2.st_mtime and st1.st_size==st2.st_size:
                    if not self.generator.bld.progress_bar:
                        c1=Logs.colors.NORMAL
                        c2=Logs.colors.BLUE
                        Logs.info('%s- install %s%s%s (from %s)',c1,c2,lbl_tgt,c1,lbl)
                    return False
        if not self.generator.bld.progress_bar:
            c1=Logs.colors.NORMAL
            c2=Logs.colors.BLUE
            Logs.info('%s+ install %s%s%s (from %s)',c1,c2,lbl_tgt,c1,lbl)
        try:
            os.chmod(tgt,Utils.O644|stat.S_IMODE(os.stat(tgt).st_mode))
        except EnvironmentError:
            pass
        try:
            os.remove(tgt)
        except OSError:
            pass
        try:
            self.copy_fun(src,tgt)
        except EnvironmentError as e:
            if not os.path.exists(src):
                Logs.error('File %r does not exist',src)
            elif not os.path.isfile(src):
                Logs.error('Input %r is not a file',src)
            raise Errors.WafError('Could not install the file %r'%tgt,e)
