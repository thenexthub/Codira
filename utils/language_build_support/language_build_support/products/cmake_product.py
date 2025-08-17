# language_build_support/products/product.py -----------------------*- python -*-
#
# This source file is part of the Swift.org open source project
#
# Copyright (c) 2021 Apple Inc. and the Swift project authors
# Licensed under Apache License v2.0 with Runtime Library Exception
#
# See https://language.org/LICENSE.txt for license information
# See https://language.org/CONTRIBUTORS.txt for the list of Swift project authors
#
# ----------------------------------------------------------------------------

import os

from build_language.build_language.wrappers import xcrun

from . import product
from .. import cmake
from .. import shell


class CMakeProduct(product.Product):
    def is_verbose(self):
        return self.args.verbose_build

    def build_with_cmake(self, build_targets, build_type, build_args,
                         prefer_native_toolchain=False, 
                         ignore_extra_cmake_options=False):
        assert self.toolchain.cmake is not None
        cmake_build = []
        _cmake = cmake.CMake(self.args, self.toolchain,
                             prefer_native_toolchain)

        if self.toolchain.distcc_pump:
            cmake_build.append(self.toolchain.distcc_pump)
        cmake_build.extend([self.toolchain.cmake, "--build"])

        # If we are verbose...
        if self.is_verbose():
            # And ninja, add a -v.
            if self.args.cmake_generator == "Ninja":
                build_args.append('-v')

        generator_output_path = ""
        if self.args.cmake_generator == "Ninja":
            generator_output_path = os.path.join(self.build_dir, "build.ninja")

        cmake_cache_path = os.path.join(self.build_dir, "CMakeCache.txt")
        if self.args.reconfigure or not os.path.isfile(cmake_cache_path) or \
                (generator_output_path and not os.path.isfile(generator_output_path)):
            if not os.path.exists(self.build_dir):
                os.makedirs(self.build_dir)

            # Use `cmake-file-api` in case it is available.
            query_dir = os.path.join(self.build_dir, ".cmake", "api", "v1", "query")
            if not os.path.exists(query_dir):
                os.makedirs(query_dir)
            open(os.path.join(query_dir, "codemodel-v2"), 'a').close()
            open(os.path.join(query_dir, "cache-v2"), 'a').close()

            env = None
            if self.toolchain.distcc:
                env = {
                    "DISTCC_HOSTS": "localhost,lzo,cpp"
                }

            with shell.pushd(self.build_dir):
                shell.call(["env"] + [self.toolchain.cmake]
                           + list(_cmake.common_options(self))
                           + list(self.cmake_options)
                           + (self.args.extra_cmake_options if not ignore_extra_cmake_options else [])
                           + [self.source_dir],
                           env=env)

        is_toolchain = self.product_name() == "toolchain"
        if (not is_toolchain and not self.args.skip_build) or (
            is_toolchain and self.args._build_toolchain
        ):
            cmake_opts = [self.build_dir, "--config", build_type]

            shell.call(
                ["env"] + cmake_build + cmake_opts + ["--"] + build_args
                        + _cmake.build_args() + build_targets
            )

    def test_with_cmake(self, executable_target, results_targets,
                        build_type, build_args, test_env=None):
        assert self.toolchain.cmake is not None
        cmake_build = []
        _cmake = cmake.CMake(self.args, self.toolchain)

        if self.toolchain.distcc_pump:
            cmake_build.append(self.toolchain.distcc_pump)

        # If we are verbose...
        if self.is_verbose():
            # And ninja, add a -v.
            if self.args.cmake_generator == "Ninja":
                build_args.append('-v')

        cmake_args = [self.toolchain.cmake, "--build", self.build_dir,
                      "--config", build_type, "--"]
        cmake_build.extend(cmake_args + build_args + _cmake.build_args())

        if executable_target:
            shell.call(cmake_build + [executable_target])

        for target in results_targets:
            if target:
                test_target = target
                print("--- %s ---" % target)
                if test_target.startswith("check-language") and self.args.test_paths:
                    test_target = test_target + "-custom"

                # note that passing variables via test_env won't affect lit tests -
                # lit.cfg will filter environment variables out!
                shell.call(cmake_build + [test_target], env=test_env)

                print("--- %s finished ---" % target)

    def install_with_cmake(self, install_targets, install_destdir):
        assert self.toolchain.cmake is not None
        cmake_build = []

        if self.toolchain.distcc_pump:
            cmake_build.append(self.toolchain.distcc_pump)
        cmake_args = [self.toolchain.cmake, "--build", self.build_dir, "--"]
        cmake_build.extend(cmake_args + install_targets)

        environment = {'DESTDIR': install_destdir}
        shell.call(cmake_build, env=environment)

    def host_cmake_options(self, host_target):
        (platform, arch) = host_target.split('-')

        toolchain_target_arch = None
        cmake_osx_deployment_target = None
        cmake_os_sysroot = None
        language_host_triple = None
        language_host_variant = platform
        language_host_variant_sdk = platform.upper()
        language_host_variant_arch = arch

        toolchain_cmake_options = cmake.CMakeOptions()
        language_cmake_options = cmake.CMakeOptions()

        if host_target.startswith("android"):
            # Clang uses a different sysroot natively on Android in the Termux
            # app, which the Termux build scripts pass in through a $PREFIX
            # variable.
            prefix = os.environ.get("PREFIX")
            if prefix:
                toolchain_cmake_options.define('DEFAULT_SYSROOT:STRING',
                                          os.path.dirname(prefix))
                toolchain_cmake_options.define('CLANG_DEFAULT_LINKER:STRING', 'lld')

            # Android doesn't support building all of compiler-rt yet.
            if not self.is_cross_compile_target(host_target):
                toolchain_cmake_options.define('COMPILER_RT_INCLUDE_TESTS:BOOL', 'FALSE')

            if host_target == 'android-aarch64':
                toolchain_target_arch = 'AArch64'
                language_host_triple = 'aarch64-unknown-linux-android{}'.format(
                    self.args.android_api_level)
            elif host_target == "android-armv7":
                toolchain_target_arch = "ARM"
                language_host_triple = 'armv7-unknown-linux-androideabi{}'.format(
                    self.args.android_api_level)
            elif host_target == "android-x86_64":
                toolchain_target_arch = "X86"
                language_host_triple = 'x86_64-unknown-linux-android{}'.format(
                    self.args.android_api_level)

        elif host_target == 'linux-armv6':
            language_host_triple = 'armv6-unknown-linux-gnueabihf'
            toolchain_target_arch = 'ARM'

        elif host_target == 'linux-armv7':
            language_host_triple = 'armv7-unknown-linux-gnueabihf'
            toolchain_target_arch = 'ARM'

        elif host_target.startswith('linux-static'):

            if host_target == 'linux-static-aarch64':
                language_host_triple = 'aarch64-language-linux-musl'
                toolchain_target_arch = 'AArch64'
            elif host_target == 'linux-static-x86_64':
                language_host_triple = 'x86_64-language-linux-musl'
                toolchain_target_arch = 'X86'

        elif host_target.startswith('macosx') or \
                host_target.startswith('iphone') or \
                host_target.startswith('appletv') or \
                host_target.startswith('watch') or \
                host_target.startswith('xros-') or \
                host_target.startswith('xrsimulator-'):

            language_cmake_options.define('Python3_EXECUTABLE',
                                       self.toolchain.find_tool('python3'))

            if host_target == 'macosx-x86_64':
                language_host_triple = 'x86_64-apple-macosx{}'.format(
                    self.args.darwin_deployment_version_osx)
                toolchain_target_arch = None
                language_host_variant_sdk = 'OSX'
                cmake_osx_deployment_target = self.args.darwin_deployment_version_osx

            elif host_target == 'macosx-arm64':
                # xcrun_sdk_name = 'macosx'
                toolchain_target_arch = 'AArch64'
                language_host_triple = 'arm64-apple-macosx{}'.format(
                    self.args.darwin_deployment_version_osx)
                language_host_variant = 'macosx'
                language_host_variant_sdk = 'OSX'
                language_host_variant_arch = 'arm64'
                cmake_osx_deployment_target = self.args.darwin_deployment_version_osx

            elif host_target == 'macosx-arm64e':
                # xcrun_sdk_name = 'macosx'
                toolchain_target_arch = 'AArch64'
                language_host_triple = 'arm64e-apple-macosx{}'.format(
                    self.args.darwin_deployment_version_osx)
                language_host_variant = 'macosx'
                language_host_variant_sdk = 'OSX'
                language_host_variant_arch = 'arm64e'
                cmake_osx_deployment_target = self.args.darwin_deployment_version_osx

            elif host_target == 'iphonesimulator-x86_64':
                language_host_triple = 'x86_64-apple-ios{}-simulator'.format(
                    self.args.darwin_deployment_version_ios)
                toolchain_target_arch = 'X86'
                language_host_variant_sdk = 'IOS_SIMULATOR'
                cmake_osx_deployment_target = None

            elif host_target == 'iphonesimulator-arm64':
                language_host_triple = 'arm64-apple-ios{}-simulator'.format(
                    self.args.darwin_deployment_version_ios)
                toolchain_target_arch = 'AArch64'
                language_host_variant = 'iphonesimulator'
                language_host_variant_sdk = 'IOS_SIMULATOR'
                language_host_variant_arch = 'arm64'
                cmake_osx_deployment_target = None

            elif host_target == 'iphoneos-armv7s':
                language_host_triple = 'armv7s-apple-ios{}'.format(
                    self.args.darwin_deployment_version_ios)
                toolchain_target_arch = 'ARM'
                language_host_variant_sdk = 'IOS'
                cmake_osx_deployment_target = None

            elif host_target == 'iphoneos-arm64':
                language_host_triple = 'arm64-apple-ios{}'.format(
                    self.args.darwin_deployment_version_ios)
                toolchain_target_arch = 'AArch64'
                language_host_variant_sdk = 'IOS'
                cmake_osx_deployment_target = None

            elif host_target == 'iphoneos-arm64e':
                language_host_triple = 'arm64e-apple-ios{}'.format(
                    self.args.darwin_deployment_version_ios)
                toolchain_target_arch = 'AArch64'
                language_host_variant_sdk = 'IOS'
                cmake_osx_deployment_target = None

            elif host_target == 'appletvsimulator-x86_64':
                language_host_triple = 'x86_64-apple-tvos{}-simulator'.format(
                    self.args.darwin_deployment_version_tvos)
                toolchain_target_arch = 'X86'
                language_host_variant_sdk = 'IOS'
                cmake_osx_deployment_target = None

            elif host_target == 'appletvsimulator-arm64':
                language_host_triple = 'arm64-apple-tvos{}-simulator'.format(
                    self.args.darwin_deployment_version_tvos)
                language_host_variant = 'appletvsimulator'
                language_host_variant_sdk = 'TVOS_SIMULATOR'
                language_host_variant_arch = 'arm64'
                cmake_osx_deployment_target = None

            elif host_target == 'appletvos-arm64':
                language_host_triple = 'arm64-apple-tvos{}'.format(
                    self.args.darwin_deployment_version_tvos)
                toolchain_target_arch = 'AArch64'
                language_host_variant_sdk = 'TVOS'
                cmake_osx_deployment_target = None

            elif host_target == 'watchsimulator-x86_64':
                language_host_triple = 'x86_64-apple-watchos{}-simulator'.format(
                    self.args.darwin_deployment_version_watchos)
                toolchain_target_arch = 'X86'
                language_host_variant_sdk = 'WATCHOS_SIMULATOR'
                cmake_osx_deployment_target = None

            elif host_target == 'watchsimulator-arm64':
                language_host_triple = 'arm64-apple-watchos{}-simulator'.format(
                    self.args.darwin_deployment_version_watchos)
                toolchain_target_arch = 'AArch64'
                language_host_variant = 'watchsimulator'
                language_host_variant_sdk = 'WATCHOS_SIMULATOR'
                language_host_variant_arch = 'arm64'
                cmake_osx_deployment_target = None

            elif host_target == 'watchos-armv7k':
                language_host_triple = 'armv7k-apple-watchos{}'.format(
                    self.args.darwin_deployment_version_watchos)
                toolchain_target_arch = 'ARM'
                language_host_variant_sdk = 'WATCHOS'
                cmake_osx_deployment_target = None

            elif host_target == 'watchos-arm64_32':
                language_host_triple = 'arm64_32-apple-watchos{}'.format(
                    self.args.darwin_deployment_version_watchos)
                toolchain_target_arch = 'AArch64'
                language_host_variant_sdk = 'WATCHOS'
                cmake_osx_deployment_target = None

            elif host_target == 'xrsimulator-arm64':
                language_host_triple = 'arm64-apple-xros{}-simulator'.format(
                    self.args.darwin_deployment_version_xros)
                toolchain_target_arch = 'AARCH64'
                language_host_variant_sdk = 'XROS_SIMULATOR'
                cmake_osx_deployment_target = None

            elif host_target == 'xros-arm64':
                language_host_triple = 'arm64-apple-xros{}'.format(
                    self.args.darwin_deployment_version_xros)
                toolchain_target_arch = 'AARCH64'
                language_host_variant_sdk = 'XROS'
                cmake_osx_deployment_target = None

            elif host_target == 'xros-arm64e':
                language_host_triple = 'arm64e-apple-xros{}'.format(
                    self.args.darwin_deployment_version_xros)
                toolchain_target_arch = 'AARCH64'
                language_host_variant_sdk = 'XROS'
                cmake_osx_deployment_target = None

            darwin_sdk_deployment_targets = os.environ.get(
                'DARWIN_SDK_DEPLOYMENT_TARGETS')
            if darwin_sdk_deployment_targets:
                targets = darwin_sdk_deployment_targets.split(';')
                for target in targets:
                    (target_platform, target_arch, target_version) = target.split('-')
                    sdk_target = '{}_{}'.format(target_platform.upper(),
                                                target_arch.upper())
                    option_name = 'SWIFTLIB_DEPLOYMENT_VERSION_{}'.format(sdk_target)
                    language_cmake_options.define(option_name, target_version)

            cmake_os_sysroot = xcrun.sdk_path(sdk=platform)

            toolchain_cmake_options.define('CMAKE_OSX_DEPLOYMENT_TARGET:STRING',
                                      cmake_osx_deployment_target)
            toolchain_cmake_options.define('CMAKE_OSX_SYSROOT:PATH', cmake_os_sysroot)
            toolchain_cmake_options.define('COMPILER_RT_ENABLE_IOS:BOOL', 'FALSE')
            toolchain_cmake_options.define('COMPILER_RT_ENABLE_WATCHOS:BOOL', 'FALSE')
            toolchain_cmake_options.define('COMPILER_RT_ENABLE_TVOS:BOOL', 'FALSE')
            toolchain_cmake_options.define('SANITIZER_MIN_OSX_VERSION',
                                      cmake_osx_deployment_target)
            toolchain_cmake_options.define('LLVM_ENABLE_MODULES:BOOL',
                                      cmake.CMakeOptions.true_false(
                                          self.args.toolchain_enable_modules))
            toolchain_cmake_options.define('CMAKE_OSX_ARCHITECTURES', arch)

            # in build-script, if lto_type is defined, it will be set for toolchain and
            # language, so we don't have to have separate cases here
            if self.args.lto_type:
                toolchain_cmake_options.define('LLVM_PARALLEL_LINK_JOBS',
                                          self.args.toolchain_max_parallel_lto_link_jobs)
                language_cmake_options.define('SWIFT_PARALLEL_LINK_JOBS', self.args
                                           .language_tools_max_parallel_lto_link_jobs)
                toolchain_cmake_options.define('LLVM_ENABLE_MODULE_DEBUGGING:BOOL', 'NO')
                language_cmake_options.define('LLVM_ENABLE_MODULE_DEBUGGING:BOOL', 'NO')

            language_cmake_options.define('SWIFT_DARWIN_DEPLOYMENT_VERSION_OSX',
                                       self.args.darwin_deployment_version_osx)
            language_cmake_options.define('SWIFT_DARWIN_DEPLOYMENT_VERSION_IOS',
                                       self.args.darwin_deployment_version_ios)
            language_cmake_options.define('SWIFT_DARWIN_DEPLOYMENT_VERSION_TVOS',
                                       self.args.darwin_deployment_version_tvos)
            language_cmake_options.define('SWIFT_DARWIN_DEPLOYMENT_VERSION_WATCHOS',
                                       self.args.darwin_deployment_version_watchos)
            language_cmake_options.define('CMAKE_OSX_SYSROOT:PATH', cmake_os_sysroot)
            # This is needed to make sure to avoid using the wrong architecture
            # in the compiler checks CMake performs
            language_cmake_options.define('CMAKE_OSX_ARCHITECTURES', arch)

        # If we are asked to not generate test targets for LLVM and or Swift,
        # disable as many LLVM tools as we can. This improves compile time when
        # compiling with LTO.
        #
        # *NOTE* Currently we do not support testing LLVM via build-script. But in a
        # future commit we will.
        if toolchain_target_arch:
            toolchain_cmake_options.define('LLVM_TARGET_ARCH', toolchain_target_arch)

        # For cross-compilable hosts, we need to know the triple
        # and it must be the same for both LLVM and Swift
        if language_host_triple:
            toolchain_cmake_options.define('LLVM_HOST_TRIPLE:STRING', language_host_triple)
            language_cmake_options.define('SWIFT_HOST_TRIPLE:STRING', language_host_triple)

        language_cmake_options.define('SWIFT_HOST_VARIANT', language_host_variant)
        language_cmake_options.define('SWIFT_HOST_VARIANT_SDK', language_host_variant_sdk)
        language_cmake_options.define('SWIFT_HOST_VARIANT_ARCH', language_host_variant_arch)

        toolchain_cmake_options.define('LLVM_LIT_ARGS', '{} -j {}'.format(
            self.args.lit_args, self.args.lit_jobs))
        language_cmake_options.define('LLVM_LIT_ARGS', '{} -j {}'.format(
            self.args.lit_args, self.args.lit_jobs))

        if self.args.clang_profile_instr_use:
            toolchain_cmake_options.define('LLVM_PROFDATA_FILE',
                                      self.args.clang_profile_instr_use)

        if self.args.language_profile_instr_use:
            language_cmake_options.define('SWIFT_PROFDATA_FILE',
                                       self.args.language_profile_instr_use)

        toolchain_cmake_options.define('COVERAGE_DB', self.args.coverage_db)

        # This provides easier access to certain settings
        # users may need without having to use CMakeOptions interface
        relevant_options = {'CMAKE_OSX_SYSROOT': cmake_os_sysroot}

        return (toolchain_cmake_options, language_cmake_options, relevant_options)
