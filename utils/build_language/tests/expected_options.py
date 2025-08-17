# This source file is part of the Swift.org open source project
#
# Copyright (c) 2014 - 2020 Apple Inc. and the Swift project authors
# Licensed under Apache License v2.0 with Runtime Library Exception
#
# See https://language.org/LICENSE.txt for license information
# See https://language.org/CONTRIBUTORS.txt for the list of Swift project authors


import multiprocessing
import platform

from build_language import argparse
from build_language import defaults

from language_build_support.language_build_support import targets


__all__ = [
    'HelpOption',
    'SetOption',
    'SetTrueOption',
    'SetFalseOption',
    'DisableOption',
    'EnableOption',
    'ChoicesOption',
    'IntOption',
    'StrOption',
    'PathOption',
    'AppendOption',
    'UnsupportedOption',
    'IgnoreOption',
    'EXPECTED_OPTIONS',
    'EXPECTED_DEFAULTS',
]


# -----------------------------------------------------------------------------

EXPECTED_DEFAULTS = {
    'android': False,
    'android_api_level': '21',
    'android_deploy_device_path': '/data/local/tmp',
    'android_ndk': None,
    'android_arch': 'aarch64',
    'assertions': True,
    'benchmark': False,
    'benchmark_num_o_iterations': 3,
    'benchmark_num_onone_iterations': 3,
    'build_android': False,
    'build_args': [],
    'build_benchmarks': True,
    'build_clang_tools_extra': True,
    'build_compiler_rt': True,
    'build_cygwin': True,
    'build_external_benchmarks': False,
    'build_foundation': False,
    'build_cmark': True,
    'build_language': True,
    'build_toolchain': True,
    '_build_toolchain': True,
    'build_freebsd': True,
    'build_ios': True,
    'build_ios_device': False,
    'build_ios_simulator': False,
    'build_jobs': multiprocessing.cpu_count(),
    'build_libdispatch': False,
    'build_libxml2': False,
    'build_zlib': False,
    'build_curl': False,
    'build_linux': True,
    'build_llbuild': False,
    'build_lldb': False,
    'build_libcxx': False,
    'build_linux_static': False,
    'build_ninja': False,
    'build_lld': True,
    'build_osx': True,
    'build_playgroundsupport': False,
    'build_runtime_with_host_compiler': False,
    'build_stdlib_deployment_targets': ['all'],
    'build_subdir': None,
    'build_language_dynamic_sdk_overlay': platform.system() != "Darwin",
    'build_language_dynamic_stdlib': True,
    'build_language_inspect': False,
    'build_language_private_stdlib': True,
    'build_language_static_sdk_overlay': False,
    'build_language_static_stdlib': False,
    'build_language_stdlib_unittest_extra': False,
    'build_language_stdlib_static_print': False,
    'build_language_stdlib_unicode_data': True,
    'build_embedded_stdlib': True,
    'build_embedded_stdlib_cross_compiling': False,
    'build_language_clang_overlays': True,
    'build_language_remote_mirror': True,
    'build_languagepm': False,
    'build_language_driver': False,
    'build_language_libexec': True,
    'build_language_testing': False,
    'build_language_testing_macros': False,
    'build_early_language_driver': True,
    'build_early_languagesyntax': True,
    'build_languagesyntax': False,
    'build_skstresstester': False,
    'build_languageformat': False,
    'build_languagedocc': False,
    'build_minimalstdlib': False,
    'build_indexstoredb': False,
    'test_indexstoredb_sanitize_all': False,
    'test_sourcekitlsp_sanitize_all': False,
    'build_sourcekitlsp': False,
    'sourcekitlsp_verify_generated_files': False,
    'sourcekitlsp_lint': False,
    'install_toolchain': False,
    'install_static_linux_config': False,
    'install_languagepm': False,
    'install_languagesyntax': False,
    'install_language_testing': False,
    'install_language_testing_macros': False,
    'install_language_driver': False,
    'install_languagedocc': False,
    'install_wasmkit': False,
    'languagesyntax_verify_generated_files': False,
    'languagesyntax_enable_rawsyntax_validation': False,
    'languagesyntax_enable_test_fuzzing': False,
    'install_playgroundsupport': False,
    'install_sourcekitlsp': False,
    'install_languageformat': False,
    'install_skstresstester': False,
    'build_toolchainbenchmarks': False,
    'build_toolchain_only': False,
    'build_tvos': True,
    'build_tvos_device': False,
    'build_tvos_simulator': False,
    'build_variant': 'Debug',
    'build_watchos': True,
    'build_watchos_device': False,
    'build_watchos_simulator': False,
    'build_xros': True,
    'build_xros_device': False,
    'build_xros_simulator': False,
    'build_xctest': False,
    'build_wasmstdlib': False,
    'build_wasmkit': False,
    'cmake_c_launcher': None,
    'cmake_cxx_launcher': None,
    'clang_compiler_version': None,
    'clang_profile_instr_use': None,
    'clang_user_visible_version': defaults.CLANG_USER_VISIBLE_VERSION,
    'clean': False,
    'cmake': None,
    'cmake_generator': 'Ninja',
    'cmark_assertions': True,
    'cmark_build_variant': 'Debug',
    'color_in_tests': True,
    'compiler_vendor': defaults.COMPILER_VENDOR,
    'coverage_db': None,
    'cross_compile_append_host_target_to_destdir': True,
    'cross_compile_deps_path': None,
    'cross_compile_hosts': [],
    'infer_cross_compile_hosts_on_darwin': False,
    'darwin_deployment_version_ios':
        defaults.DARWIN_DEPLOYMENT_VERSION_IOS,
    'darwin_deployment_version_osx':
        defaults.DARWIN_DEPLOYMENT_VERSION_OSX,
    'darwin_deployment_version_tvos':
        defaults.DARWIN_DEPLOYMENT_VERSION_TVOS,
    'darwin_deployment_version_watchos':
        defaults.DARWIN_DEPLOYMENT_VERSION_WATCHOS,
    'darwin_deployment_version_xros':
        defaults.DARWIN_DEPLOYMENT_VERSION_XROS,
    'darwin_test_deployment_version_osx':
        defaults.DARWIN_DEPLOYMENT_VERSION_OSX,
    'darwin_test_deployment_version_ios':
        defaults.DARWIN_DEPLOYMENT_VERSION_IOS,
    'darwin_test_deployment_version_tvos':
        defaults.DARWIN_DEPLOYMENT_VERSION_TVOS,
    'darwin_test_deployment_version_watchos':
        defaults.DARWIN_DEPLOYMENT_VERSION_WATCHOS,
    'darwin_test_deployment_version_xros':
        defaults.DARWIN_DEPLOYMENT_VERSION_XROS,
    'darwin_symroot_path_filters': [],
    'darwin_xcrun_toolchain': None,
    'distcc': False,
    'sccache': False,
    'dry_run': False,
    'dsymutil_jobs': defaults.DSYMUTIL_JOBS,
    'extra_dsymutil_args': [],
    'enable_asan': False,
    'enable_experimental_differentiable_programming': True,
    'enable_experimental_concurrency': True,
    'enable_experimental_cxx_interop': True,
    'enable_cxx_interop_language_bridging_header': True,
    'enable_experimental_distributed': True,
    'enable_experimental_string_processing': True,
    'enable_experimental_observation': True,
    'enable_experimental_parser_validation': True,
    'language_enable_backtracing': True,
    'enable_synchronization': True,
    'enable_volatile': True,
    'enable_runtime_module': True,
    'enable_lsan': False,
    'enable_sanitize_coverage': False,
    'disable_guaranteed_normal_arguments': False,
    'enable_stdlibcore_exclusivity_checking': False,
    'enable_tsan': False,
    'enable_tsan_runtime': False,
    'enable_ubsan': False,
    'export_compile_commands': False,
    'extra_cmake_options': [],
    'extra_toolchain_cmake_options': [],
    'extra_language_args': [],
    'extra_language_cmake_options': [],
    'language_debuginfo_non_lto_args': None,
    'force_optimized_typechecker': False,
    'foundation_build_variant': 'Debug',
    'foundation_tests_build_variant': 'Debug',
    'host_cc': None,
    'host_cxx': None,
    'host_libtool': None,
    'host_lipo': None,
    'host_target': targets.StdlibDeploymentTarget.host_target().name,
    'host_test': False,
    'only_executable_test': False,
    'only_non_executable_test': False,
    'infer_dependencies': False,
    'install_backdeployconcurrency': False,
    'install_prefix': targets.install_prefix(),
    'install_symroot': None,
    'install_destdir': None,
    'install_all': False,
    'ios': False,
    'ios_all': False,
    'legacy_impl': False,
    'libdispatch_build_variant': 'Debug',
    'libxml2_build_variant': 'Debug',
    'linux_archs': None,
    'lit_jobs': multiprocessing.cpu_count(),
    'zlib_build_variant': 'Debug',
    'curl_build_variant': 'Debug',
    'bootstrapping_mode': None,
    'lit_args': '-sv',
    'llbuild_assertions': True,
    'lldb_assertions': True,
    'lldb_build_variant': 'Debug',
    'lldb_build_with_xcode': '0',
    'toolchain_assertions': True,
    'toolchain_build_variant': 'Debug',
    'toolchain_cmake_options': [],
    'toolchain_enable_modules': False,
    'toolchain_include_tests': True,
    'toolchain_ninja_targets': [],
    'toolchain_ninja_targets_for_cross_compile_hosts': [],
    'toolchain_max_parallel_lto_link_jobs':
        defaults.LLVM_MAX_PARALLEL_LTO_LINK_JOBS,
    'toolchain_targets_to_build':
        'X86;ARM;AArch64;PowerPC;SystemZ;Mips;RISCV;WebAssembly;AVR',
    'tsan_libdispatch_test': False,
    'long_test': False,
    'lto_type': None,
    'maccatalyst': False,
    'maccatalyst_ios_tests': False,
    'musl_path': '/usr/local/musl',
    'linux_static_archs': ['x86_64', 'aarch64'],
    'native_clang_tools_path': None,
    'native_toolchain_tools_path': None,
    'native_language_tools_path': None,
    'dump_config': False,
    'reconfigure': False,
    'relocate_xdg_cache_home_under_build_subdir': False,
    'show_sdks': False,
    'skip_build': False,
    'skip_local_build': False,
    'stdlib_deployment_targets': None,
    'stress_test': False,
    'language_analyze_code_coverage': defaults.SWIFT_ANALYZE_CODE_COVERAGE,
    'language_assertions': True,
    'language_build_variant': 'Debug',
    'language_compiler_version': None,
    'language_disable_dead_stripping': False,
    'language_darwin_module_archs': None,
    'language_darwin_supported_archs': None,
    'language_freestanding_is_darwin': False,
    'language_profile_instr_use': None,
    'language_runtime_fixed_backtracer_path': None,
    'language_stdlib_assertions': True,
    'language_stdlib_strict_availability': False,
    'language_stdlib_build_variant': 'Debug',
    'language_tools_ld64_lto_codegen_only_for_supporting_targets': False,
    'language_tools_max_parallel_lto_link_jobs':
        defaults.SWIFT_MAX_PARALLEL_LTO_LINK_JOBS,
    'language_user_visible_version': defaults.SWIFT_USER_VISIBLE_VERSION,
    'build_stdlib_docs': False,
    'preview_stdlib_docs': False,
    'symbols_package': None,
    'clean_libdispatch': True,
    'clean_foundation': True,
    'clean_xctest': True,
    'clean_llbuild': True,
    'clean_languagepm': True,
    'clean_language_driver': True,
    'clean_early_language_driver': False,
    'test': None,
    'test_early_language_driver': None,
    'test_android': False,
    'test_android_host': False,
    'test_cygwin': False,
    'test_freebsd': False,
    'test_ios': False,
    'test_ios_host': False,
    'test_ios_simulator': False,
    'test_linux': False,
    'test_linux_static': False,
    'test_optimize_for_size': None,
    'test_optimize_none_with_implicit_dynamic': None,
    'test_optimized': None,
    'test_osx': False,
    'test_paths': [],
    'test_language_inspect': True,
    'test_tvos': False,
    'test_tvos_host': False,
    'test_tvos_simulator': False,
    'test_watchos': False,
    'test_watchos_host': False,
    'test_watchos_simulator': False,
    'test_xros': False,
    'test_xros_host': False,
    'test_xros_simulator': False,
    'test_playgroundsupport': True,
    'test_cmark': False,
    'test_languagepm': False,
    'test_foundation': False,
    'test_language_driver': False,
    'test_languagesyntax': False,
    'test_indexstoredb': False,
    'test_sourcekitlsp': False,
    'test_skstresstester': False,
    'test_languageformat': False,
    'test_languagedocc': False,
    'test_toolchainbenchmarks': False,
    'test_wasmstdlib': False,
    'tvos': False,
    'tvos_all': False,
    'validation_test': None,
    'verbose_build': False,
    'watchos': False,
    'watchos_all': False,
    'xros': False,
    'xros_all': False,
    'toolchain_install_components': defaults.toolchain_install_components(),
    'clean_install_destdir': False,
    'use_linker': None,
    'enable_new_runtime_build': False,
}


# -----------------------------------------------------------------------------

def _sanitize_option_string(option_string):
    if option_string.startswith('--'):
        return option_string[2:].replace('-', '_')

    if len(option_string) == 2 and option_string[0] == '-':
        return option_string[1]

    raise ValueError('invalid option_string format: ' + option_string)


class _BaseOption(object):

    def __init__(self, option_string, dest=None, default=None):
        if dest is None:
            dest = _sanitize_option_string(option_string)

        if default is None:
            default = EXPECTED_DEFAULTS.get(dest, None)

        self.option_string = option_string
        self.dest = dest
        self.default = default

    def sanitized_string(self):
        return _sanitize_option_string(self.option_string)


class HelpOption(_BaseOption):
    """Option that prints the help message and exits."""

    pass


class SetOption(_BaseOption):
    """Option that accepts no arguments, setting the destination to a
    hard-coded value or None.
    """

    def __init__(self, *args, **kwargs):
        self.value = kwargs.pop('value', None)
        super(SetOption, self).__init__(*args, **kwargs)


class SetTrueOption(_BaseOption):
    """Option that accepts no arguments, setting the destination value to True
    if parsed and defaulting to False otherwise.
    """

    pass


class SetFalseOption(_BaseOption):
    """Option that accepts no arguments, setting the destination value to False
    if parsed and defaulting to True otherwise.
    """

    pass


class EnableOption(_BaseOption):
    """Option that sets the destination to True when parsed and False by default.
    Can be toggled True or False with an optional bool argument.
    """

    pass


class DisableOption(_BaseOption):
    """Option that sets the destination to False when parsed and True by default.
    Can be toggled True or False with an optional bool argument, which is then
    negated. Thus if an option is passed the value 'True' it will set the
    destination to False and vice versa.
    """

    pass


class ChoicesOption(_BaseOption):
    """Option that accepts an argument from a predefined list of choices."""

    def __init__(self, *args, **kwargs):
        self.choices = kwargs.pop('choices', None)
        super(ChoicesOption, self).__init__(*args, **kwargs)


class IntOption(_BaseOption):
    """Option that accepts an int argument."""

    pass


class StrOption(_BaseOption):
    """Option that accepts a str argument."""

    pass


class PathOption(_BaseOption):
    """Option that accepts a path argument."""

    pass


class AppendOption(_BaseOption):
    """Option that can be called more than once to append argument to internal
    list.
    """

    pass


class UnsupportedOption(_BaseOption):
    """Option that is not supported."""

    pass


class IgnoreOption(_BaseOption):
    """Option that should be ignored when generating tests. Instead a test
    should be written manually as the behavior cannot or should not be auto-
    generated.
    """

    pass


class BuildScriptImplOption(_BaseOption):
    """Option that gets forwarded to build-script-impl by migration.py and is
    only listed for disambiguation by argparse.
    """

    pass


# -----------------------------------------------------------------------------

EXPECTED_OPTIONS = [
    # Ignore the help options since they always call sys.exit(0)
    HelpOption('-h', dest='help', default=argparse.SUPPRESS),
    HelpOption('--help', dest='help', default=argparse.SUPPRESS),

    SetOption('--debug', dest='build_variant', value='Debug'),
    SetOption('--debug-cmark', dest='cmark_build_variant', value='Debug'),
    SetOption('--debug-foundation',
              dest='foundation_build_variant', value='Debug'),
    SetOption('--debug-libdispatch',
              dest='libdispatch_build_variant', value='Debug'),
    SetOption('--debug-libxml2', dest='libxml2_build_variant', value='Debug'),
    SetOption('--debug-zlib', dest='zlib_build_variant', value='Debug'),
    SetOption('--debug-curl', dest='curl_build_variant', value='Debug'),
    SetOption('--debug-lldb', dest='lldb_build_variant', value='Debug'),
    SetOption('--lldb-build-with-xcode', dest='lldb_build_with_xcode',
              value='1'),
    SetOption('--lldb-build-with-cmake', dest='lldb_build_with_xcode',
              value='0'),
    SetOption('--debug-toolchain', dest='toolchain_build_variant', value='Debug'),
    SetOption('--debug-language', dest='language_build_variant', value='Debug'),
    SetOption('--debug-language-stdlib',
              dest='language_stdlib_build_variant', value='Debug'),
    SetOption('--eclipse',
              dest='cmake_generator', value='Eclipse CDT4 - Ninja'),
    SetOption('--make', dest='cmake_generator', value='Unix Makefiles'),
    SetOption('--release', dest='build_variant', value='Release'),
    SetOption('--release-debuginfo',
              dest='build_variant', value='RelWithDebInfo'),
    SetOption('--min-size-release',
              dest='build_variant', value='MinSizeRel'),
    SetOption('--xcode', dest='cmake_generator', value='Xcode'),
    SetOption('-R', dest='build_variant', value='Release'),
    SetOption('-d', dest='build_variant', value='Debug'),
    SetOption('-e', dest='cmake_generator', value='Eclipse CDT4 - Ninja'),
    SetOption('-m', dest='cmake_generator', value='Unix Makefiles'),
    SetOption('-r', dest='build_variant', value='RelWithDebInfo'),
    SetOption('-x', dest='cmake_generator', value='Xcode'),

    # FIXME: Convert these options to set_true actions
    SetOption('--assertions', value=True),
    SetOption('--cmark-assertions', value=True),
    SetOption('--lldb-assertions', value=True),
    SetOption('--toolchain-assertions', value=True),
    SetOption('--llbuild-assertions', value=True),
    SetOption('--language-assertions', value=True),
    SetOption('--language-stdlib-assertions', value=True),
    SetOption('-T', dest='validation_test', value=True),
    SetOption('-o', dest='test_optimized', value=True),
    SetOption('-s', dest='test_optimize_for_size', value=True),
    SetOption('-y',
              dest='test_optimize_none_with_implicit_dynamic', value=True),
    SetOption('-t', dest='test', value=True),
    SetOption('-a', dest='assertions', value=True),

    # FIXME: Convert these options to set_false actions
    SetOption('--no-assertions', dest='assertions', value=False),
    SetOption('-A', dest='assertions', value=False),
    SetOption('--no-lldb-assertions', dest='lldb_assertions', value=False),
    SetOption('--no-toolchain-assertions', dest='toolchain_assertions', value=False),
    SetOption('--no-llbuild-assertions',
              dest='llbuild_assertions', value=False),
    SetOption('--no-language-assertions', dest='language_assertions', value=False),
    SetOption('--no-language-stdlib-assertions',
              dest='language_stdlib_assertions', value=False),
    SetOption('--skip-ios', dest='ios', value=False),
    SetOption('--skip-tvos', dest='tvos', value=False),
    SetOption('--skip-watchos', dest='watchos', value=False),
    SetOption('--skip-xros', dest='xros', value=False),
    SetOption('--skip-test-early-language-driver',
              dest='test_early_language_driver', value=False),

    SetOption('--language-stdlib-strict-availability', value=True),
    SetOption('--no-language-stdlib-strict-availability',
              dest='language_stdlib_strict_availability', value=False),

    DisableOption('--no-toolchain-include-tests', dest='toolchain_include_tests'),
    EnableOption('--toolchain-include-tests', dest='toolchain_include_tests'),

    SetTrueOption('--install-back-deploy-concurrency',
                  dest='install_backdeployconcurrency'),
    SetTrueOption('--benchmark'),
    SetTrueOption('--clean'),
    SetTrueOption('--clean-install-destdir'),
    SetTrueOption('--dry-run'),
    SetTrueOption('--dump-config'),
    SetTrueOption('--disable-guaranteed-normal-arguments'),
    SetTrueOption('--enable-stdlibcore-exclusivity-checking'),
    SetTrueOption('--force-optimized-typechecker'),
    SetTrueOption('--ios'),
    SetTrueOption('--llbuild', dest='build_llbuild'),
    SetTrueOption('--lldb', dest='build_lldb'),
    SetTrueOption('--libcxx', dest='build_libcxx'),
    SetTrueOption('--maccatalyst', dest='maccatalyst'),
    SetTrueOption('--maccatalyst-ios-tests', dest='maccatalyst_ios_tests'),
    SetTrueOption('--playgroundsupport', dest='build_playgroundsupport'),
    SetTrueOption('--install-playgroundsupport',
                  dest='install_playgroundsupport'),
    SetTrueOption('--skip-build'),
    SetTrueOption('--languagepm', dest='build_languagepm'),
    SetTrueOption('--language-driver', dest='build_language_driver'),
    SetTrueOption('--languagesyntax', dest='build_languagesyntax'),
    SetTrueOption('--language-testing', dest='build_language_testing'),
    SetTrueOption('--language-testing-macros', dest='build_language_testing_macros'),
    SetTrueOption('--skstresstester', dest='build_skstresstester'),
    SetTrueOption('--languageformat', dest='build_languageformat'),
    SetTrueOption('--languagedocc', dest='build_languagedocc'),
    SetTrueOption('--build-minimal-stdlib', dest='build_minimalstdlib'),
    SetTrueOption('--build-wasm-stdlib', dest='build_wasmstdlib'),
    SetTrueOption('--test-wasm-stdlib', dest='test_wasmstdlib'),
    SetTrueOption('--wasmkit', dest='build_wasmkit'),
    SetTrueOption('--build-stdlib-docs'),
    SetTrueOption('--preview-stdlib-docs'),
    SetTrueOption('-B', dest='benchmark'),
    SetTrueOption('-S', dest='skip_build'),
    SetTrueOption('-b', dest='build_llbuild'),
    SetTrueOption('-c', dest='clean'),
    SetTrueOption('-i', dest='ios'),
    SetTrueOption('-l', dest='build_lldb'),
    SetTrueOption('-n', dest='dry_run'),
    SetTrueOption('-p', dest='build_languagepm'),

    SetTrueOption('--legacy-impl', dest='legacy_impl'),
    SetTrueOption('--infer', dest='infer_dependencies'),
    SetTrueOption('--reconfigure'),

    EnableOption('--android'),
    EnableOption('--build-external-benchmarks'),
    EnableOption('--build-ninja'),
    EnableOption('--build-lld'),
    EnableOption('--build-runtime-with-host-compiler'),
    EnableOption('--build-language-dynamic-sdk-overlay'),
    EnableOption('--build-language-dynamic-stdlib'),
    EnableOption('--build-language-static-sdk-overlay'),
    EnableOption('--build-language-static-stdlib'),
    EnableOption('--build-language-stdlib-unittest-extra'),
    EnableOption('--build-language-stdlib-static-print'),
    EnableOption('--build-toolchain-only'),
    EnableOption('--build-language-private-stdlib'),
    EnableOption('--build-language-stdlib-unicode-data'),
    EnableOption('--build-embedded-stdlib'),
    EnableOption('--build-embedded-stdlib-cross-compiling'),
    EnableOption('--build-language-libexec'),
    EnableOption('--build-language-clang-overlays'),
    EnableOption('--build-language-remote-mirror'),
    EnableOption('--cross-compile-append-host-target-to-destdir'),
    EnableOption('--color-in-tests'),
    EnableOption('--distcc'),
    EnableOption('--sccache'),
    EnableOption('--enable-asan'),
    EnableOption('--enable-experimental-differentiable-programming'),
    EnableOption('--enable-experimental-concurrency'),
    EnableOption('--enable-experimental-cxx-interop'),
    EnableOption('--enable-cxx-interop-language-bridging-header'),
    EnableOption('--enable-experimental-distributed'),
    EnableOption('--enable-experimental-string-processing'),
    EnableOption('--enable-experimental-observation'),
    EnableOption('--enable-experimental-parser-validation'),
    EnableOption('--enable-lsan'),
    EnableOption('--enable-runtime-module'),
    EnableOption('--enable-sanitize-coverage'),
    EnableOption('--enable-tsan'),
    EnableOption('--enable-tsan-runtime'),
    EnableOption('--enable-ubsan'),
    EnableOption('--export-compile-commands'),
    EnableOption('--language-enable-backtracing'),
    EnableOption('--enable-synchronization'),
    EnableOption('--enable-volatile'),
    EnableOption('--foundation', dest='build_foundation'),
    EnableOption('--host-test'),
    EnableOption('--only-executable-test'),
    EnableOption('--only-non-executable-test'),
    EnableOption('--libdispatch', dest='build_libdispatch'),
    EnableOption('--static-libxml2', dest='build_libxml2'),
    EnableOption('--static-zlib', dest='build_zlib'),
    EnableOption('--static-curl', dest='build_curl'),
    EnableOption('--indexstore-db', dest='build_indexstoredb'),
    EnableOption('--test-indexstore-db-sanitize-all',
                 dest='test_indexstoredb_sanitize_all'),
    EnableOption('--sourcekit-lsp', dest='build_sourcekitlsp'),
    EnableOption('--test-sourcekit-lsp-sanitize-all',
                 dest='test_sourcekitlsp_sanitize_all'),
    EnableOption('--sourcekit-lsp-verify-generated-files',
                 dest='sourcekitlsp_verify_generated_files'),
    EnableOption('--sourcekit-lsp-lint',
                 dest='sourcekitlsp_lint'),
    EnableOption('--install-toolchain', dest='install_toolchain'),
    EnableOption('--install-static-linux-config', dest='install_static_linux_config'),
    EnableOption('--install-languagesyntax', dest='install_languagesyntax'),
    EnableOption('--install-language-testing', dest='install_language_testing'),
    EnableOption('--install-language-testing-macros',
                 dest='install_language_testing_macros'),
    EnableOption('--languagesyntax-verify-generated-files',
                 dest='languagesyntax_verify_generated_files'),
    EnableOption('--languagesyntax-enable-rawsyntax-validation',
                 dest='languagesyntax_enable_rawsyntax_validation'),
    EnableOption('--languagesyntax-enable-test-fuzzing',
                 dest='languagesyntax_enable_test_fuzzing'),
    EnableOption('--install-languagepm', dest='install_languagepm'),
    EnableOption('--install-language-driver', dest='install_language_driver'),
    EnableOption('--install-sourcekit-lsp', dest='install_sourcekitlsp'),
    EnableOption('--install-languageformat', dest='install_languageformat'),
    EnableOption('--install-skstresstester', dest='install_skstresstester'),
    EnableOption('--install-languagedocc', dest='install_languagedocc'),
    EnableOption('--install-wasmkit', dest='install_wasmkit'),
    EnableOption('--toolchain-benchmarks', dest='build_toolchainbenchmarks'),
    EnableOption('--language-inspect', dest='build_language_inspect'),
    EnableOption('--tsan-libdispatch-test'),
    EnableOption('--long-test'),
    EnableOption('--show-sdks'),
    EnableOption('--skip-local-build'),
    EnableOption('--stress-test'),
    EnableOption('--test'),
    EnableOption('--test-optimize-for-size'),
    EnableOption('--test-optimize-none-with-implicit-dynamic'),
    EnableOption('--test-optimized'),
    EnableOption('--tvos'),
    EnableOption('--validation-test'),
    EnableOption('--verbose-build'),
    EnableOption('--watchos'),
    EnableOption('--xros'),
    EnableOption('--xctest', dest='build_xctest'),
    EnableOption('--language-disable-dead-stripping'),
    EnableOption('--clean-early-language-driver', dest='clean_early_language_driver'),
    EnableOption('--toolchain-enable-modules'),
    EnableOption('--build-toolchain', dest='_build_toolchain'),

    DisableOption('--skip-build-cmark', dest='build_cmark'),
    DisableOption('--skip-build-toolchain', dest='build_toolchain'),
    DisableOption('--skip-build-language', dest='build_language'),

    DisableOption('--skip-build-android', dest='build_android'),
    DisableOption('--skip-build-benchmarks', dest='build_benchmarks'),
    DisableOption('--skip-build-cygwin', dest='build_cygwin'),
    DisableOption('--skip-build-freebsd', dest='build_freebsd'),
    DisableOption('--skip-build-ios', dest='build_ios'),
    DisableOption('--skip-build-ios-device', dest='build_ios_device'),
    DisableOption('--skip-build-ios-simulator',
                  dest='build_ios_simulator'),
    DisableOption('--skip-build-linux', dest='build_linux'),
    EnableOption('--build-linux-static'),
    DisableOption('--skip-build-osx', dest='build_osx'),
    DisableOption('--skip-build-tvos', dest='build_tvos'),
    DisableOption('--skip-build-tvos-device', dest='build_tvos_device'),
    DisableOption('--skip-build-tvos-simulator',
                  dest='build_tvos_simulator'),
    DisableOption('--skip-build-watchos', dest='build_watchos'),
    DisableOption('--skip-build-watchos-device',
                  dest='build_watchos_device'),
    DisableOption('--skip-build-watchos-simulator',
                  dest='build_watchos_simulator'),
    DisableOption('--skip-build-xros', dest='build_xros'),
    DisableOption('--skip-build-xros-device', dest='build_xros_device'),
    DisableOption('--skip-build-xros-simulator', dest='build_xros_simulator'),
    DisableOption('--skip-clean-libdispatch', dest='clean_libdispatch'),
    DisableOption('--skip-clean-foundation', dest='clean_foundation'),
    DisableOption('--skip-clean-xctest', dest='clean_xctest'),
    DisableOption('--skip-clean-llbuild', dest='clean_llbuild'),
    DisableOption('--skip-early-language-driver', dest='build_early_language_driver'),
    DisableOption('--skip-early-languagesyntax', dest='build_early_languagesyntax'),
    DisableOption('--skip-clean-languagepm', dest='clean_languagepm'),
    DisableOption('--skip-clean-language-driver', dest='clean_language_driver'),
    DisableOption('--skip-test-android', dest='test_android'),
    DisableOption('--skip-test-android-host', dest='test_android_host'),
    DisableOption('--skip-test-cygwin', dest='test_cygwin'),
    DisableOption('--skip-test-freebsd', dest='test_freebsd'),
    DisableOption('--skip-test-ios', dest='test_ios'),
    DisableOption('--skip-test-ios-host', dest='test_ios_host'),
    DisableOption('--skip-test-ios-simulator', dest='test_ios_simulator'),
    DisableOption('--skip-test-linux', dest='test_linux'),
    DisableOption('--skip-test-linux-static', dest='test_linux_static'),
    DisableOption('--skip-test-osx', dest='test_osx'),
    DisableOption('--skip-test-tvos', dest='test_tvos'),
    DisableOption('--skip-test-tvos-host', dest='test_tvos_host'),
    DisableOption('--skip-test-tvos-simulator',
                  dest='test_tvos_simulator'),
    DisableOption('--skip-test-watchos', dest='test_watchos'),
    DisableOption('--skip-test-watchos-host', dest='test_watchos_host'),
    DisableOption('--skip-test-watchos-simulator',
                  dest='test_watchos_simulator'),
    DisableOption('--skip-test-playgroundsupport',
                  dest='test_playgroundsupport'),
    DisableOption('--skip-test-cmark', dest='test_cmark'),
    DisableOption('--skip-test-languagepm', dest='test_languagepm'),
    DisableOption('--skip-test-foundation', dest='test_foundation'),
    DisableOption('--skip-test-language-driver', dest='test_language_driver'),
    DisableOption('--skip-test-languagesyntax', dest='test_languagesyntax'),
    DisableOption('--skip-test-indexstore-db', dest='test_indexstoredb'),
    DisableOption('--skip-test-sourcekit-lsp', dest='test_sourcekitlsp'),
    DisableOption('--skip-test-skstresstester', dest='test_skstresstester'),
    DisableOption('--skip-test-languageformat', dest='test_languageformat'),
    DisableOption('--skip-test-languagedocc', dest='test_languagedocc'),
    DisableOption('--skip-test-toolchain-benchmarks',
                  dest='test_toolchainbenchmarks'),
    DisableOption('--skip-test-language-inspect',
                  dest='test_language_inspect'),
    DisableOption('--skip-test-wasm-stdlib',
                  dest='test_wasmstdlib'),
    DisableOption('--skip-build-clang-tools-extra',
                  dest='build_clang_tools_extra'),
    DisableOption('--skip-build-libxml2', dest='build_libxml2'),
    DisableOption('--skip-build-zlib', dest='build_zlib'),
    DisableOption('--skip-build-curl', dest='build_curl'),
    DisableOption('--skip-build-compiler-rt', dest='build_compiler_rt'),
    DisableOption('--skip-build-lld', dest='build_lld'),

    ChoicesOption('--compiler-vendor',
                  choices=['none', 'apple']),
    ChoicesOption('--language-analyze-code-coverage',
                  choices=['false', 'not-merged', 'merged']),
    ChoicesOption('--android-arch',
                  choices=['armv7', 'aarch64', 'x86_64']),
    ChoicesOption('--foundation-tests-build-type',
                  dest='foundation_tests_build_variant', choices=['Debug', 'Release']),

    StrOption('--android-api-level'),
    StrOption('--build-args'),
    StrOption('--build-stdlib-deployment-targets'),
    StrOption('--darwin-deployment-version-ios'),
    StrOption('--darwin-deployment-version-osx'),
    StrOption('--darwin-deployment-version-tvos'),
    StrOption('--darwin-deployment-version-watchos'),
    StrOption('--darwin-deployment-version-xros'),
    StrOption('--darwin-test-deployment-version-osx'),
    StrOption('--darwin-test-deployment-version-ios'),
    StrOption('--darwin-test-deployment-version-tvos'),
    StrOption('--darwin-test-deployment-version-watchos'),
    StrOption('--darwin-test-deployment-version-xros'),
    DisableOption('--skip-test-xros-host', dest='test_xros_host'),
    DisableOption('--skip-test-xros', dest='test_xros'),
    DisableOption('--skip-test-xros-simulator', dest='test_xros_simulator'),
    StrOption('--darwin-xcrun-toolchain'),
    StrOption('--host-target'),
    StrOption('--lit-args'),
    StrOption('--toolchain-targets-to-build'),
    StrOption('--stdlib-deployment-targets'),
    StrOption('--language-darwin-module-archs'),
    StrOption('--language-darwin-supported-archs'),
    SetTrueOption('--language-freestanding-is-darwin'),
    AppendOption('--extra-language-cmake-options'),

    StrOption('--linux-archs'),
    StrOption('--linux-static-archs'),

    PathOption('--android-deploy-device-path'),
    PathOption('--android-ndk'),
    PathOption('--build-subdir'),
    SetTrueOption('--relocate-xdg-cache-home-under-build-subdir'),
    PathOption('--clang-profile-instr-use'),
    PathOption('--cmake'),
    PathOption('--coverage-db'),
    PathOption('--cross-compile-deps-path'),
    PathOption('--host-cc'),
    PathOption('--host-cxx'),
    PathOption('--host-libtool'),
    PathOption('--host-lipo'),
    PathOption('--install-prefix'),
    PathOption('--install-symroot'),
    PathOption('--install-destdir'),
    EnableOption('--install-all'),
    PathOption('--native-clang-tools-path'),
    PathOption('--native-toolchain-tools-path'),
    PathOption('--native-language-tools-path'),
    PathOption('--symbols-package'),
    PathOption('--cmake-c-launcher'),
    PathOption('--cmake-cxx-launcher'),
    PathOption('--language-profile-instr-use'),
    PathOption('--language-runtime-fixed-backtracer-path'),
    PathOption('--musl-path'),

    IntOption('--benchmark-num-o-iterations'),
    IntOption('--benchmark-num-onone-iterations'),
    IntOption('--jobs', dest='build_jobs'),
    IntOption('--toolchain-max-parallel-lto-link-jobs'),
    IntOption('--language-tools-max-parallel-lto-link-jobs'),
    EnableOption('--language-tools-ld64-lto-codegen-only-for-supporting-targets'),
    IntOption('-j', dest='build_jobs'),
    IntOption('--lit-jobs', dest='lit_jobs'),
    IntOption('--dsymutil-jobs', dest='dsymutil_jobs'),
    AppendOption('--extra-dsymutil-args', dest='extra_dsymutil_args'),

    AppendOption('--cross-compile-hosts'),
    SetTrueOption('--infer-cross-compile-hosts-on-darwin'),
    AppendOption('--extra-cmake-options'),
    AppendOption('--extra-language-args'),
    AppendOption('--language-debuginfo-non-lto-args'),
    AppendOption('--test-paths'),
    AppendOption('--toolchain-ninja-targets'),
    AppendOption('--toolchain-ninja-targets-for-cross-compile-hosts'),
    AppendOption('--toolchain-cmake-options'),
    AppendOption('--extra-toolchain-cmake-options'),
    AppendOption('--darwin-symroot-path-filters'),

    UnsupportedOption('--build-jobs'),
    UnsupportedOption('--common-cmake-options'),
    UnsupportedOption('--only-execute'),
    UnsupportedOption('--skip-test-optimize-for-size'),
    UnsupportedOption('--skip-test-optimize-none-with-implicit-dynamic'),
    UnsupportedOption('--skip-test-optimized'),

    # Options forwarded to build-script-impl
    BuildScriptImplOption('--skip-test-language', dest='impl_skip_test_language'),
    BuildScriptImplOption('--install-language', dest='impl_install_language'),

    # NOTE: LTO flag is a special case that acts both as an option and has
    # valid choices
    SetOption('--lto', dest='lto_type'),
    ChoicesOption('--lto', dest='lto_type', choices=['thin', 'full']),

    SetOption('--bootstrapping', dest='bootstrapping_mode'),
    ChoicesOption('--bootstrapping', dest='bootstrapping_mode',
                  choices=['hosttools', 'bootstrapping',
                           'bootstrapping-with-hostlibs']),

    # NOTE: We'll need to manually test the behavior of these since they
    # validate compiler version strings.
    IgnoreOption('--clang-compiler-version'),
    IgnoreOption('--clang-user-visible-version'),
    IgnoreOption('--language-compiler-version'),
    IgnoreOption('--language-user-visible-version'),

    # TODO: Migrate to unavailable options once new parser is in place
    IgnoreOption('-I'),
    IgnoreOption('--ios-all'),
    IgnoreOption('--tvos-all'),
    IgnoreOption('--watchos-all'),
    IgnoreOption('--xros-all'),

    StrOption('--toolchain-install-components'),
    ChoicesOption('--use-linker', dest='use_linker', choices=['gold', 'lld']),
    EnableOption('--enable-new-runtime-build', dest='enable_new_runtime_build'),
]
