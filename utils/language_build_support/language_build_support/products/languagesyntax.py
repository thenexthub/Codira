# language_build_support/products/languagesyntax.py --------------------*- python -*-
#
# This source file is part of the Swift.org open source project
#
# Copyright (c) 2014 - 2017 Apple Inc. and the Swift project authors
# Licensed under Apache License v2.0 with Runtime Library Exception
#
# See https://language.org/LICENSE.txt for license information
# See https://language.org/CONTRIBUTORS.txt for the list of Swift project authors
#
# ----------------------------------------------------------------------------

import os

from build_language.build_language.constants import MULTIROOT_DATA_FILE_PATH

from . import cmark
from . import foundation
from . import libcxx
from . import libdispatch
from . import llbuild
from . import toolchain
from . import product
from . import language
from . import languagepm
from . import xctest
from .. import shell


class SwiftSyntax(product.Product):
    @classmethod
    def product_source_name(cls):
        """product_source_name() -> str

        The name of the source code directory of this product.
        """
        return "language-syntax"

    @classmethod
    def is_build_script_impl_product(cls):
        return False

    @classmethod
    def is_before_build_script_impl_product(cls):
        return False

    @classmethod
    def is_languagepm_unified_build_product(cls):
        return True

    def run_languagesyntax_build_script(self, target, command, additional_params=[]):
        script_path = os.path.join(self.source_dir, 'SwiftSyntaxDevUtils')

        build_cmd = [
            os.path.join(self.install_toolchain_path(target), "bin", "language"),
            'run',
        ]
        if self.args.verbose_build:
            build_cmd.append('--vv')
        build_cmd += [
            '--package-path', script_path,
            'language-syntax-dev-utils',
            command,
            '--build-dir', self.build_dir,
            '--multiroot-data-file', MULTIROOT_DATA_FILE_PATH,
            '--toolchain', self.install_toolchain_path(target)
        ]

        if self.is_release():
            build_cmd.append('--release')

        if self.args.languagesyntax_enable_rawsyntax_validation:
            build_cmd.append('--enable-rawsyntax-validation')

        if self.args.languagesyntax_enable_test_fuzzing:
            build_cmd.append('--enable-test-fuzzing')

        if self.args.verbose_build:
            build_cmd.append('--verbose')

        build_cmd.extend(additional_params)

        env = dict(os.environ)
        env["SWIFTCI_USE_LOCAL_DEPS"] = "1"

        shell.call(build_cmd, env=env)

    def should_build(self, host_target):
        return True

    def run_language_syntax_dev_utils(self, host_target, command, arguments=[]):
        language_syntax_dev_utils = os.path.join(self.source_dir, 'SwiftSyntaxDevUtils')

        run_cmd = [
            os.path.join(self.install_toolchain_path(host_target), "bin", "language"),
            'run',
        ]
        if self.args.verbose_build:
            run_cmd.append('--vv')
        run_cmd += [
            '--package-path', language_syntax_dev_utils,
            'language-syntax-dev-utils',
            command
        ]
        run_cmd += arguments
        if self.args.verbose_build:
            run_cmd.append('--verbose')

        env = dict(os.environ)
        env["SWIFTCI_USE_LOCAL_DEPS"] = "1"

        shell.call(run_cmd, env=env)

    def build(self, host_target):
        if self.args.languagesyntax_verify_generated_files:
            self.run_language_syntax_dev_utils(
                host_target,
                "verify-source-code",
                ['--toolchain', self.install_toolchain_path(host_target)]
            )

        self.run_languagesyntax_build_script(target=host_target,
                                          command='build')

    def should_test(self, host_target):
        return self.args.test_languagesyntax

    def test(self, host_target):
        toolchain_build_dir = os.path.join(self.build_dir, '..', 'toolchain-' + host_target)
        toolchain_build_dir = os.path.realpath(toolchain_build_dir)

        self.run_languagesyntax_build_script(target=host_target,
                                          command='test')

    def should_install(self, host_target):
        return self.args.install_languagesyntax

    def install(self, target_name):
        # SwiftSyntax doesn't produce any products that should be installed
        # into the toolchain. All tools using it link against SwiftSyntax
        # statically.
        pass

    @classmethod
    def get_dependencies(cls):
        return [cmark.CMark,
                toolchain.LLVM,
                libcxx.LibCXX,
                language.Swift,
                libdispatch.LibDispatch,
                foundation.Foundation,
                xctest.XCTest,
                llbuild.LLBuild,
                languagepm.SwiftPM]
