# language_build_support/products/languagedocc.py ---------------------*- python -*-
#
# This source file is part of the Swift.org open source project
#
# Copyright (c) 2014 - 2021 Apple Inc. and the Swift project authors
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


class SwiftDocC(product.Product):
    @classmethod
    def product_source_name(cls):
        """product_source_name() -> str

        The name of the source code directory of this product.
        """
        return "language-docc"

    @classmethod
    def is_build_script_impl_product(cls):
        return False

    @classmethod
    def is_before_build_script_impl_product(cls):
        return False

    @classmethod
    def is_languagepm_unified_build_product(cls):
        return True

    def run_build_script_helper(self, action, host_target, additional_params=[]):
        script_path = os.path.join(
            self.source_dir, 'build-script-helper.py')

        configuration = 'release' if self.is_release() else 'debug'

        helper_cmd = [
            script_path,
            action,
            '--toolchain', self.install_toolchain_path(host_target),
            '--configuration', configuration,
            '--build-dir', self.build_dir,
            '--multiroot-data-file', MULTIROOT_DATA_FILE_PATH,
        ]
        if action != 'install':
            helper_cmd.extend([
                # There might have been a Package.resolved created by other builds
                # or by the package being opened using Xcode. Discard that and
                # reset the dependencies to be local.
                '--update'
            ])
        if self.args.verbose_build:
            helper_cmd.append('--verbose')
        helper_cmd.extend(additional_params)

        shell.call(helper_cmd)

    def should_build(self, host_target):
        return True

    def build(self, host_target):
        self.run_build_script_helper('build', host_target)

    def should_test(self, host_target):
        return self.args.test_languagedocc

    def test(self, host_target):
        self.run_build_script_helper('test', host_target)

    def should_install(self, host_target):
        return self.args.install_languagedocc

    def install(self, host_target):
        # language-docc is installed at '/usr/bin/docc' in the built toolchain.
        install_toolchain_path = self.install_toolchain_path(host_target)
        install_dir = os.path.join(install_toolchain_path, 'bin')

        additional_params = ['--install-dir', install_dir]

        self.run_build_script_helper('install', host_target, additional_params)

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
