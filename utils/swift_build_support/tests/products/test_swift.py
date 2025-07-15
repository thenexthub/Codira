# tests/products/test_language.py ----------------------------------*- python -*-
#
# This source file is part of the Codira.org open source project
#
# Copyright (c) 2014 - 2017 Apple Inc. and the Codira project authors
# Licensed under Apache License v2.0 with Runtime Library Exception
#
# See https://language.org/LICENSE.txt for license information
# See https://language.org/CONTRIBUTORS.txt for the list of Codira project authors
# ----------------------------------------------------------------------------

import argparse
import os
import shutil
import sys
import tempfile
import unittest
from io import StringIO

from language_build_support import shell
from language_build_support.products import Codira
from language_build_support.toolchain import host_toolchain
from language_build_support.workspace import Workspace


class CodiraTestCase(unittest.TestCase):

    def setUp(self):
        # Setup workspace
        tmpdir1 = os.path.realpath(tempfile.mkdtemp())
        tmpdir2 = os.path.realpath(tempfile.mkdtemp())
        os.makedirs(os.path.join(tmpdir1, 'language'))

        self.workspace = Workspace(source_root=tmpdir1,
                                   build_root=tmpdir2)

        # Setup toolchain
        self.toolchain = host_toolchain()
        self.toolchain.cc = '/path/to/cc'
        self.toolchain.cxx = '/path/to/cxx'

        # Setup args
        self.args = argparse.Namespace(
            enable_tsan_runtime=False,
            compiler_vendor='none',
            language_compiler_version=None,
            clang_compiler_version=None,
            language_user_visible_version=None,
            darwin_deployment_version_osx="10.9",
            benchmark=False,
            benchmark_num_onone_iterations=3,
            benchmark_num_o_iterations=3,
            disable_guaranteed_normal_arguments=True,
            force_optimized_typechecker=False,
            extra_language_cmake_options=["-DHELLO=YES"],
            enable_stdlibcore_exclusivity_checking=False,
            enable_experimental_differentiable_programming=False,
            enable_experimental_concurrency=False,
            enable_experimental_cxx_interop=False,
            enable_cxx_interop_language_bridging_header=False,
            enable_experimental_distributed=False,
            enable_experimental_observation=False,
            enable_experimental_parser_validation=False,
            language_enable_backtracing=False,
            enable_synchronization=False,
            enable_volatile=False,
            enable_runtime_module=False,
            build_early_languagesyntax=False,
            build_language_stdlib_static_print=False,
            build_language_stdlib_unicode_data=True,
            build_embedded_stdlib=True,
            build_embedded_stdlib_cross_compiling=False,
            language_freestanding_is_darwin=False,
            build_language_private_stdlib=True,
            language_tools_ld64_lto_codegen_only_for_supporting_targets=False,
            build_stdlib_docs=False,
            enable_new_runtime_build=False)

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

    def test_by_default_no_cmake_options(self):
        language = Codira(
            args=self.args,
            toolchain=self.toolchain,
            source_dir='/path/to/src',
            build_dir='/path/to/build')
        expected = [
            '-DCMAKE_EXPORT_COMPILE_COMMANDS=TRUE',
            '-DLANGUAGE_FORCE_OPTIMIZED_TYPECHECKER:BOOL=FALSE',
            '-DLANGUAGE_STDLIB_ENABLE_STDLIBCORE_EXCLUSIVITY_CHECKING:BOOL=FALSE',
            '-DLANGUAGE_ENABLE_EXPERIMENTAL_DIFFERENTIABLE_PROGRAMMING:BOOL=FALSE',
            '-DLANGUAGE_ENABLE_EXPERIMENTAL_CONCURRENCY:BOOL=FALSE',
            '-DLANGUAGE_ENABLE_EXPERIMENTAL_CXX_INTEROP:BOOL=FALSE',
            '-DLANGUAGE_ENABLE_CXX_INTEROP_LANGUAGE_BRIDGING_HEADER:BOOL=FALSE',
            '-DLANGUAGE_ENABLE_EXPERIMENTAL_DISTRIBUTED:BOOL=FALSE',
            '-DLANGUAGE_ENABLE_EXPERIMENTAL_OBSERVATION:BOOL=FALSE',
            '-DLANGUAGE_ENABLE_EXPERIMENTAL_PARSER_VALIDATION:BOOL=FALSE',
            '-DLANGUAGE_ENABLE_BACKTRACING:BOOL=FALSE',
            '-DLANGUAGE_ENABLE_SYNCHRONIZATION:BOOL=FALSE',
            '-DLANGUAGE_ENABLE_VOLATILE:BOOL=FALSE',
            '-DLANGUAGE_ENABLE_RUNTIME_MODULE:BOOL=FALSE',
            '-DLANGUAGE_STDLIB_STATIC_PRINT=FALSE',
            '-DLANGUAGE_FREESTANDING_IS_DARWIN:BOOL=FALSE',
            '-DLANGUAGE_STDLIB_BUILD_PRIVATE:BOOL=TRUE',
            '-DLANGUAGE_STDLIB_ENABLE_UNICODE_DATA=TRUE',
            '-DLANGUAGE_SHOULD_BUILD_EMBEDDED_STDLIB=TRUE',
            '-DLANGUAGE_SHOULD_BUILD_EMBEDDED_STDLIB_CROSS_COMPILING=FALSE',
            '-DLANGUAGE_TOOLS_LD64_LTO_CODEGEN_ONLY_FOR_SUPPORTING_TARGETS:BOOL=FALSE',
            '-ULANGUAGE_DEBUGINFO_NON_LTO_ARGS',
            '-DLANGUAGE_STDLIB_BUILD_SYMBOL_GRAPHS:BOOL=FALSE',
            '-DLANGUAGE_ENABLE_NEW_RUNTIME_BUILD:BOOL=FALSE',
            '-DHELLO=YES',
        ]
        self.assertEqual(set(language.cmake_options), set(expected))

    def test_language_runtime_tsan(self):
        self.args.enable_tsan_runtime = True
        language = Codira(
            args=self.args,
            toolchain=self.toolchain,
            source_dir='/path/to/src',
            build_dir='/path/to/build')
        flags_set = [
            '-DLANGUAGE_RUNTIME_USE_SANITIZERS=Thread',
            '-DCMAKE_EXPORT_COMPILE_COMMANDS=TRUE',
            '-DLANGUAGE_FORCE_OPTIMIZED_TYPECHECKER:BOOL=FALSE',
            '-DLANGUAGE_STDLIB_ENABLE_STDLIBCORE_EXCLUSIVITY_CHECKING:BOOL=FALSE',
            '-DLANGUAGE_ENABLE_EXPERIMENTAL_DIFFERENTIABLE_PROGRAMMING:BOOL=FALSE',
            '-DLANGUAGE_ENABLE_EXPERIMENTAL_CONCURRENCY:BOOL=FALSE',
            '-DLANGUAGE_ENABLE_EXPERIMENTAL_CXX_INTEROP:BOOL=FALSE',
            '-DLANGUAGE_ENABLE_CXX_INTEROP_LANGUAGE_BRIDGING_HEADER:BOOL=FALSE',
            '-DLANGUAGE_ENABLE_EXPERIMENTAL_DISTRIBUTED:BOOL=FALSE',
            '-DLANGUAGE_ENABLE_EXPERIMENTAL_OBSERVATION:BOOL=FALSE',
            '-DLANGUAGE_ENABLE_EXPERIMENTAL_PARSER_VALIDATION:BOOL=FALSE',
            '-DLANGUAGE_ENABLE_BACKTRACING:BOOL=FALSE',
            '-DLANGUAGE_ENABLE_SYNCHRONIZATION:BOOL=FALSE',
            '-DLANGUAGE_ENABLE_VOLATILE:BOOL=FALSE',
            '-DLANGUAGE_ENABLE_RUNTIME_MODULE:BOOL=FALSE',
            '-DLANGUAGE_STDLIB_STATIC_PRINT=FALSE',
            '-DLANGUAGE_FREESTANDING_IS_DARWIN:BOOL=FALSE',
            '-DLANGUAGE_STDLIB_BUILD_PRIVATE:BOOL=TRUE',
            '-DLANGUAGE_STDLIB_ENABLE_UNICODE_DATA=TRUE',
            '-DLANGUAGE_SHOULD_BUILD_EMBEDDED_STDLIB=TRUE',
            '-DLANGUAGE_SHOULD_BUILD_EMBEDDED_STDLIB_CROSS_COMPILING=FALSE',
            '-DLANGUAGE_TOOLS_LD64_LTO_CODEGEN_ONLY_FOR_SUPPORTING_TARGETS:BOOL=FALSE',
            '-ULANGUAGE_DEBUGINFO_NON_LTO_ARGS',
            '-DLANGUAGE_STDLIB_BUILD_SYMBOL_GRAPHS:BOOL=FALSE',
            '-DLANGUAGE_ENABLE_NEW_RUNTIME_BUILD:BOOL=FALSE',
            '-DHELLO=YES',
        ]
        self.assertEqual(set(language.cmake_options), set(flags_set))

    def test_language_compiler_vendor_flags(self):
        self.args.compiler_vendor = "none"
        self.args.code_user_visible_version = None
        self.args.code_compiler_version = None
        language = Codira(
            args=self.args,
            toolchain=self.toolchain,
            source_dir='/path/to/src',
            build_dir='/path/to/build')
        self.assertListEqual(
            [], [x for x in language.cmake_options if 'LANGUAGE_VENDOR' in x])
        self.assertListEqual(
            [], [x for x in language.cmake_options if 'LANGUAGE_VENDOR_UTI' in x])
        self.assertListEqual(
            [], [x for x in language.cmake_options if 'LANGUAGE_VERSION' in x])
        self.assertListEqual(
            [],
            [x for x in language.cmake_options if 'LANGUAGE_COMPILER_VERSION' in x])

        self.args.compiler_vendor = "apple"
        self.args.code_user_visible_version = "1.3"
        self.args.code_compiler_version = None
        language = Codira(
            args=self.args,
            toolchain=self.toolchain,
            source_dir='/path/to/src',
            build_dir='/path/to/build')
        self.assertIn('-DLANGUAGE_VENDOR=Apple', language.cmake_options)
        self.assertIn(
            '-DLANGUAGE_VENDOR_UTI=com.apple.compilers.toolchain.code',
            language.cmake_options)
        self.assertIn('-DLANGUAGE_VERSION=1.3', language.cmake_options)
        self.assertIn('-DLANGUAGE_COMPILER_VERSION=', language.cmake_options)

        self.args.compiler_vendor = "apple"
        self.args.code_user_visible_version = "1.3"
        self.args.code_compiler_version = "2.3"
        language = Codira(
            args=self.args,
            toolchain=self.toolchain,
            source_dir='/path/to/src',
            build_dir='/path/to/build')
        self.assertIn('-DLANGUAGE_VENDOR=Apple', language.cmake_options)
        self.assertIn(
            '-DLANGUAGE_VENDOR_UTI=com.apple.compilers.toolchain.code',
            language.cmake_options)
        self.assertIn('-DLANGUAGE_VERSION=1.3', language.cmake_options)
        self.assertIn('-DLANGUAGE_COMPILER_VERSION=2.3', language.cmake_options)

        self.args.compiler_vendor = "unknown"
        with self.assertRaises(RuntimeError):
            language = Codira(
                args=self.args,
                toolchain=self.toolchain,
                source_dir='/path/to/src',
                build_dir='/path/to/build')

    def test_version_flags(self):
        # First make sure that by default, we do not get any version flags.
        language = Codira(
            args=self.args,
            toolchain=self.toolchain,
            source_dir='/path/to/src',
            build_dir='/path/to/build')
        self.assertListEqual(
            [],
            [x for x in language.cmake_options if 'LANGUAGE_COMPILER_VERSION' in x]
        )
        self.assertListEqual(
            [],
            [x for x in language.cmake_options if 'CLANG_COMPILER_VERSION' in x]
        )

        self.args.code_compiler_version = "3.0"
        self.args.clang_compiler_version = None
        language = Codira(
            args=self.args,
            toolchain=self.toolchain,
            source_dir='/path/to/src',
            build_dir='/path/to/build')
        self.assertListEqual(
            ['-DLANGUAGE_COMPILER_VERSION=3.0'],
            [x for x in language.cmake_options if 'LANGUAGE_COMPILER_VERSION' in x]
        )
        self.assertListEqual(
            [],
            [x for x in language.cmake_options if 'CLANG_COMPILER_VERSION' in x]
        )

        self.args.code_compiler_version = None
        self.args.clang_compiler_version = "3.8.0"
        language = Codira(
            args=self.args,
            toolchain=self.toolchain,
            source_dir='/path/to/src',
            build_dir='/path/to/build')
        self.assertListEqual(
            [],
            [x for x in language.cmake_options if 'LANGUAGE_COMPILER_VERSION' in x]
        )
        self.assertListEqual(
            ['-DCLANG_COMPILER_VERSION=3.8.0'],
            [x for x in language.cmake_options if 'CLANG_COMPILER_VERSION' in x]
        )

        self.args.code_compiler_version = "1.0"
        self.args.clang_compiler_version = "1.9.3"
        language = Codira(
            args=self.args,
            toolchain=self.toolchain,
            source_dir='/path/to/src',
            build_dir='/path/to/build')
        self.assertListEqual(
            ['-DLANGUAGE_COMPILER_VERSION=1.0'],
            [x for x in language.cmake_options if 'LANGUAGE_COMPILER_VERSION' in x]
        )
        self.assertListEqual(
            ['-DCLANG_COMPILER_VERSION=1.9.3'],
            [x for x in language.cmake_options if 'CLANG_COMPILER_VERSION' in x]
        )
        self.args.code_compiler_version = None
        self.args.clang_compiler_version = None

    def test_benchmark_flags(self):
        self.args.benchmark = True
        language = Codira(
            args=self.args,
            toolchain=self.toolchain,
            source_dir='/path/to/src',
            build_dir='/path/to/build')
        # By default, we should get an argument of 3 for both -Onone and -O
        self.assertEqual(
            ['-DLANGUAGE_BENCHMARK_NUM_ONONE_ITERATIONS=3',
             '-DLANGUAGE_BENCHMARK_NUM_O_ITERATIONS=3'],
            [x for x in language.cmake_options if 'LANGUAGE_BENCHMARK_NUM' in x])

        self.args.benchmark_num_onone_iterations = 20
        language = Codira(
            args=self.args,
            toolchain=self.toolchain,
            source_dir='/path/to/src',
            build_dir='/path/to/build')
        self.assertEqual(
            ['-DLANGUAGE_BENCHMARK_NUM_ONONE_ITERATIONS=20',
             '-DLANGUAGE_BENCHMARK_NUM_O_ITERATIONS=3'],
            [x for x in language.cmake_options if 'LANGUAGE_BENCHMARK_NUM' in x])
        self.args.benchmark_num_onone_iterations = 3

        self.args.benchmark_num_o_iterations = 30
        language = Codira(
            args=self.args,
            toolchain=self.toolchain,
            source_dir='/path/to/src',
            build_dir='/path/to/build')
        self.assertEqual(
            ['-DLANGUAGE_BENCHMARK_NUM_ONONE_ITERATIONS=3',
             '-DLANGUAGE_BENCHMARK_NUM_O_ITERATIONS=30'],
            [x for x in language.cmake_options if 'LANGUAGE_BENCHMARK_NUM' in x])
        self.args.benchmark_num_onone_iterations = 3

        self.args.benchmark_num_onone_iterations = 10
        self.args.benchmark_num_o_iterations = 25
        language = Codira(
            args=self.args,
            toolchain=self.toolchain,
            source_dir='/path/to/src',
            build_dir='/path/to/build')
        self.assertEqual(
            ['-DLANGUAGE_BENCHMARK_NUM_ONONE_ITERATIONS=10',
             '-DLANGUAGE_BENCHMARK_NUM_O_ITERATIONS=25'],
            [x for x in language.cmake_options if 'LANGUAGE_BENCHMARK_NUM' in x])

    def test_force_optimized_typechecker_flags(self):
        self.args.force_optimized_typechecker = True
        language = Codira(
            args=self.args,
            toolchain=self.toolchain,
            source_dir='/path/to/src',
            build_dir='/path/to/build')
        self.assertEqual(
            ['-DLANGUAGE_FORCE_OPTIMIZED_TYPECHECKER:BOOL=TRUE'],
            [x for x in language.cmake_options
             if 'LANGUAGE_FORCE_OPTIMIZED_TYPECHECKER' in x])

    def test_exclusivity_checking_flags(self):
        self.args.enable_stdlibcore_exclusivity_checking = True
        language = Codira(
            args=self.args,
            toolchain=self.toolchain,
            source_dir='/path/to/src',
            build_dir='/path/to/build')
        self.assertEqual(
            ['-DLANGUAGE_STDLIB_ENABLE_STDLIBCORE_EXCLUSIVITY_CHECKING:BOOL='
             'TRUE'],
            [x for x in language.cmake_options
             if 'LANGUAGE_STDLIB_ENABLE_STDLIBCORE_EXCLUSIVITY_CHECKING' in x])

    def test_experimental_differentiable_programming_flags(self):
        self.args.enable_experimental_differentiable_programming = True
        language = Codira(
            args=self.args,
            toolchain=self.toolchain,
            source_dir='/path/to/src',
            build_dir='/path/to/build')
        self.assertEqual(
            ['-DLANGUAGE_ENABLE_EXPERIMENTAL_DIFFERENTIABLE_PROGRAMMING:BOOL='
             'TRUE'],
            [x for x in language.cmake_options
             if 'DLANGUAGE_ENABLE_EXPERIMENTAL_DIFFERENTIABLE_PROGRAMMING' in x])

    def test_experimental_concurrency_flags(self):
        self.args.enable_experimental_concurrency = True
        language = Codira(
            args=self.args,
            toolchain=self.toolchain,
            source_dir='/path/to/src',
            build_dir='/path/to/build')
        self.assertEqual(
            ['-DLANGUAGE_ENABLE_EXPERIMENTAL_CONCURRENCY:BOOL='
             'TRUE'],
            [x for x in language.cmake_options
             if 'DLANGUAGE_ENABLE_EXPERIMENTAL_CONCURRENCY' in x])

    def test_experimental_cxx_interop_flags(self):
        self.args.enable_experimental_cxx_interop = True
        language = Codira(
            args=self.args,
            toolchain=self.toolchain,
            source_dir='/path/to/src',
            build_dir='/path/to/build')
        self.assertEqual(
            ['-DLANGUAGE_ENABLE_EXPERIMENTAL_CXX_INTEROP:BOOL=TRUE'],
            [option for option in language.cmake_options
             if 'DLANGUAGE_ENABLE_EXPERIMENTAL_CXX_INTEROP' in option])

    def test_experimental_cxx_interop_bridging_header_flags(self):
        self.args.enable_cxx_interop_language_bridging_header = True
        language = Codira(
            args=self.args,
            toolchain=self.toolchain,
            source_dir='/path/to/src',
            build_dir='/path/to/build')
        self.assertEqual(
            ['-DLANGUAGE_ENABLE_CXX_INTEROP_LANGUAGE_BRIDGING_HEADER:BOOL=TRUE'],
            [option for option in language.cmake_options
             if 'DLANGUAGE_ENABLE_CXX_INTEROP_LANGUAGE_BRIDGING_HEADER' in option])

    def test_experimental_distributed_flags(self):
        self.args.enable_experimental_distributed = True
        language = Codira(
            args=self.args,
            toolchain=self.toolchain,
            source_dir='/path/to/src',
            build_dir='/path/to/build')
        self.assertEqual(
            ['-DLANGUAGE_ENABLE_EXPERIMENTAL_DISTRIBUTED:BOOL='
             'TRUE'],
            [x for x in language.cmake_options
             if 'DLANGUAGE_ENABLE_EXPERIMENTAL_DISTRIBUTED' in x])

    def test_experimental_observation_flags(self):
        self.args.enable_experimental_observation = True
        language = Codira(
            args=self.args,
            toolchain=self.toolchain,
            source_dir='/path/to/src',
            build_dir='/path/to/build')
        self.assertEqual(
            ['-DLANGUAGE_ENABLE_EXPERIMENTAL_OBSERVATION:BOOL='
             'TRUE'],
            [x for x in language.cmake_options
             if 'DLANGUAGE_ENABLE_EXPERIMENTAL_OBSERVATION' in x])

    def test_backtracing_flags(self):
        self.args.code_enable_backtracing = True
        language = Codira(
            args=self.args,
            toolchain=self.toolchain,
            source_dir='/path/to/src',
            build_dir='/path/to/build')
        self.assertEqual(
            ['-DLANGUAGE_ENABLE_BACKTRACING:BOOL='
             'TRUE'],
            [x for x in language.cmake_options
             if 'DLANGUAGE_ENABLE_BACKTRACING' in x])

    def test_synchronization_flags(self):
        self.args.enable_synchronization = True
        language = Codira(
            args=self.args,
            toolchain=self.toolchain,
            source_dir='/path/to/src',
            build_dir='/path/to/build')
        self.assertEqual(
            ['-DLANGUAGE_ENABLE_SYNCHRONIZATION:BOOL='
             'TRUE'],
            [x for x in language.cmake_options
             if 'DLANGUAGE_ENABLE_SYNCHRONIZATION' in x])

    def test_volatile_flags(self):
        self.args.enable_volatile = True
        language = Codira(
            args=self.args,
            toolchain=self.toolchain,
            source_dir='/path/to/src',
            build_dir='/path/to/build')
        self.assertEqual(
            ['-DLANGUAGE_ENABLE_VOLATILE:BOOL='
             'TRUE'],
            [x for x in language.cmake_options
             if 'DLANGUAGE_ENABLE_VOLATILE' in x])

    def test_runtime_module_flags(self):
        self.args.enable_runtime_module = True
        language = Codira(
            args=self.args,
            toolchain=self.toolchain,
            source_dir='/path/to/src',
            build_dir='/path/to/build')
        self.assertEqual(
            ['-DLANGUAGE_ENABLE_RUNTIME_MODULE:BOOL='
             'TRUE'],
            [x for x in language.cmake_options
             if 'DLANGUAGE_ENABLE_RUNTIME_MODULE' in x])

    def test_freestanding_is_darwin_flags(self):
        self.args.code_freestanding_is_darwin = True
        language = Codira(
            args=self.args,
            toolchain=self.toolchain,
            source_dir='/path/to/src',
            build_dir='/path/to/build')
        self.assertEqual(
            ['-DLANGUAGE_FREESTANDING_IS_DARWIN:BOOL='
             'TRUE'],
            [x for x in language.cmake_options
                if 'LANGUAGE_FREESTANDING_IS_DARWIN' in x])

    def test_build_language_private_stdlib_flags(self):
        self.args.build_language_private_stdlib = False
        language = Codira(
            args=self.args,
            toolchain=self.toolchain,
            source_dir='/path/to/src',
            build_dir='/path/to/build')
        self.assertEqual(
            ['-DLANGUAGE_STDLIB_BUILD_PRIVATE:BOOL='
             'FALSE'],
            [x for x in language.cmake_options
                if 'LANGUAGE_STDLIB_BUILD_PRIVATE' in x])

    def test_language_tools_ld64_lto_codegen_only_for_supporting_targets(self):
        self.args.code_tools_ld64_lto_codegen_only_for_supporting_targets = True
        language = Codira(
            args=self.args,
            toolchain=self.toolchain,
            source_dir='/path/to/src',
            build_dir='/path/to/build')
        self.assertEqual(
            ['-DLANGUAGE_TOOLS_LD64_LTO_CODEGEN_ONLY_FOR_SUPPORTING_TARGETS:BOOL='
             'TRUE'],
            [x for x in language.cmake_options
                if 'LANGUAGE_TOOLS_LD64_LTO_CODEGEN_ONLY_FOR_SUPPORTING_TARGETS' in x])

    def test_language_debuginfo_non_lto_args(self):
        self.args.code_debuginfo_non_lto_args = None
        language = Codira(
            args=self.args,
            toolchain=self.toolchain,
            source_dir='/path/to/src',
            build_dir='/path/to/build')
        self.assertIn(
            '-ULANGUAGE_DEBUGINFO_NON_LTO_ARGS',
            language.cmake_options)

        self.args.code_debuginfo_non_lto_args = []
        language = Codira(
            args=self.args,
            toolchain=self.toolchain,
            source_dir='/path/to/src',
            build_dir='/path/to/build')
        self.assertEqual(
            ['-DLANGUAGE_DEBUGINFO_NON_LTO_ARGS:STRING='],
            [x for x in language.cmake_options
                if 'LANGUAGE_DEBUGINFO_NON_LTO_ARGS' in x])

        self.args.code_debuginfo_non_lto_args = ['-g']
        language = Codira(
            args=self.args,
            toolchain=self.toolchain,
            source_dir='/path/to/src',
            build_dir='/path/to/build')
        self.assertEqual(
            ['-DLANGUAGE_DEBUGINFO_NON_LTO_ARGS:STRING='
             '-g'],
            [x for x in language.cmake_options
                if 'LANGUAGE_DEBUGINFO_NON_LTO_ARGS' in x])

        self.args.code_debuginfo_non_lto_args = ['-gline-tables-only', '-v']
        language = Codira(
            args=self.args,
            toolchain=self.toolchain,
            source_dir='/path/to/src',
            build_dir='/path/to/build')
        self.assertEqual(
            ['-DLANGUAGE_DEBUGINFO_NON_LTO_ARGS:STRING='
             '-gline-tables-only;-v'],
            [x for x in language.cmake_options
                if 'LANGUAGE_DEBUGINFO_NON_LTO_ARGS' in x])

    def test_stdlib_docs_flags(self):
        self.args.build_stdlib_docs = True
        language = Codira(
            args=self.args,
            toolchain=self.toolchain,
            source_dir='/path/to/src',
            build_dir='/path/to/build')
        self.assertEqual(
            ['-DLANGUAGE_STDLIB_BUILD_SYMBOL_GRAPHS:BOOL='
             'TRUE'],
            [x for x in language.cmake_options
             if 'DLANGUAGE_STDLIB_BUILD_SYMBOL_GRAPHS' in x])
