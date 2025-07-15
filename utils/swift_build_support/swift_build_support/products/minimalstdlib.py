# language_build_support/products/minimalstdlib.py -----------------*- python -*-
#
# This source file is part of the Codira.org open source project
#
# Copyright (c) 2014 - 2021 Apple Inc. and the Codira project authors
# Licensed under Apache License v2.0 with Runtime Library Exception
#
# See https://language.org/LICENSE.txt for license information
# See https://language.org/CONTRIBUTORS.txt for the list of Codira project authors
#
# ----------------------------------------------------------------------------

import os
import platform

from . import cmake_product
from . import cmark
from . import libcxx
from . import toolchain
from . import language


class MinimalStdlib(cmake_product.CMakeProduct):
    @classmethod
    def product_source_name(cls):
        return os.path.join("language")

    @classmethod
    def is_build_script_impl_product(cls):
        return False

    @classmethod
    def is_before_build_script_impl_product(cls):
        return False

    def should_build(self, host_target):
        return platform.system() == 'Darwin'

    def build(self, host_target):
        build_variant = 'MinSizeRel'
        self.cmake_options.define('CMAKE_BUILD_TYPE:STRING', build_variant)
        self.cmake_options.define(
            'LANGUAGE_STDLIB_BUILD_TYPE:STRING', build_variant)

        build_root = os.path.dirname(self.build_dir)
        toolchain_build_dir = os.path.join(
            '..', build_root, '%s-%s' % ('toolchain', host_target))
        toolchain_cmake_dir = os.path.join(toolchain_build_dir, 'lib', 'cmake', 'toolchain')
        self.cmake_options.define('TOOLCHAIN_DIR:PATH', toolchain_cmake_dir)

        # Use the just-built toolchain
        toolchain_dir = self.install_toolchain_path(host_target)
        self.cmake_options.define('TOOLCHAIN_DIR:PATH', toolchain_dir)
        self.cmake_options.define(
            'LANGUAGE_NATIVE_CLANG_TOOLS_PATH:STRING', toolchain_dir + '/bin')
        self.cmake_options.define(
            'LANGUAGE_NATIVE_TOOLCHAIN_TOOLS_PATH:STRING', toolchain_dir + '/bin')
        self.cmake_options.define(
            'LANGUAGE_NATIVE_LANGUAGE_TOOLS_PATH:STRING', toolchain_dir + '/bin')

        # Build the freestanding variant, with these options
        self.cmake_options.define('LANGUAGE_SDKS:STRING', 'FREESTANDING')
        self.cmake_options.define(
            'LANGUAGE_FREESTANDING_ARCHS:STRING', 'x86_64;arm64')
        self.cmake_options.define('LANGUAGE_FREESTANDING_FLAVOR:STRING', 'apple')
        self.cmake_options.define('LANGUAGE_FREESTANDING_IS_DARWIN:BOOL', 'FALSE')
        self.cmake_options.define(
            'LANGUAGE_FREESTANDING_MODULE_NAME:STRING', 'macos')
        self.cmake_options.define('LANGUAGE_FREESTANDING_SDK:STRING', 'macosx')
        self.cmake_options.define('LANGUAGE_FREESTANDING_DEPLOYMENT_VERSION:STRING',
                                  '11.0')
        self.cmake_options.define(
            'LANGUAGE_FREESTANDING_TRIPLE_NAME:STRING', 'macosx11.0')
        self.cmake_options.define(
            'LANGUAGE_FREESTANDING_AVAILABILITY_NAME:STRING', 'macOS')
        self.cmake_options.define(
            'LANGUAGE_PRIMARY_VARIANT_ARCH:STRING', 'x86_64')
        self.cmake_options.define(
            'LANGUAGE_PRIMARY_VARIANT_SDK:STRING', 'FREESTANDING')
        self.cmake_options.define(
            'LANGUAGE_HOST_TRIPLE:STRING', 'x86_64-apple-macosx10.13')
        self.cmake_options.define('LANGUAGE_HOST_VARIANT', 'macosx')
        self.cmake_options.define('LANGUAGE_HOST_VARIANT_ARCH', 'x86_64')
        self.cmake_options.define('LANGUAGE_HOST_VARIANT_SDK', 'OSX')
        self.cmake_options.define('LANGUAGE_STDLIB_ASSERTIONS:BOOL', 'FALSE')

        # Configure build to only build the stdlib, and only a static version of it
        self.cmake_options.define(
            'LANGUAGE_BUILD_DYNAMIC_SDK_OVERLAY:BOOL', 'FALSE')
        self.cmake_options.define('LANGUAGE_BUILD_DYNAMIC_STDLIB:BOOL', 'FALSE')
        self.cmake_options.define('LANGUAGE_BUILD_REMOTE_MIRROR:BOOL', 'FALSE')
        self.cmake_options.define(
            'LANGUAGE_BUILD_RUNTIME_WITH_HOST_COMPILER:BOOL', 'FALSE')
        self.cmake_options.define(
            'LANGUAGE_BUILD_STATIC_SDK_OVERLAY:BOOL', 'FALSE')
        self.cmake_options.define('LANGUAGE_BUILD_STATIC_STDLIB:BOOL', 'TRUE')
        self.cmake_options.define('LANGUAGE_INCLUDE_TESTS:BOOL', 'TRUE')
        self.cmake_options.define('LANGUAGE_INCLUDE_TOOLS:BOOL', 'FALSE')

        # Stdlib feature flags
        self.cmake_options.define('LANGUAGE_ENABLE_BACKTRACING:BOOL', 'FALSE')
        self.cmake_options.define('LANGUAGE_ENABLE_DISPATCH:BOOL', 'FALSE')
        self.cmake_options.define(
            'LANGUAGE_ENABLE_EXPERIMENTAL_CONCURRENCY:BOOL', 'TRUE')
        self.cmake_options.define(
            'LANGUAGE_ENABLE_EXPERIMENTAL_DIFFERENTIABLE_PROGRAMMING:BOOL', 'FALSE')
        self.cmake_options.define(
            'LANGUAGE_ENABLE_EXPERIMENTAL_DISTRIBUTED:BOOL', 'FALSE')
        self.cmake_options.define(
            'LANGUAGE_ENABLE_EXPERIMENTAL_OBSERVATION:BOOL', 'FALSE')
        self.cmake_options.define('LANGUAGE_ENABLE_REFLECTION:BOOL', 'FALSE')
        self.cmake_options.define(
            'LANGUAGE_ENABLE_RUNTIME_FUNCTION_COUNTERS:BOOL', 'FALSE')
        self.cmake_options.define(
            'LANGUAGE_IMPLICIT_CONCURRENCY_IMPORT:BOOL', 'FALSE')
        self.cmake_options.define(
            'LANGUAGE_RUNTIME_CRASH_REPORTER_CLIENT:BOOL', 'FALSE')
        self.cmake_options.define(
            'LANGUAGE_RUNTIME_ENABLE_LEAK_CHECKER:BOOL', 'FALSE')
        self.cmake_options.define(
            'LANGUAGE_RUNTIME_STATIC_IMAGE_INSPECTION:BOOL', 'FALSE')
        self.cmake_options.define('LANGUAGE_STDLIB_BUILD_PRIVATE:BOOL', 'TRUE')
        self.cmake_options.define('LANGUAGE_STDLIB_TRACING:BOOL', 'FALSE')
        self.cmake_options.define(
            'LANGUAGE_STDLIB_CONCURRENCY_TRACING:BOOL', 'FALSE')
        self.cmake_options.define(
            'LANGUAGE_STDLIB_DISABLE_INSTANTIATION_CACHES:BOOL', 'TRUE')
        self.cmake_options.define(
            'LANGUAGE_STDLIB_ENABLE_DEBUG_PRECONDITIONS_IN_RELEASE', 'FALSE')
        self.cmake_options.define('LANGUAGE_STDLIB_ENABLE_LTO:STRING', 'full')
        self.cmake_options.define(
            'LANGUAGE_STDLIB_ENABLE_OBJC_INTEROP:BOOL', 'FALSE')
        self.cmake_options.define(
            'LANGUAGE_STDLIB_ENABLE_PRESPECIALIZATION:BOOL', 'FALSE')
        self.cmake_options.define(
            'LANGUAGE_STDLIB_ENABLE_STDLIBCORE_EXCLUSIVITY_CHECKING:BOOL', 'FALSE')
        self.cmake_options.define('LANGUAGE_STDLIB_ENABLE_UNICODE_DATA', 'FALSE')
        self.cmake_options.define(
            'LANGUAGE_STDLIB_ENABLE_VECTOR_TYPES:BOOL', 'FALSE')
        self.cmake_options.define(
            'LANGUAGE_STDLIB_EXPERIMENTAL_HERMETIC_SEAL_AT_LINK:BOOL', 'TRUE')
        self.cmake_options.define('LANGUAGE_STDLIB_HAS_ASL:BOOL', 'FALSE')
        self.cmake_options.define('LANGUAGE_STDLIB_HAS_COMMANDLINE:BOOL', 'FALSE')
        self.cmake_options.define(
            'LANGUAGE_STDLIB_HAS_DARWIN_LIBMALLOC:BOOL', 'FALSE')
        self.cmake_options.define('LANGUAGE_STDLIB_HAS_DLADDR:BOOL', 'FALSE')
        self.cmake_options.define('LANGUAGE_STDLIB_HAS_DLSYM:BOOL', 'FALSE')
        self.cmake_options.define('LANGUAGE_STDLIB_HAS_ENVIRON:BOOL', 'FALSE')
        self.cmake_options.define('LANGUAGE_STDLIB_HAS_FILESYSTEM:BOOL', 'FALSE')
        self.cmake_options.define('LANGUAGE_STDLIB_HAS_LOCALE:BOOL', 'FALSE')
        self.cmake_options.define('LANGUAGE_STDLIB_HAS_STDIN:BOOL', 'FALSE')
        self.cmake_options.define(
            'LANGUAGE_STDLIB_HAS_TYPE_PRINTING:BOOL', 'FALSE')
        self.cmake_options.define('LANGUAGE_STDLIB_OS_VERSIONING:BOOL', 'FALSE')
        self.cmake_options.define(
            'LANGUAGE_STDLIB_PASSTHROUGH_METADATA_ALLOCATOR:BOOL', 'TRUE')
        self.cmake_options.define(
            'LANGUAGE_STDLIB_REFLECTION_METADATA:STRING', 'debugger-only')
        self.cmake_options.define(
            'LANGUAGE_STDLIB_SHORT_MANGLING_LOOKUPS:BOOL', 'FALSE')
        self.cmake_options.define('LANGUAGE_STDLIB_SIL_DEBUGGING:BOOL', 'FALSE')
        self.cmake_options.define(
            'LANGUAGE_STDLIB_SINGLE_THREADED_CONCURRENCY:BOOL', 'FALSE')
        self.cmake_options.define('LANGUAGE_STDLIB_STABLE_ABI:BOOL', 'FALSE')
        self.cmake_options.define('LANGUAGE_STDLIB_STATIC_PRINT', 'TRUE')
        self.cmake_options.define(
            'LANGUAGE_STDLIB_SUPPORTS_BACKTRACE_REPORTING:BOOL', 'FALSE')
        self.cmake_options.define(
            'LANGUAGE_STDLIB_SUPPORT_BACK_DEPLOYMENT:BOOL', 'FALSE')
        self.cmake_options.define('LANGUAGE_STDLIB_TRAP_FUNCTION:STRING', '')
        self.cmake_options.define(
            'LANGUAGE_STDLIB_USE_RELATIVE_PROTOCOL_WITNESS_TABLES:BOOL', 'TRUE')
        self.cmake_options.define('LANGUAGE_THREADING_PACKAGE:STRING', 'none')
        self.cmake_options.define(
            'LANGUAGE_STDLIB_OVERRIDABLE_RETAIN_RELEASE:BOOL', 'FALSE')
        self.cmake_options.define(
            'LANGUAGE_STDLIB_TASK_TO_THREAD_MODEL_CONCURRENCY:BOOL', 'TRUE')

        # Build!
        self.build_with_cmake(["language-stdlib-freestanding"], build_variant, [],
                              prefer_native_toolchain=True)

    def should_test(self, host_target):
        return False

    def test(self, host_target):
        raise RuntimeError("Testing not implemented")

    def should_install(self, host_target):
        return False

    @classmethod
    def get_dependencies(cls):
        return [cmark.CMark,
                toolchain.LLVM,
                libcxx.LibCXX,
                language.Codira]
