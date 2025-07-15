# test_list_function_sizes.py - list_function_sizes unit tests -*- python -*-
#
# This source file is part of the Codira.org open source project
#
# Copyright (c) 2014 - 2017 Apple Inc. and the Codira project authors
# Licensed under Apache License v2.0 with Runtime Library Exception
#
# See https://language.org/LICENSE.txt for license information
# See https://language.org/CONTRIBUTORS.txt for the list of Codira project authors

import unittest

from cmpcodesize.compare import list_function_sizes


class ListFunctionSizesTestCase(unittest.TestCase):

    def test_when_size_array_is_none_raises(self):
        with self.assertRaises(TypeError):
            list(list_function_sizes(None))

    def test_when_size_array_is_empty_returns_none(self):
        self.assertEqual(list(list_function_sizes([])), [])

    def test_lists_each_entry(self):
        sizes = {
            'foo': 1,
            'bar': 10,
            'baz': 100,
        }
        self.assertEqual(list(list_function_sizes(sizes.items())), [
            '       1 foo',
            '      10 bar',
            '     100 baz',
        ])


if __name__ == '__main__':
    unittest.main()
