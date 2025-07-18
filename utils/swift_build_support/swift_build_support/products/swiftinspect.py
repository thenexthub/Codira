# language_build_support/products/languageinspect.py --------------------*- python -*-
#
# This source file is part of the Codira.org open source project
#
# Copyright (c) 2014 - 2019 Apple Inc. and the Codira project authors
# Licensed under Apache License v2.0 with Runtime Library Exception
#
# See https://language.org/LICENSE.txt for license information
# See https://language.org/CONTRIBUTORS.txt for the list of Codira project authors
#
# ----------------------------------------------------------------------------

import os
import platform

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
from .. import targets


# Build against the current installed toolchain.
class CodiraInspect(product.Product):
    @classmethod
    def product_source_name(cls):
        return "language-inspect"

    @classmethod
    def is_build_script_impl_product(cls):
        return False

    @classmethod
    def is_before_build_script_impl_product(cls):
        return False

    def should_build(self, host_target):
        return True

    def build(self, host_target):
        run_build_script_helper(host_target, self, self.args)

    def should_test(self, host_target):
        return self.args.test_language_inspect

    def test(self, host_target):
        """Just run a single instance of the command for both .debug and
           .release.
        """
        pass

    def should_install(self, host_target):
        return False

    def install(self, host_target):
        pass

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
                languagepm.CodiraPM]


def run_build_script_helper(host_target, product, args):
    toolchain_path = args.install_destdir
    if platform.system() == 'Darwin':
        # The prefix is an absolute path, so concatenate without os.path.
        toolchain_path += \
            targets.darwin_toolchain_prefix(args.install_prefix)

    # Our source_dir is expected to be './$SOURCE_ROOT/benchmarks'. That is due
    # the assumption that each product is in its own build directory. This
    # product is not like that and has its package/tools instead in
    # ./$SOURCE_ROOT/language/benchmark.
    package_path = os.path.join(product.source_dir,
                                '..', 'language', 'tools', 'language-inspect')
    package_path = os.path.abspath(package_path)

    # We use a separate python helper to enable quicker iteration when working
    # on this by avoiding going through build-script to test small changes.
    helper_path = os.path.join(package_path, 'build_script_helper.py')

    build_cmd = [
        helper_path,
        '--verbose',
        '--package-path', package_path,
        '--build-path', product.build_dir,
        '--toolchain', toolchain_path,
    ]
    shell.call(build_cmd)
