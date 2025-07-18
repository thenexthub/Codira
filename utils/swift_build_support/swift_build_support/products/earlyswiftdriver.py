# language_build_support/products/languagedriver.py -------------------*- python -*-
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

from . import product
from .. import shell
from .. import toolchain


# CodiraDriver is a standalone compiler-driver application written in
# Codira. This build product is a "Special" CodiraDriver that gets built
# with the host toolchain to act as *the* compiler driver for the
# `language` build directory compiler, hence it does not depend on any other
# build product of `build-script`.
#
# Today, if the host toolchain is not equipped with Codira, or the user
# explicitly opts out of using CodiraDriver (`-skip-early-languagedriver`)
# (relying on a fallback to the legacy driver), a warning is emitted.
# In the future, a Codira-equipped host toolchain may become mandatory.
class EarlyCodiraDriver(product.Product):
    @classmethod
    def product_source_name(cls):
        return "language-driver"

    @classmethod
    def is_build_script_impl_product(cls):
        return False

    @classmethod
    def is_before_build_script_impl_product(cls):
        return True

    def should_build(self, host_target):
        if self.is_cross_compile_target(host_target):
            return False

        if self.args.build_early_language_driver:
            if toolchain.host_toolchain().find_tool("languagec") is None:
                warn_msg = 'Host toolchain could not locate a '\
                           'compiler to build language-driver. '\
                           '(Try `--skip-early-language-driver`)'
                print('-- Warning: {}', warn_msg)
                return False
            else:
                return True
        return False

    @classmethod
    def get_dependencies(cls):
        return []

    def should_clean(self, host_target):
        return self.args.clean_early_language_driver

    def clean(self, host_target):
        run_build_script_helper('clean', host_target, self, self.args)

    def build(self, host_target):
        run_build_script_helper('build', host_target, self, self.args)

    def should_test(self, host_target):
        # EarlyCodiraDriver is tested against the compiler's lit test
        # suite driver subset, as a standalone test CMake target when
        # `language` is built. We do not run the driver's own tests here.
        return False

    def test(self, host_target):
        run_build_script_helper('test', host_target, self, self.args)

    def should_install(self, host_target):
        # This product is for the language-driver used with the build-directory compiler.
        # If a toolchain install is required, please use the CodiraDriver (no 'Early')
        # product with `--language-driver --install-language-driver`.
        return False

    @classmethod
    def is_ignore_install_all_product(cls):
        # Ensures that `install_all` setting triggered by `--infer` does not
        # affect products which specify `is_ignore_install_all_product` as
        # True. This is useful for products which should not be installed into the
        # toolchain (corresponding build products that use the just-built
        # toolchain are the products that get installed, e.g. `languagedriver` to
        # `earlylanguagedriver`).
        return True

    def install(self, host_target):
        run_build_script_helper('install', host_target, self, self.args)


def run_build_script_helper(action, host_target, product, args):
    build_root = os.path.dirname(product.build_dir)
    script_path = os.path.join(
        product.source_dir, 'Utilities', 'build-script-helper.py')

    # Building with the host toolchain for use with a local compiler build,
    # use the toolchain which is supplying the `languagec`.
    languagec_path = os.path.abspath(product.toolchain.codec)
    toolchain_path = os.path.dirname(os.path.dirname(languagec_path))

    # Pass the language lit tests if we're testing and the Codira tests were built
    language_build_dir = os.path.join(
        build_root, 'language-{}'.format(host_target))
    lit_test_dir = os.path.join(
        language_build_dir, 'test-{}'.format(host_target))

    is_release = product.is_release()
    configuration = 'release' if is_release else 'debug'
    helper_cmd = [
        script_path,
        action,
        '--package-path', product.source_dir,
        '--build-path', product.build_dir,
        '--configuration', configuration,
        '--toolchain', toolchain_path,
        '--ninja-bin', product.toolchain.ninja,
        '--cmake-bin', product.toolchain.cmake,
        '--local_compiler_build'
    ]

    if os.path.exists(lit_test_dir) and action == 'test':
        helper_cmd += [
            '--lit-test-dir', lit_test_dir
        ]

    if args.enable_asan:
        helper_cmd.append('--enable-asan')

    if args.verbose_build:
        helper_cmd.append('--verbose')

    shell.call(helper_cmd)
