#! /usr/bin/env python
# encoding: utf-8

APPNAME = 'chunkie'
VERSION = '7.0.0'


def build(bld):

    bld.env.append_unique(
        'DEFINES_STEINWURF_VERSION',
        'STEINWURF_CHUNKIE_VERSION="{}"'.format(
            VERSION))

    bld.recurse('src/chunkie')

    if bld.is_toplevel():

        # Only build tests when executed from the top-level wscript,
        # i.e. not when included as a dependency
        bld.recurse('test')
        bld.recurse('examples')
