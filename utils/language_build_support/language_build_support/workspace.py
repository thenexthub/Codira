# language_build_support/workspace.py ------------------------------*- python -*-
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
"""
Represent whole source tree and the build directory
"""
# ----------------------------------------------------------------------------

import os.path


class Workspace(object):
    def __init__(self, source_root, build_root):
        self.source_root = source_root
        self.build_root = build_root

    def source_dir(self, path):
        return os.path.join(self.source_root, path)

    def build_dir(self, deployment_target, product):
        return os.path.join(self.build_root,
                            '%s-%s' % (product, deployment_target))

    def languagepm_unified_build_dir(self, deployment_target):
        """ languagepm_unified_build_dir() -> str

        Build directory that all SwiftPM unified build products share.
        """
        return os.path.join(self.build_root,
                            'unified-languagepm-build-%s' %
                            deployment_target)


def compute_build_subdir(args):
    # Create a name for the build directory.
    build_subdir = args.cmake_generator.replace(" ", "_")

    cmark_build_dir_label = args.cmark_build_variant
    if args.cmark_assertions:
        cmark_build_dir_label += "Assert"

    toolchain_build_dir_label = args.toolchain_build_variant
    if args.toolchain_assertions:
        toolchain_build_dir_label += "Assert"

    language_build_dir_label = args.language_build_variant
    if args.language_assertions:
        language_build_dir_label += "Assert"
    if args.language_analyze_code_coverage != "false":
        language_build_dir_label += "Coverage"

    language_stdlib_build_dir_label = args.language_stdlib_build_variant
    if args.language_stdlib_assertions:
        language_stdlib_build_dir_label += "Assert"

    # FIXME: mangle LLDB build configuration into the directory name.
    if (toolchain_build_dir_label == language_build_dir_label and
            toolchain_build_dir_label == language_stdlib_build_dir_label and
            language_build_dir_label == cmark_build_dir_label):
        # Use a simple directory name if all projects use the same build
        # type.
        build_subdir += "-" + toolchain_build_dir_label
    elif (toolchain_build_dir_label != language_build_dir_label and
            toolchain_build_dir_label == language_stdlib_build_dir_label and
            language_build_dir_label == cmark_build_dir_label):
        # Swift build type differs.
        build_subdir += "-" + toolchain_build_dir_label
        build_subdir += "+language-" + language_build_dir_label
    elif (toolchain_build_dir_label == language_build_dir_label and
            toolchain_build_dir_label != language_stdlib_build_dir_label and
            language_build_dir_label == cmark_build_dir_label):
        # Swift stdlib build type differs.
        build_subdir += "-" + toolchain_build_dir_label
        build_subdir += "+stdlib-" + language_stdlib_build_dir_label
    elif (toolchain_build_dir_label == language_build_dir_label and
            toolchain_build_dir_label == language_stdlib_build_dir_label and
            language_build_dir_label != cmark_build_dir_label):
        # cmark build type differs.
        build_subdir += "-" + toolchain_build_dir_label
        build_subdir += "+cmark-" + cmark_build_dir_label
    else:
        # We don't know how to create a short name, so just mangle in all
        # the information.
        build_subdir += "+cmark-" + cmark_build_dir_label
        build_subdir += "+toolchain-" + toolchain_build_dir_label
        build_subdir += "+language-" + language_build_dir_label
        build_subdir += "+stdlib-" + language_stdlib_build_dir_label

    # If we have a sanitizer enabled, mangle it into the subdir.
    if args.enable_asan:
        build_subdir += "+asan"
    if args.enable_ubsan:
        build_subdir += "+ubsan"
    if args.enable_tsan:
        build_subdir += "+tsan"
    return build_subdir


def relocate_xdg_cache_home_under(new_cache_location):
    """
    This allows under Linux to relocate the default location
    of the module cache -- this can be useful when there are
    are lot of invocations to touch or when some invocations
    can't be easily configured (as is the case for Swift
    compiler detection in CMake)
    """
    os.environ['XDG_CACHE_HOME'] = new_cache_location
