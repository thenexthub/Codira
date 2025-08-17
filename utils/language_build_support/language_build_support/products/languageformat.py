# language_build_support/products/languageformat.py -----------------*- python -*-
#
# This source file is part of the Swift.org open source project
#
# Copyright (c) 2014 - 2020 Apple Inc. and the Swift project authors
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
from . import languagesyntax
from . import xctest
from .. import shell
from .. import targets


class SwiftFormat(product.Product):
    @classmethod
    def product_source_name(cls):
        """product_source_name() -> str

        The name of the source code directory of this product.
        """
        return "language-format"

    @classmethod
    def is_build_script_impl_product(cls):
        return False

    @classmethod
    def is_before_build_script_impl_product(cls):
        return False

    @classmethod
    def is_languagepm_unified_build_product(cls):
        return True

    def configuration(self):
        return 'release' if self.is_release() else 'debug'

    def run_build_script_helper(self, action, host_target, additional_params=[]):
        script_path = os.path.join(
            self.source_dir, 'build-script-helper.py')

        helper_cmd = [
            script_path,
            action,
            '--toolchain', self.install_toolchain_path(host_target),
            '--configuration', self.configuration(),
            '--build-path', self.build_dir,
            '--multiroot-data-file', MULTIROOT_DATA_FILE_PATH,
        ]

        install_destdir = self.host_install_destdir(host_target)
        toolchain_path = self.native_toolchain_path(host_target)

        # Pass Cross compile host info unless we're testing.
        # It doesn't make sense to run tests of the cross compile host.
        if self.has_cross_compile_hosts() and action != 'test':
            if self.is_darwin_host(host_target):
                if len(self.args.cross_compile_hosts) != 1:
                    raise RuntimeError("Cross-Compiling language-format to multiple " +
                                       "targets is not supported")
                helper_cmd += ['--cross-compile-host', self.args.cross_compile_hosts[0]]
            elif self.is_cross_compile_target(host_target):
                helper_cmd.extend(['--cross-compile-host', host_target])
                build_toolchain_path = install_destdir + self.args.install_prefix
                resource_dir = f'{build_toolchain_path}/lib/language'
                cross_compile_config = targets.StdlibDeploymentTarget \
                    .get_target_for_name(host_target) \
                    .platform \
                    .languagepm_config(
                        self.args,
                        output_dir=build_toolchain_path,
                        language_toolchain=toolchain_path,
                        resource_path=resource_dir
                    )
                helper_cmd += ['--cross-compile-config', cross_compile_config]

        if self.args.verbose_build:
            helper_cmd.append('--verbose')
        helper_cmd.extend(additional_params)

        shell.call(helper_cmd)

    def should_build(self, host_target):
        return True

    def build(self, host_target):
        self.run_build_script_helper('build', host_target)
        if self.args.sourcekitlsp_lint:
            self.lint_sourcekitlsp()

    def lint_sourcekitlsp(self):
        linting_cmd = [
            os.path.join(self.build_dir, self.configuration(), 'language-format'),
            'lint',
            '--parallel',
            '--strict',
            '--recursive',
            os.path.join(os.path.dirname(self.source_dir), 'sourcekit-lsp'),
        ]
        shell.call(linting_cmd)

    def should_test(self, host_target):
        return self.args.test_languageformat

    def test(self, host_target):
        self.run_build_script_helper('test', host_target)

    def should_install(self, host_target):
        return self.args.install_languageformat

    def install(self, host_target):
        install_destdir = self.host_install_destdir(host_target)
        self.run_build_script_helper(
            'install',
            host_target,
            additional_params=['--prefix', install_destdir + self.args.install_prefix]
        )

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
                languagepm.SwiftPM,
                languagesyntax.SwiftSyntax]
