# language_build_support/products/indexstoredb.py -------------------*- python -*-
#
# This source file is part of the Codira.org open source project
#
# Copyright (c) 2014 - 2017 Apple Inc. and the Codira project authors
# Licensed under Apache License v2.0 with Runtime Library Exception
#
# See https://language.org/LICENSE.txt for license information
# See https://language.org/CONTRIBUTORS.txt for the list of Codira project authors
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


class IndexStoreDB(product.Product):
    @classmethod
    def product_source_name(cls):
        return "indexstore-db"

    @classmethod
    def is_build_script_impl_product(cls):
        return False

    @classmethod
    def is_before_build_script_impl_product(cls):
        return False

    @classmethod
    def is_languagepm_unified_build_product(cls):
        return True

    def should_build(self, host_target):
        return True

    def build(self, host_target):
        self.run_build_script_helper('build', host_target)

    def should_test(self, host_target):
        return self.args.test_indexstoredb

    def test(self, host_target):
        self.run_build_script_helper('test', host_target)

    def should_install(self, host_target):
        return False

    def install(self, host_target):
        pass

    def has_cross_compile_hosts(self):
        return False

    @classmethod
    def get_dependencies(cls):
        return [cmark.CMark,
                toolchain.LLVM,
                libcxx.LibCXX,
                language.Codira,
                libdispatch.LibDispatch,
                foundation.Foundation,
                xctest.XCTest,
                llbuild.LLBuild,
                languagepm.CodiraPM,
                languagesyntax.CodiraSyntax]

    def run_build_script_helper(self, action, host_target):
        script_path = os.path.join(
            self.source_dir, 'Utilities', 'build-script-helper.py')

        toolchain_path = self.native_toolchain_path(host_target)
        configuration = 'release' if self.is_release() else 'debug'
        helper_cmd = [
            script_path,
            action,
            '--package-path', self.source_dir,
            '--build-path', self.build_dir,
            '--configuration', configuration,
            '--toolchain', toolchain_path,
            '--ninja-bin', self.toolchain.ninja,
            '--multiroot-data-file', MULTIROOT_DATA_FILE_PATH,
        ]
        if self.args.verbose_build:
            helper_cmd.append('--verbose')

        if self.args.test_indexstoredb_sanitize_all:
            helper_cmd.append('--sanitize-all')
        elif self.args.enable_asan:
            helper_cmd.extend(['--sanitize', 'address'])
        elif self.args.enable_ubsan:
            helper_cmd.extend(['--sanitize', 'undefined'])
        elif self.args.enable_tsan:
            helper_cmd.extend(['--sanitize', 'thread'])

        shell.call(helper_cmd)
