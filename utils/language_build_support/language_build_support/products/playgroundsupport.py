# language_build_support/products/playgroundsupport.py -------------*- python -*-
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
import re

from . import cmark
from . import foundation
from . import libcxx
from . import libdispatch
from . import llbuild
from . import lldb
from . import toolchain
from . import product
from . import language
from . import languagepm
from . import xctest
from .. import shell
from .. import targets


def get_os_spelling(os):
    return {
        'macosx': 'macOS',
        'iphonesimulator': 'iOS',
        'appletvsimulator': 'tvOS',
    }[os]


class PlaygroundSupport(product.Product):
    @classmethod
    def product_source_name(cls):
        return "language-xcode-playground-support"

    @classmethod
    def is_build_script_impl_product(cls):
        return False

    @classmethod
    def is_before_build_script_impl_product(cls):
        return False

    def should_build(self, host_target):
        return self.args.build_playgroundsupport

    def build(self, host_target):
        root = os.path.dirname(os.path.dirname(self.toolchain.languagec))
        language_lib_dir = os.path.join(root, 'lib', 'language')
        (host_os, host_arch) = host_target.split('-')

        with shell.pushd(self.source_dir):
            shell.call([
                "xcodebuild",
                "-configuration", self.args.build_variant,
                "-workspace", "language-xcode-playground-support.xcworkspace",
                "-scheme", "BuildScript-{}".format(get_os_spelling(host_os)),
                "-sdk", host_os,
                "-arch", host_arch,
                "-derivedDataPath", os.path.join(self.build_dir, "DerivedData"),
                "SWIFT_EXEC={}".format(self.toolchain.languagec),
                "SWIFT_LIBRARY_PATH={}/$(PLATFORM_NAME)".format(language_lib_dir),
                "ONLY_ACTIVE_ARCH=NO",
            ])

    def should_test(self, host_target):
        return re.match('macosx', host_target) and \
            self.args.test_playgroundsupport

    def test(self, host_target):
        root = os.path.dirname(os.path.dirname(self.toolchain.languagec))
        language_lib_dir = os.path.join(root, 'lib', 'language')
        (host_os, host_arch) = host_target.split('-')

        with shell.pushd(self.source_dir):
            shell.call([
                "xcodebuild",
                "test",
                # NOTE: this *always* needs to run in Debug configuration
                "-configuration", "Debug",
                "-workspace", "language-xcode-playground-support.xcworkspace",
                "-scheme", "BuildScript-Test-PlaygroundLogger-{}".format(
                    get_os_spelling(host_os)),
                "-sdk", host_os,
                "-arch", host_arch,
                "-derivedDataPath", os.path.join(self.build_dir, "DerivedData"),
                "SWIFT_EXEC={}".format(self.toolchain.languagec),
                "SWIFT_LIBRARY_PATH={}/$(PLATFORM_NAME)".format(language_lib_dir),
                "ONLY_ACTIVE_ARCH=NO",
            ])

    def should_install(self, host_target):
        return self.args.install_playgroundsupport

    def install(self, host_target):
        root = os.path.dirname(os.path.dirname(self.toolchain.languagec))
        language_lib_dir = os.path.join(root, 'lib', 'language')
        (host_os, host_arch) = host_target.split('-')
        toolchain_prefix = \
            targets.darwin_toolchain_prefix(self.args.install_prefix)

        with shell.pushd(self.source_dir):
            shell.call([
                "xcodebuild",
                "install",
                "-configuration", self.args.build_variant,
                "-workspace", "language-xcode-playground-support.xcworkspace",
                "-scheme", "BuildScript-{}".format(get_os_spelling(host_os)),
                "-sdk", host_os,
                "-arch", host_arch,
                "-derivedDataPath", os.path.join(self.build_dir, "DerivedData"),
                "SWIFT_EXEC={}".format(self.toolchain.languagec),
                "SWIFT_LIBRARY_PATH={}/$(PLATFORM_NAME)".format(language_lib_dir),
                "ONLY_ACTIVE_ARCH=NO",
                "DSTROOT={}".format(self.args.install_destdir),
                "TOOLCHAIN_INSTALL_DIR={}".format(toolchain_prefix),
                "BUILD_PLAYGROUND_LOGGER_TESTS=NO",
            ])

    @classmethod
    def get_dependencies(cls):
        return [cmark.CMark,
                toolchain.LLVM,
                libcxx.LibCXX,
                language.Swift,
                lldb.LLDB,
                libdispatch.LibDispatch,
                foundation.Foundation,
                xctest.XCTest,
                llbuild.LLBuild,
                languagepm.SwiftPM]
