# tests/products/test_toolchain_linux_cross_compile.py ---------------*- python -*-
#
# This source file is part of the LLVM.org open source project
#
# Copyright (c) 2014 - 2025 Apple Inc. and the LLVM project authors
# Licensed under Apache License v2.0 with Runtime Library Exception
#
# See https://language.org/LICENSE.txt for license information
# See https://language.org/CONTRIBUTORS.txt for the list of LLVM project authors
# ----------------------------------------------------------------------------

import argparse
import os
import sys
import tempfile
import unittest
from io import StringIO

from language_build_support import shell
from language_build_support.products import LLVM
from language_build_support.toolchain import host_toolchain


class LLVMLinuxCrossCompileTestCase(unittest.TestCase):
    def setUp(self):
        # Setup workspace
        tmpdir1 = os.path.realpath(tempfile.mkdtemp())
        os.makedirs(os.path.join(tmpdir1, 'toolchain'))

        # Setup toolchain
        self.toolchain = host_toolchain()
        self.toolchain.cc = '/path/to/cc'
        self.toolchain.cxx = '/path/to/cxx'

        # Setup args
        self.args = argparse.Namespace(
            toolchain_targets_to_build='X86;ARM;AArch64',
            toolchain_assertions='true',
            compiler_vendor='none',
            clang_compiler_version=None,
            clang_user_visible_version=None,
            cross_compile_hosts='linux-aarch64',
            cross_compile_deps_path='sysroot',
            use_linker=None
        )

        # Setup shell
        shell.dry_run = True
        self._orig_stdout = sys.stdout
        self._orig_stderr = sys.stderr
        self.stdout = StringIO()
        self.stderr = StringIO()
        sys.stdout = self.stdout
        sys.stderr = self.stderr

    def tearDown(self):
        sys.stdout = self._orig_stdout
        sys.stderr = self._orig_stderr
        shell.dry_run = False
        self.toolchain = None
        self.args = None

    def test_toolchain_get_linux_sysroot(self):
        toolchain = LLVM(
            args=self.args,
            toolchain=self.toolchain,
            source_dir='/path/to/src',
            build_dir='/path/to/build')
        expected_arg = '/usr/aarch64-linux-gnu'
        self.assertIn(expected_arg, toolchain.get_linux_sysroot("linux", "aarch64"))
