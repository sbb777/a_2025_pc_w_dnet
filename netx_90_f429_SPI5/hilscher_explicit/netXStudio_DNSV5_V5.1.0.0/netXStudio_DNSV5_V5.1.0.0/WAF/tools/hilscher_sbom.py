# vim:fileencoding=utf-8:sw=4:tw=4:et
########################################################################################
# Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.
########################################################################################
# $Id: hilscher_sbom.py 1107 2025-01-30 10:57:50Z AMesser $:
#
# Hilscher cyclone dx sbom support
########################################################################################
import waflib.Build
from waflib.Errors import WafError
from waflib.TaskGen import feature, after_method, taskgen_method
from waflib import Logs
import os.path
import json
import pathlib
import hashlib
import copy
import datetime

def configure(conf):
    pass

class CycloneDXSBOM(dict):
    @property
    def dependencies(self):
        ''' property to get a dict of dependencies '''
        return dict((d['ref'], d) for d in self.get('dependencies', []))

    @property
    def components(self):
        ''' property to get a dict of components by their bom-ref '''
        return dict((c['bom-ref'],c) for c in self.get('components', []) if 'bom-ref' in c)

class taskgen_property(property):
    def __init__(self, getter):
        self.__name__ = getter.__name__
        self.getter = getter

        taskgen_method(self)

    def __get__(self, obj, objtype = None):
        return self.getter(obj)

    def __call__(self, tgen):
        return self.getter(tgen)

@taskgen_method
def report_sbom_finding(self, node, msg):
    ''' Report a finding for a sbom file only during install stage'''
    if self.bld.is_install:
        self.report_finding(node, msg)

@taskgen_property
def is_precompiled(self):
    return (self.is_sdk() and 'sdk' not in self.features) or 'fake_lib' in self.features

@taskgen_property
def sbom(self):
    # we keep a dictionary of sboms associated with the folder the tgen's wscript is in
    sboms = self.bld.sboms = getattr(self.bld, 'sboms', {})
    key = self.path

    try:
        sbom = sboms[key]
    except KeyError:
        node = self.find_input_sbom(self)

        if node:
            Logs.debug(f'hilscher_sbom: Initializing bom for {self.path} from {node}')
            try:
                sbom = CycloneDXSBOM(node.read_json())
            except Exception as e:
                self.file_error(f'Failed to parse sbom file: {e!s}', node)
        else:
            Logs.debug(f'hilscher_sbom: Initializing empty bom for {self.path}')

            if self.is_precompiled:
                self.report_sbom_finding(self.path, 'SBOM file missing')

            sbom = CycloneDXSBOM()

        def check_copyright(node, component):
            if not component.get('copyright', ''):
                name = component.get('name', '<top level>')
                if node:
                    self.report_sbom_finding(node, f'copyright information missing for \'{name}\'')
                else:
                    self.report_sbom_finding(self.path, f'SBOM file missing')

        # check copyright existence of sbom component
        check_copyright(node, sbom.get('metadata',{}).get('component',{}))

        # check copyright existence of contained components
        for component in sbom.get('components',[]):
            check_copyright(node, component)

        sboms[key] = sbom

    return sbom

@taskgen_property
def sbom_ref(self):
    self.hil_ensure_posted()

    if not hasattr(self, 'version'):
        self.wscript_error(f'Can not determine version of {self.name}')

    return f'{self.name}/{self.version}'

@feature('*')
def update_sbom_external_sdk(tgen):
    ''' handle sbom for sdks of externaly built component components'''
    if tgen.is_sdk() and not 'sdk' in tgen.features:
        components = tgen.sbom.setdefault('components', [])
        for c in components:
            if c['name'] == tgen.name:
                break
        else:
            tgen.report_sbom_finding(tgen.path, f'No SBOM information for Pre-Compiled SDK {tgen.name}')

            # Generating a basic sbom entry from information in wscript
            c = dict(type='library', name=tgen.name)
            components.append(c)

            c['version']     = tgen.version
            c['bom-ref']     = tgen.sbom_ref
            c['description'] = tgen.description

@feature('fake_lib')
def update_sbom_external_component(tgen):
    components = tgen.sbom.setdefault('components', [])
    for c in components:
        if c['name'] == tgen.name:
            break
    else:
        tgen.report_sbom_finding(tgen.path, f'No SBOM information for Pre-Compiled library {tgen.name}')

        # Generating a basic sbom entry from information in wscript
        c = dict(type='library', name=tgen.name)
        components.append(c)

        c['version']     = tgen.version
        c['bom-ref']     = tgen.sbom_ref
        c['description'] = tgen.description


@taskgen_method
def iterate_use_sbom(self, use):
    ''' Returns task generators to consider for sbom referenced according to a use paramater value '''
    use_stack = use[:]
    use_seen  = set()

    while use_stack:
        x = use_stack.pop(0)

        if x not in use_seen:
            tgen = self.bld.get_tgen_by_name(x)
            use_seen.add(x)

            if getattr(tgen,'typ', '') == 'objects':
                # Special handling when the "used" task generator is of kind "objects"
                # Instead of returning the task generator itself, we will consider the use parameters
                # of the object task generator
                use_stack.extend(self.to_list(getattr(tgen, 'use', [])))
            elif tgen.is_sdk() or getattr(tgen, 'hidden_from_list', None) != 'Internal':
                yield tgen

def update_sbom(self, typ, use):
    components = self.sbom.setdefault('components', [])
    for c in components:
        if c['name'] == self.name:
            break
    else:
        c = dict(type=typ, name=self.name)
        components.append(c)

    try:
        # The version number and bom-ref in SBOM is always overriden by waf knowledge
        c['version']     = self.version
        c['bom-ref']     = self.sbom_ref

        # Description field is not override if given in sbom
        c['description'] = c.get('description', self.description)

        if use:
            dependencies = self.sbom.setdefault('dependencies', [])

            for d in dependencies:
                if d['ref'] == self.sbom_ref:
                    break
            else:
                d = dict(ref=self.sbom_ref, dependsOn=[])
                dependencies.append(d)

            depends_on = set(t.sbom_ref for t in self.iterate_use_sbom(use))

            d['dependsOn'] = list(set(d['dependsOn']) | depends_on)

        # propagate copyright from top-level if not yet set
        top_level_c = self.sbom.get('metadata', {}).get('component', {})
        if 'copyright' not in c and 'copyright' in top_level_c:
            c['copyright'] = top_level_c['copyright']

    except AttributeError as e:
        self.wscript_warning(f'Insufficient arguments to generate SBOM for {self.name}: {e!s}')
        pass


@feature('cxxstlib', 'cstlib', 'cxxshlib', 'cshlib', 'sdk')
@after_method('hil_parse_version_header', 'hil_inherit_version')
def update_sbom_lib(self):
    ''' Update internal SBOM store for libs & sdks built in this project '''
    use = self.to_list(getattr(self,'use',[]))
    update_sbom(self, 'library', use)

@feature('hboot', 'bootimage')
@after_method('hil_parse_version_header', 'hil_inherit_version')
def update_sbom_firmware(self):
    '''  Update internal sbom store for firmwares build in this project'''
    program, = self.to_list(self.use)
    program_tgen = self.bld.get_tgen_by_name(program)

    use = self.to_list(getattr(program_tgen,'use',[]))
    update_sbom(self, 'firmware', use)


def find_deprecated_sbom(self, tgen):
    ''' Find deprecated sbom for a distributed component '''
    node = tgen.path.find_resource('sbom.json')

    if node:
        if tgen.is_precompiled:
            self.report_sbom_finding(node, f'Deprecated SBOM file name {node.name}. Please contact component provider.')
        else:
            self.report_sbom_finding(node, f'Deprecated SBOM file name {node.name}. Please change extension to .cdx.json')

    return node

@taskgen_method
def find_input_sbom(self, dist_tgen):
    ''' Find input sbom for a task generator '''
    nodes = dist_tgen.path.ant_glob(['*.cdx.json', 'cdx.json'], excl='*~')

    if len(nodes) > 1:
        self.wscript_error(f'Found more than one sbom file in {dist_tgen.path}: {nodes}')

    if len(nodes) == 1:
        return nodes[0]
    else:
        return find_deprecated_sbom(self, dist_tgen)

@feature('distribute_lib')
@after_method('dist_binary')
def distribute_lib_sbom(self):
    bld = self.bld

    # SBOM will be generated only during install
    if not getattr(bld,'is_install', False):
        return

    sbom_task = self.create_task('build_sbom', [], [])
    sbom_task.dest = sbom_task.install_to = self.install_path + '/cdx.json'

    wscript_task = self.wscript_task

    input_sbom_node = None
    component_nodes = []

    # Find input sboms for all distributed task generators
    # and ensure that only one input sbom is available
    for tgen, stlib_node in wscript_task.generators:
        n = self.find_input_sbom(tgen)

        if n:
            if (input_sbom_node or n) != n:
                self.wscript_warning(f'Can not merge sbom {n.nice_path()!r} into {input_sbom_node.nice_path()!r}')

            input_sbom_node = n

            if stlib_node:
                component_nodes.append(stlib_node)

                a = pathlib.Path(wscript_task.install_dirs[stlib_node]) / stlib_node.name
                b = pathlib.Path(sbom_task.install_to)

                install_to = '/'.join(a.relative_to(b.parent).parts)
            else:
                install_to = None

            sbom_task.add_library(tgen, install_to, stlib_node)

    if not input_sbom_node:
        self.wscript_error(f'Did not found template sbom for distributed components')

    sbom_task.set_inputs([input_sbom_node] + component_nodes)

@feature('distribute_firmware')
@after_method('dist_binary')
def distribute_firmware_sbom(self):
    bld = self.bld

    # SBOM will be generated only during install
    if not getattr(bld,'is_install', False):
        return

    for tgen, node, install_to in self.installed_nodes:
        if 'hboot' in tgen.features or 'bootimage' in tgen.features:
            input_sbom_node = self.find_input_sbom(tgen)

            if not input_sbom_node:
                self.wscript_error(f'Did not found template sbom for distributed component {tgen.name}')

            firmware_install_path = pathlib.Path(install_to)
            sbom_install_path = firmware_install_path.with_suffix('.cdx.json')

            sbom_task = self.create_task('build_sbom', [input_sbom_node, node], [])
            sbom_task.dest = sbom_task.install_to = '/'.join(sbom_install_path.parts)

            c = sbom_task.register_meta_component(tgen)

            properties = c.setdefault('properties', [])

            p = '/'.join(firmware_install_path.relative_to(sbom_install_path.parent).parts)
            properties.append({ 'name' : 'path', 'value' : p})
            sbom_task.installed_nodes[p] = node

            program, = self.to_list(tgen.use)
            program_tgen = bld.get_tgen_by_name(program)

            for tg in self.iterate_use_sbom(program_tgen.tmp_use_seen):
                sbom_task.add_referenced_component(tg)

class build_sbom(waflib.Build.inst):
    ''' Generate sbom for distribution '''
    color     = 'PINK'
    log_str   = '[SBOM] $TARGETS'

    # make waf2 happy
    link      = ''

    def __init__(self, *args, **kwargs):
        super().__init__(*args,**kwargs)
        self.meta_component  = None
        self.components      = {}
        self.dependencies    = {}
        self.installed_nodes = {}

    def add_library(self, tgen, install_to, stlib_node):
        c = self.register_component(tgen)

        if install_to:
            properties = c.setdefault('properties', [])
            properties.append({ 'name' : 'path', 'value' : install_to })
            self.installed_nodes[install_to] = stlib_node

        for x in tgen.iterate_use_sbom(getattr(tgen, 'tmp_use_seen', [])):
            self.add_referenced_component(x)


    def clean_ref_component_sbom(self, c):
        ''' Cleans imported sbom information from path & hash information '''
        # remove file paths for not individually distributed components
        props_filtered = list(p for p in c.pop('properties',[]) if p['name'] != 'path')

        if props_filtered:
            c['properties'] = props_filtered

        # remove hashes for not individually distributed
        c.pop('hashes', [])

    def register_dependencies(self, tgen, bom_ref):
        ''' Add source sbom dependencies for tgen to generated sbom

            This will also import sbom & components from source sbom file '''
        sbom_deps       = tgen.sbom.dependencies

        deps_processed = set()
        deps_tmp       = [bom_ref]

        # import all depedency relationships from source sbom
        # into our sbom
        while deps_tmp:
            br = deps_tmp.pop(0)
            deps_processed.add(br)

            d = sbom_deps.get(br, None)

            if d:
                self.dependencies[br] = copy.deepcopy(d)
                deps_tmp.extend(set(d.get('dependsOn',[])) - deps_processed)

        # import all dependency components from source sbom
        # into our sbom as well
        sbom_components = tgen.sbom.components
        for br in (deps_processed - set([bom_ref])):
            try:
                sbom_c = sbom_components[br]
            except KeyError:
                if tgen.is_precompiled:
                    # actually, this is an error because an sbom must be self contained/complete
                    tgen.report_sbom_finding(tgen.path, f'component bom-ref {br!r} from dependency not found in sbom')
            else:
                if br not in self.components:
                    c = copy.deepcopy(sbom_c)
                    self.clean_ref_component_sbom(c)
                    self.components[br] = c

    def register_component(self, tgen):
        bom_ref   = sbom_ref(tgen)

        try:
            c = self.components[bom_ref]
        except KeyError:
            try:
                c = self.components[bom_ref] = copy.deepcopy(tgen.sbom.components[bom_ref])
            except KeyError:
                raise WafError(f'Did not found component {bom_ref} in sbom {tgen.sbom}')

            self.register_dependencies(tgen, bom_ref)

        return c

    def add_referenced_component(self, tgen):
        bom_ref   = sbom_ref(tgen)

        if bom_ref not in self.components:
            try:
                c = copy.deepcopy(tgen.sbom.components[bom_ref])
            except KeyError:
                typ = getattr(tgen, 'typ', None)

                # "objects" task generator do not auto generate sbom information since
                # this makes not much sense. The user may optionall specifiy this
                # information in template sbom if needed. Do not fail if the
                # information is not available
                if typ != 'objects':
                    raise WafError(f'Did not found component {bom_ref} in sbom {tgen.sbom}')
            else:
                self.clean_ref_component_sbom(c)

                self.components[bom_ref] = c
                self.register_dependencies(tgen, bom_ref)

    def register_meta_component(self, tgen):
        bom_ref   = sbom_ref(tgen)

        if self.meta_component:
            raise WafError('Meta-Component already registered')

        try:
            c = copy.deepcopy(tgen.sbom['metadata']['component'])
        except AttributeError:
            c = dict()

        c.update(tgen.sbom.components[bom_ref])

        self.meta_component = c

        self.register_dependencies(tgen, bom_ref)

        return self.meta_component

    def update_checksums(self, c):
        for p in c.get('properties', []):
            if (p['name'] == 'path'):
                artifact_install_to  = p['value']
                artifact_node = self.installed_nodes[artifact_install_to]

                sha256 = hashlib.sha256()
                sha512 = hashlib.sha512()
                with open(artifact_node.abspath(), 'rb') as f:
                    content = f.read()

                    sha256.update(content)
                    sha512.update(content)

                h = c.setdefault('hashes',[])

                for entry in h:
                    if entry['alg'] == 'SHA-256':
                        if entry['content'] != sha256.hexdigest():
                            raise WafError(f'Mismatch between calculated & sbom provided hash value for {artifact_install_to}')
                        sha256 = None

                    if entry['alg'] == 'SHA-512':
                        if entry['content'] != sha512.hexdigest():
                            raise WafError(f'Mismatch between calculated & sbom provided hash value for {artifact_install_to}')
                        sha512 = None

                if sha256:
                    h.append({'alg' : 'SHA-256', 'content' : sha256.hexdigest()})

                if sha512:
                    h.append({'alg' : 'SHA-512', 'content' : sha512.hexdigest()})

    def run(self):
        if self.meta_component:
            self.update_checksums(self.meta_component)

        for c in self.components.values():
            self.update_checksums(c)

        input_sbom_node = self.inputs[0]

        # use the input sbom as a base
        cyclone_dx = input_sbom_node.read_json()

        metadata = cyclone_dx.setdefault('metadata',{})

        # Set default SBOM manufacturer. See WAF-301 for details
        if 'manufacturer' not in metadata:
            manufacturer = metadata['manufacturer'] = {}

            manufacturer['name'] = 'Hilscher Gesellschaft fuer Systemautomation mbH'
            manufacturer['url']  = ['https://www.hilscher.com']

        # provide timestamp. Instead of using strftime and hardcoding a 'Z', we replace
        # +00:00 by 'Z'. This also makes code more robust if the datetime is not in utc
        metadata['timestamp'] = self.generator.get_build_stamp().astimezone(datetime.timezone.utc).isoformat().replace('+00:00', 'Z')

        if self.meta_component:
            metadata['component'] = self.meta_component


        cyclone_dx['components']   = list(self.components.values())
        cyclone_dx['dependencies'] = list(self.dependencies.values())

        #self.sbom            = CycloneDXSBOM(bomFormat="CycloneDX", specVersion="1.6", version=1)

        cyclone_dx_path = self.get_install_path()

        d = os.path.dirname(cyclone_dx_path)
        try:
            os.makedirs(d)
        except:
            pass

        with open(cyclone_dx_path, 'wt') as f:
            json.dump(cyclone_dx, f, indent=2)


