# vim:fileencoding=utf-8:sw=4:tw=4:et
########################################################################################
# Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.
########################################################################################
# $Id: hilscher_plantuml.py 951 2024-08-23 06:38:32Z AMesser $:
#
# Provides support for plantuml tool
########################################################################################
import os.path
import re

extract_plantuml_version = re.compile(r'([0-9]+)[.]([0-9]+)[.]([0-9]+)').match

def configure(conf):
    # change to parent environment object
    conf.setenv('')

    plantuml_search_paths = [
        os.path.join(os.environ['PATH_BUILDTOOLS'], 'plantuml'),
        os.path.join(os.environ['PATH_BUILDTOOLS'], 'doc', 'plantuml')
    ]

    ret = None
    for p in plantuml_search_paths:
        plantuml_buildtools_node = conf.root.find_dir(p)

        if plantuml_buildtools_node:
            break

    if plantuml_buildtools_node:
        # find all available versions
        jar_nodes = plantuml_buildtools_node.ant_glob('**/plantuml.jar')

        nodes_by_version = {}
        for n in jar_nodes:
            # we have to support different folder formats:
            if n.parent == plantuml_buildtools_node:
                # new: plantuml.jar
                nodes_by_version = { 'new_style' : n }
                break
            else:
                # old: a.b.c/plantuml.jar
                m = extract_plantuml_version(n.parent.name)

                version = tuple(int(x,10) for x in m.groups())

                # make mapping version -> file
                nodes_by_version[version] = n

        # get most recent version
        k = sorted(nodes_by_version.keys())[-1]

        ret = nodes_by_version[k].abspath()

    conf.msg('Checking for plantuml', ret or False)

    if ret:
        conf.env['PLANTUML'] = ret
