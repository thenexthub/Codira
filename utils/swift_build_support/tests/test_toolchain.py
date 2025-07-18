# -*- python -*-
# test_toolchain.py - Unit tests for language_build_support.toolchain
#
# This source file is part of the Codira.org open source project
#
# Copyright (c) 2014 - 2017 Apple Inc. and the Codira project authors
# Licensed under Apache License v2.0 with Runtime Library Exception
#
# See https://language.org/LICENSE.txt for license information
# See https://language.org/CONTRIBUTORS.txt for the list of Codira project authors

import os
import platform
import unittest

from language_build_support import toolchain
from language_build_support.toolchain import host_toolchain


def get_suffix(path, prefix):
    basename = os.path.basename(path)
    return basename[len(prefix):]


class ToolchainTestCase(unittest.TestCase):

    def test_clang_tools(self):
        tc = host_toolchain()

        self.assertIsNotNone(tc.cc)
        self.assertIsNotNone(tc.cxx)

        self.assertTrue(
            os.path.isabs(tc.cc) and
            os.path.basename(tc.cc).startswith(self._platform_cc_name()))
        self.assertTrue(
            os.path.isabs(tc.cxx) and
            os.path.basename(tc.cxx).startswith(self._platform_cxx_name()))

    def test_toolchain_tools(self):
        tc = host_toolchain()

        self.assertTrue(
            tc.toolchain_profdata is None or
            os.path.isabs(tc.toolchain_profdata) and
            os.path.basename(tc.toolchain_profdata).startswith('toolchain-profdata'))
        self.assertTrue(
            tc.toolchain_cov is None or
            os.path.isabs(tc.toolchain_cov) and
            os.path.basename(tc.toolchain_cov).startswith('toolchain-cov'))

    def test_misc_tools(self):
        tc = host_toolchain()

        # CMake
        self.assertIsNotNone(tc.cmake)
        self.assertTrue(
            os.path.basename(tc.cmake).startswith('cmake'))

        # Ninja
        self.assertTrue(tc.ninja is None or
                        os.path.basename(tc.ninja) == 'ninja' or
                        os.path.basename(tc.ninja) == 'ninja-build')
        # distcc
        self.assertTrue(tc.distcc is None or
                        os.path.basename(tc.distcc) == 'distcc')
        # pump
        self.assertTrue(tc.distcc_pump is None or
                        os.path.basename(tc.distcc_pump) == 'pump' or
                        os.path.basename(tc.distcc_pump) == 'distcc-pump')
        # sccache
        self.assertTrue(tc.sccache is None or
                        os.path.basename(tc.sccache) == 'sccache')

    def test_find_tool(self):
        tc = host_toolchain()

        # Toolchain.find_tool(path) can find arbitrary tool in PATH

        sh = tc.find_tool('sh')
        self.assertTrue(sh is not None and
                        os.path.isabs(sh) and
                        os.path.basename(sh) == 'sh')
        tar = tc.find_tool('tar')
        self.assertTrue(tar is not None and
                        os.path.isabs(tar) and
                        os.path.basename(tar) == 'tar')

    def test_tools_suffix_match(self):
        tc = host_toolchain()

        # CC and CXX must have consistent suffix
        cc_suffix = get_suffix(tc.cc, self._platform_cc_name())
        cxx_suffix = get_suffix(tc.cxx, self._platform_cxx_name())
        self.assertEqual(cc_suffix, cxx_suffix)

    def test_tools_toolchain_suffix(self):
        tc = host_toolchain()

        cov_suffix = None
        profdata_suffix = None
        if tc.toolchain_cov:
            cov_suffix = get_suffix(tc.toolchain_cov, 'toolchain-cov')
        if tc.toolchain_profdata:
            profdata_suffix = get_suffix(tc.toolchain_profdata, 'toolchain-profdata')

        if profdata_suffix is not None and cov_suffix is not None:
            self.assertEqual(profdata_suffix, cov_suffix)

        # If we have suffixed clang, toolchain tools must have the same suffix.
        cc_suffix = get_suffix(tc.cc, self._platform_cc_name())
        if cc_suffix != '':
            if cov_suffix is not None:
                self.assertEqual(cc_suffix, cov_suffix)
            if profdata_suffix is not None:
                self.assertEqual(cc_suffix, profdata_suffix)

    def test_toolchain_instances(self):
        # Check that we can instantiate every toolchain, even if it isn't the
        # current platform.
        toolchain.MacOSX()
        toolchain.Linux()
        toolchain.FreeBSD()
        toolchain.Cygwin()

    def _platform_cc_name(self):
        if platform.system() == 'Windows':
            return 'clang-cl'
        else:
            return 'clang'

    def _platform_cxx_name(self):
        if platform.system() == 'Windows':
            return 'clang-cl'
        else:
            return 'clang++'


if __name__ == '__main__':
    unittest.main()
