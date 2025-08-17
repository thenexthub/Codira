# tests/products/test_toolchain.py -----------------------------------*- python -*-
#
# This source file is part of the LLVM.org open source project
#
# Copyright (c) 2014 - 2017 Apple Inc. and the LLVM project authors
# Licensed under Apache License v2.0 with Runtime Library Exception
#
# See https://language.org/LICENSE.txt for license information
# See https://language.org/CONTRIBUTORS.txt for the list of LLVM project authors
# ----------------------------------------------------------------------------

import argparse
import os
import shutil
import sys
import tempfile
import unittest
from io import StringIO

from language_build_support import shell
from language_build_support.products import LLVM
from language_build_support.toolchain import host_toolchain
from language_build_support.workspace import Workspace


class LLVMTestCase(unittest.TestCase):

    def setUp(self):
        # Setup workspace
        tmpdir1 = os.path.realpath(tempfile.mkdtemp())
        tmpdir2 = os.path.realpath(tempfile.mkdtemp())
        os.makedirs(os.path.join(tmpdir1, 'toolchain'))

        self.workspace = Workspace(source_root=tmpdir1,
                                   build_root=tmpdir2)

        # Setup toolchain
        self.toolchain = host_toolchain()
        self.toolchain.cc = '/path/to/cc'
        self.toolchain.cxx = '/path/to/cxx'

        # Setup args
        self.args = argparse.Namespace(
            toolchain_targets_to_build='X86;ARM;AArch64;PowerPC;SystemZ',
            toolchain_assertions='true',
            compiler_vendor='none',
            clang_compiler_version=None,
            clang_user_visible_version=None,
            darwin_deployment_version_osx='10.9',
            use_linker=None)

        # Setup shell
        shell.dry_run = True
        self._orig_stdout = sys.stdout
        self._orig_stderr = sys.stderr
        self.stdout = StringIO()
        self.stderr = StringIO()
        sys.stdout = self.stdout
        sys.stderr = self.stderr

    def tearDown(self):
        shutil.rmtree(self.workspace.build_root)
        shutil.rmtree(self.workspace.source_root)
        sys.stdout = self._orig_stdout
        sys.stderr = self._orig_stderr
        shell.dry_run = False
        self.workspace = None
        self.toolchain = None
        self.args = None

    def test_toolchain_targets_to_build(self):
        toolchain = LLVM(
            args=self.args,
            toolchain=self.toolchain,
            source_dir='/path/to/src',
            build_dir='/path/to/build')
        expected_targets = 'X86;ARM;AArch64;PowerPC;SystemZ'
        expected_arg = '-DLLVM_TARGETS_TO_BUILD=%s' % expected_targets
        self.assertIn(expected_arg, toolchain.cmake_options)

    def test_toolchain_enable_assertions(self):
        self.args.toolchain_assertions = True
        toolchain = LLVM(
            args=self.args,
            toolchain=self.toolchain,
            source_dir='/path/to/src',
            build_dir='/path/to/build')
        self.assertIn('-DLLVM_ENABLE_ASSERTIONS:BOOL=TRUE', toolchain.cmake_options)

        self.args.toolchain_assertions = False
        toolchain = LLVM(
            args=self.args,
            toolchain=self.toolchain,
            source_dir='/path/to/src',
            build_dir='/path/to/build')
        self.assertIn('-DLLVM_ENABLE_ASSERTIONS:BOOL=FALSE',
                      toolchain.cmake_options)

    def test_compiler_vendor_flags(self):
        self.args.compiler_vendor = "none"
        self.args.clang_user_visible_version = "1.2.3"
        toolchain = LLVM(
            args=self.args,
            toolchain=self.toolchain,
            source_dir='/path/to/src',
            build_dir='/path/to/build')
        self.assertNotIn('-DCLANG_VENDOR=Apple', toolchain.cmake_options)
        self.assertNotIn(
            '-DCLANG_VENDOR_UTI=com.apple.compilers.toolchain.clang',
            toolchain.cmake_options
        )
        self.assertNotIn('-DPACKAGE_VERSION=1.2.3', toolchain.cmake_options)

        self.args.compiler_vendor = "apple"
        self.args.clang_user_visible_version = "2.2.3"
        toolchain = LLVM(
            args=self.args,
            toolchain=self.toolchain,
            source_dir='/path/to/src',
            build_dir='/path/to/build')
        self.assertIn('-DCLANG_VENDOR=Apple', toolchain.cmake_options)
        self.assertIn(
            '-DCLANG_VENDOR_UTI=com.apple.compilers.toolchain.clang',
            toolchain.cmake_options
        )
        self.assertIn('-DPACKAGE_VERSION=2.2.3', toolchain.cmake_options)

        self.args.compiler_vendor = "unknown"
        with self.assertRaises(RuntimeError):
            toolchain = LLVM(
                args=self.args,
                toolchain=self.toolchain,
                source_dir='/path/to/src',
                build_dir='/path/to/build')

    def test_version_flags(self):
        self.args.clang_compiler_version = None
        toolchain = LLVM(
            args=self.args,
            toolchain=self.toolchain,
            source_dir='/path/to/src',
            build_dir='/path/to/build')
        self.assertListEqual(
            [],
            [x for x in toolchain.cmake_options if 'CLANG_REPOSITORY_STRING' in x]
        )

        self.args.clang_compiler_version = "2.2.3"
        toolchain = LLVM(
            args=self.args,
            toolchain=self.toolchain,
            source_dir='/path/to/src',
            build_dir='/path/to/build')
        self.assertIn(
            '-DCLANG_REPOSITORY_STRING=clang-2.2.3',
            toolchain.cmake_options
        )

    def test_use_linker(self):
        self.args.use_linker = None
        toolchain = LLVM(
            args=self.args,
            toolchain=self.toolchain,
            source_dir='/path/to/src',
            build_dir='/path/to/build')
        for s in toolchain.cmake_options:
            self.assertFalse('CLANG_DEFAULT_LINKER' in s)

        self.args.use_linker = 'gold'
        toolchain = LLVM(
            args=self.args,
            toolchain=self.toolchain,
            source_dir='/path/to/src',
            build_dir='/path/to/build')
        self.assertIn(
            '-DCLANG_DEFAULT_LINKER=gold',
            toolchain.cmake_options
        )

        self.args.use_linker = 'lld'
        toolchain = LLVM(
            args=self.args,
            toolchain=self.toolchain,
            source_dir='/path/to/src',
            build_dir='/path/to/build')
        self.assertIn(
            '-DCLANG_DEFAULT_LINKER=lld',
            toolchain.cmake_options
        )
