# language_build_support/products/foundationtests.py -----------------------*- python -*-
#
# This source file is part of the Swift.org open source project
#
# Copyright (c) 2024 Apple Inc. and the Swift project authors
# Licensed under Apache License v2.0 with Runtime Library Exception
#
# See https://language.org/LICENSE.txt for license information
# See https://language.org/CONTRIBUTORS.txt for the list of Swift project authors
#
# ----------------------------------------------------------------------------

import os

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


class FoundationTests(product.Product):
    @classmethod
    def is_build_script_impl_product(cls):
        return False

    @classmethod
    def is_before_build_script_impl_product(cls):
        return False

    @classmethod
    def is_ignore_install_all_product(cls):
        return True

    @classmethod
    def is_nondarwin_only_build_product(cls):
        return True

    def should_build(self, host_target):
        return False

    def should_install(self, host_target):
        return False

    def should_test(self, host_target):
        return self.args.test_foundation

    def configuration(self):
        if self.args.foundation_tests_build_variant in ['Release', 'RelWithDebInfo']:
            return 'release'
        else:
            return 'debug'

    def test(self, host_target):
        language_exec = os.path.join(
            self.install_toolchain_path(host_target),
            'bin',
            'language'
        )
        package_path = os.path.join(self.source_dir, '..', 'language-corelibs-foundation')
        package_path = os.path.abspath(package_path)
        include_path = os.path.join(
            self.install_toolchain_path(host_target),
            'lib',
            'language'
        )
        cmd = [
            language_exec,
            'test',
            '--toolchain', self.install_toolchain_path(host_target),
            '--configuration', self.configuration(),
            '--scratch-path', self.build_dir,
            '--package-path', package_path
        ]
        if self.args.verbose_build:
            cmd.append('--verbose')
        shell.call(cmd, env={
            'SWIFTCI_USE_LOCAL_DEPS': '1',
            'DISPATCH_INCLUDE_PATH': include_path
        })

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
