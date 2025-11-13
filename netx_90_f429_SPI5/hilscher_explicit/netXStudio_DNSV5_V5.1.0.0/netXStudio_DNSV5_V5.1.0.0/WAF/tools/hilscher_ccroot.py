# encoding: utf-8
########################################################################################
# Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.
########################################################################################
# $Id: hilscher_ccroot.py 807 2023-03-24 10:02:44Z AMesser $:
#
# Extensions / adaption to wafs standard ccroot tool
########################################################################################
from waflib.TaskGen import feature, after_method

def options(opt):
    opt.load('ccroot')

def configure(conf):
    conf.load('ccroot')

@feature('c', 'cxx', 'asm', 'includes')
@after_method('apply_incpaths')
def process_includes(self):
    ''' Function removes all empty folders from includes
        To disable this functionality add to your target parameter: features = 'no_filter_includes'
    '''
    # if feature 'no_filter_includes' is set, skip filtering of empty "includes"
    if 'no_filter_includes' in self.features:
        return

    l = self.to_list(getattr(self,'includes_nodes',[]))

    def filter_dir(x):
       try:
            if x.is_bld():
                return True
            else:
                return len(x.listdir()) > 0
       except OSError:
           return False

    self.include_nodes = list(x for x in l if filter_dir(x))

    # update 'INCPATHS'
    self.env['INCPATHS'] = [x.abspath() for x in self.includes_nodes]


@feature('c', 'cxx', 'asm', 'includes')
@after_method('apply_incpaths', 'process_includes')
def relative_includes(self):
    ''' Function replaces absolute with relative include paths.

        To disable this functionality add to your target parameter: features = 'no_relative_includes'
    '''
    # if feature 'no_filter_includes' is set, skip filtering of empty "includes"
    if 'no_relative_includes' in self.features:
        return

    project_dir = self.bld.srcnode
    cc_cwd = self.bld.path.get_bld()

    # do not use relativice paths outside of our project directory
    # this may fail in particular on windows when path is located
    # on another drive
    def make_relative_path(node):
        if node.is_child_of(project_dir):
            return node.path_from(cc_cwd)

        return node.abspath()

    self.env['INCPATHS'] = [ make_relative_path(x) for x in self.includes_nodes]
