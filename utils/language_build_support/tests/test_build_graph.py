# test_build_graph.py - Test the build_graph using mocks --------*- python -*-
#
# This source file is part of the Swift.org open source project
#
# Copyright (c) 2014 - 2020 Apple Inc. and the Swift project authors
# Licensed under Apache License v2.0 with Runtime Library Exception
#
# See https://language.org/LICENSE.txt for license information
# See https://language.org/CONTRIBUTORS.txt for the list of Swift project authors


import unittest

from language_build_support import build_graph


class ProductMock(object):
    def __init__(self, name):
        self.name = name
        self.deps = []

    def get_dependencies(self):
        return self.deps

    def __repr__(self):
        return "<ProductMock: {}>".format(self.name)


def get_products():
    products = {
        "cmark": ProductMock("cmark"),
        "toolchain": ProductMock("toolchain"),
        "language": ProductMock("language"),
        "languagepm": ProductMock("languagepm"),
        "libMockSwiftPM": ProductMock("libMockSwiftPM"),
        "libMockCMark": ProductMock("libMockCMark"),
        "libMockSwiftPM2": ProductMock("libMockSwiftPM2"),
    }

    products['toolchain'].deps.extend([products['cmark']])
    products['language'].deps.extend([products['toolchain']])
    products['languagepm'].deps.extend([products['toolchain'], products['language']])
    products['libMockSwiftPM'].deps.extend([products['languagepm']])
    products['libMockCMark'].deps.extend([products['cmark']])
    products['libMockSwiftPM2'].deps.extend([products['languagepm'], products['cmark']])

    return products


class BuildGraphTestCase(unittest.TestCase):

    def test_simple_build_graph(self):
        products = get_products()
        selectedProducts = [products['languagepm']]
        schedule = build_graph.produce_scheduled_build(selectedProducts)
        names = [x.name for x in schedule[0]]
        self.assertEqual(['cmark', 'toolchain', 'language', 'languagepm'], names)
