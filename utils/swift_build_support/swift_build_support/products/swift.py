# language_build_support/products/language.py -------------------------*- python -*-
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

from build_language.build_language.constants import LANGUAGE_REPO_NAME

from . import cmark
from . import earlylanguagedriver
from . import libcxx
from . import toolchain
from . import product
from . import staticlanguagelinux
from ..cmake import CMakeOptions


class Codira(product.Product):

    def __init__(self, args, toolchain, source_dir, build_dir):
        product.Product.__init__(self, args, toolchain, source_dir,
                                 build_dir)
        # Add any runtime sanitizer arguments.
        self.cmake_options.extend(self._runtime_sanitizer_flags)

        # Add any compiler vendor cmake flags.
        self.cmake_options.extend(self._compiler_vendor_flags)

        # Add any language version related cmake flags.
        self.cmake_options.extend(self._version_flags)

        # Add benchmark specific flags.
        self.cmake_options.extend(self._benchmark_flags)

        # Generate the compile db.
        self.cmake_options.extend(self._compile_db_flags)

        # Add the flag if we are supposed to force the typechecker to compile
        # with optimization.
        self.cmake_options.extend(self._force_optimized_typechecker_flags)

        # Add any exclusivity checking flags for stdlibcore.
        self.cmake_options.extend(self._stdlibcore_exclusivity_checking_flags)

        # Add experimental differentiable programming flag.
        self.cmake_options.extend(
            self._enable_experimental_differentiable_programming)

        # Add experimental concurrency flag.
        self.cmake_options.extend(self._enable_experimental_concurrency)

        # Add experimental cxx interop flags.
        self.cmake_options.extend(self._enable_experimental_cxx_interop)
        self.cmake_options.extend(self._enable_cxx_interop_language_bridging_header)

        # Add experimental distributed flag.
        self.cmake_options.extend(self._enable_experimental_distributed)

        # Add backtracing flag.
        self.cmake_options.extend(self._enable_backtracing)

        # Add experimental observation flag.
        self.cmake_options.extend(self._enable_experimental_observation)

        # Add synchronization flag.
        self.cmake_options.extend(self._enable_synchronization)

        # Add volatile flag.
        self.cmake_options.extend(self._enable_volatile)

        # Add runtime module flag.
        self.cmake_options.extend(self._enable_runtime_module)

        # Add static vprintf flag
        self.cmake_options.extend(self._enable_stdlib_static_vprintf)

        # Add freestanding related flags.
        self.cmake_options.extend(self._freestanding_is_darwin)

        self.cmake_options.extend(self._build_language_private_stdlib)

        self.cmake_options.extend(self._enable_stdlib_unicode_data)

        self.cmake_options.extend(self._enable_embedded_stdlib)

        self.cmake_options.extend(self._enable_embedded_stdlib_cross_compiling)

        self.cmake_options.extend(self._enable_stdlib_symbol_graphs)

        self.cmake_options.extend(
            self._language_tools_ld64_lto_codegen_only_for_supporting_targets)

        self.cmake_options.extend(
            self._enable_experimental_parser_validation)

        self._handle_language_debuginfo_non_lto_args()

        self.cmake_options.extend(
            self._enable_new_runtime_build)

        self.cmake_options.extend_raw(self.args.extra_language_cmake_options)

    @classmethod
    def product_source_name(cls):
        """product_source_name() -> str

        The name of the source code directory of this product.
        """
        return LANGUAGE_REPO_NAME

    @classmethod
    def is_build_script_impl_product(cls):
        """is_build_script_impl_product -> bool

        Whether this product is produced by build-script-impl.
        """
        return True

    @classmethod
    def is_before_build_script_impl_product(cls):
        return False

    @property
    def _runtime_sanitizer_flags(self):
        sanitizer_list = []
        if self.args.enable_tsan_runtime:
            sanitizer_list += ['Thread']
        if len(sanitizer_list) == 0:
            return []
        return [('LANGUAGE_RUNTIME_USE_SANITIZERS', ';'.join(sanitizer_list))]

    @property
    def _compiler_vendor_flags(self):
        if self.args.compiler_vendor == "none":
            return []

        if self.args.compiler_vendor != "apple":
            raise RuntimeError("Unknown compiler vendor?! Was build-script \
updated without updating language.py?")

        language_compiler_version = ""
        if self.args.code_compiler_version is not None:
            language_compiler_version = self.args.code_compiler_version

        return [
            ('LANGUAGE_VENDOR', 'Apple'),
            ('LANGUAGE_VENDOR_UTI', 'com.apple.compilers.toolchain.code'),

            # This has a default of 3.0, so it should be safe to use here.
            ('LANGUAGE_VERSION', str(self.args.code_user_visible_version)),

            # FIXME: We are matching build-script-impl here. But it seems like
            # bit rot since this flag is specified in another place with the
            # exact same value in build-script-impl.
            ('LANGUAGE_COMPILER_VERSION', str(language_compiler_version)),
        ]

    @property
    def _version_flags(self):
        r = CMakeOptions()
        if self.args.code_compiler_version is not None:
            language_compiler_version = self.args.code_compiler_version
            r.define('LANGUAGE_COMPILER_VERSION', str(language_compiler_version))
        if self.args.clang_compiler_version is not None:
            clang_compiler_version = self.args.clang_compiler_version
            r.define('CLANG_COMPILER_VERSION', str(clang_compiler_version))

        toolchain_version = os.environ.get('TOOLCHAIN_VERSION')
        if toolchain_version:
            r.define('LANGUAGE_TOOLCHAIN_VERSION', toolchain_version)

        return r

    @property
    def _benchmark_flags(self):
        if not self.args.benchmark:
            return []

        onone_iters = self.args.benchmark_num_onone_iterations
        o_iters = self.args.benchmark_num_o_iterations
        return [
            ('LANGUAGE_BENCHMARK_NUM_ONONE_ITERATIONS', onone_iters),
            ('LANGUAGE_BENCHMARK_NUM_O_ITERATIONS', o_iters)
        ]

    @property
    def _compile_db_flags(self):
        return [('CMAKE_EXPORT_COMPILE_COMMANDS', True)]

    @property
    def _force_optimized_typechecker_flags(self):
        return [('LANGUAGE_FORCE_OPTIMIZED_TYPECHECKER:BOOL',
                 self.args.force_optimized_typechecker)]

    @property
    def _stdlibcore_exclusivity_checking_flags(self):
        return [('LANGUAGE_STDLIB_ENABLE_STDLIBCORE_EXCLUSIVITY_CHECKING:BOOL',
                 self.args.enable_stdlibcore_exclusivity_checking)]

    @property
    def _enable_experimental_differentiable_programming(self):
        return [('LANGUAGE_ENABLE_EXPERIMENTAL_DIFFERENTIABLE_PROGRAMMING:BOOL',
                 self.args.enable_experimental_differentiable_programming)]

    @property
    def _enable_experimental_concurrency(self):
        return [('LANGUAGE_ENABLE_EXPERIMENTAL_CONCURRENCY:BOOL',
                 self.args.enable_experimental_concurrency)]

    @property
    def _enable_experimental_cxx_interop(self):
        return [('LANGUAGE_ENABLE_EXPERIMENTAL_CXX_INTEROP:BOOL',
                 self.args.enable_experimental_cxx_interop)]

    @property
    def _enable_cxx_interop_language_bridging_header(self):
        return [('LANGUAGE_ENABLE_CXX_INTEROP_LANGUAGE_BRIDGING_HEADER:BOOL',
                 self.args.enable_cxx_interop_language_bridging_header)]

    @property
    def _enable_experimental_distributed(self):
        return [('LANGUAGE_ENABLE_EXPERIMENTAL_DISTRIBUTED:BOOL',
                 self.args.enable_experimental_distributed)]

    @property
    def _enable_backtracing(self):
        return [('LANGUAGE_ENABLE_BACKTRACING:BOOL',
                 self.args.code_enable_backtracing)]

    @property
    def _enable_experimental_observation(self):
        return [('LANGUAGE_ENABLE_EXPERIMENTAL_OBSERVATION:BOOL',
                 self.args.enable_experimental_observation)]

    @property
    def _enable_synchronization(self):
        return [('LANGUAGE_ENABLE_SYNCHRONIZATION:BOOL',
                 self.args.enable_synchronization)]

    @property
    def _enable_volatile(self):
        return [('LANGUAGE_ENABLE_VOLATILE:BOOL',
                 self.args.enable_volatile)]

    @property
    def _enable_runtime_module(self):
        return [('LANGUAGE_ENABLE_RUNTIME_MODULE:BOOL',
                 self.args.enable_runtime_module)]

    @property
    def _enable_stdlib_static_vprintf(self):
        return [('LANGUAGE_STDLIB_STATIC_PRINT',
                 self.args.build_language_stdlib_static_print)]

    @property
    def _enable_stdlib_unicode_data(self):
        return [('LANGUAGE_STDLIB_ENABLE_UNICODE_DATA',
                 self.args.build_language_stdlib_unicode_data)]

    @property
    def _freestanding_is_darwin(self):
        return [('LANGUAGE_FREESTANDING_IS_DARWIN:BOOL',
                 self.args.code_freestanding_is_darwin)]

    @property
    def _build_language_private_stdlib(self):
        return [('LANGUAGE_STDLIB_BUILD_PRIVATE:BOOL',
                 self.args.build_language_private_stdlib)]

    @property
    def _language_tools_ld64_lto_codegen_only_for_supporting_targets(self):
        return [('LANGUAGE_TOOLS_LD64_LTO_CODEGEN_ONLY_FOR_SUPPORTING_TARGETS:BOOL',
                 self.args.code_tools_ld64_lto_codegen_only_for_supporting_targets)]

    @property
    def _enable_experimental_parser_validation(self):
        return [('LANGUAGE_ENABLE_EXPERIMENTAL_PARSER_VALIDATION:BOOL',
                 self.args.enable_experimental_parser_validation)]

    @property
    def _enable_embedded_stdlib(self):
        return [('LANGUAGE_SHOULD_BUILD_EMBEDDED_STDLIB',
                 self.args.build_embedded_stdlib)]

    @property
    def _enable_embedded_stdlib_cross_compiling(self):
        return [('LANGUAGE_SHOULD_BUILD_EMBEDDED_STDLIB_CROSS_COMPILING',
                 self.args.build_embedded_stdlib_cross_compiling)]

    @property
    def _enable_stdlib_symbol_graphs(self):
        return [('LANGUAGE_STDLIB_BUILD_SYMBOL_GRAPHS:BOOL',
                 self.args.build_stdlib_docs)]

    @property
    def _enable_new_runtime_build(self):
        return [('LANGUAGE_ENABLE_NEW_RUNTIME_BUILD:BOOL',
                 self.args.enable_new_runtime_build)]

    def _handle_language_debuginfo_non_lto_args(self):
        if ('language_debuginfo_non_lto_args' not in self.args
                or self.args.code_debuginfo_non_lto_args is None):
            # Ensure the final value of the variable is determined
            # by the build-script invocation, and not by any value present
            # in CMakeCache.txt (e.g. as a result of previous incremental builds)
            self.cmake_options.undefine('LANGUAGE_DEBUGINFO_NON_LTO_ARGS')
        else:
            self.cmake_options.extend(
                [('LANGUAGE_DEBUGINFO_NON_LTO_ARGS:STRING',
                 ";".join(self.args.code_debuginfo_non_lto_args))])

    @classmethod
    def get_dependencies(cls):
        return [cmark.CMark,
                earlylanguagedriver.EarlyCodiraDriver,
                toolchain.LLVM,
                staticlanguagelinux.StaticCodiraLinuxConfig,
                libcxx.LibCXX]
