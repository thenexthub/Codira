cmake_minimum_required(VERSION 3.19.6)


#  set_property(GLOBAL PROPERTY GLOBAL_DEPENDS_DEBUG_MODE 1)

# TODO: Fix RPATH usage to be CMP0068 compliant
# Disable Policy CMP0068 for CMake 3.9
# rdar://37725888
if(POLICY CMP0068)
  cmake_policy(SET CMP0068 OLD)
endif()

# Honour CMAKE_CXX_STANDARD in try_compile(), needed for check_cxx_native_regex.
if(POLICY CMP0067)
  cmake_policy(SET CMP0067 NEW)
endif()

# Convert relative paths to absolute for subdirectory `target_sources`
if(POLICY CMP0076)
  cmake_policy(SET CMP0076 NEW)
endif()

# Don't clobber existing variable values when evaluating `option()` declarations.
if(POLICY CMP0077)
  cmake_policy(SET CMP0077 NEW)
endif()

# Add path for custom CMake modules.
list(APPEND CMAKE_MODULE_PATH
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")

set(CMAKE_DISABLE_IN_SOURCE_BUILD YES)

if(DEFINED CMAKE_JOB_POOLS)
  # CMake < 3.11 doesn't support CMAKE_JOB_POOLS. Manually set the property.
  set_property(GLOBAL PROPERTY JOB_POOLS "${CMAKE_JOB_POOLS}")
else()
  # Make a job pool for things that can't yet be distributed
  cmake_host_system_information(
    RESULT localhost_logical_cores QUERY NUMBER_OF_LOGICAL_CORES)
  set_property(GLOBAL APPEND PROPERTY JOB_POOLS local_jobs=${localhost_logical_cores})
  # Put linking in that category
  set(CMAKE_JOB_POOL_LINK local_jobs)
endif()

enable_language(C)
enable_language(CXX)

# On Windows, use MASM or MARMASM
set(LANGUAGE_ASM_DIALECT ASM)
set(LANGUAGE_ASM_EXT S)
set(LANGUAGE_ASM_AVAILABLE YES)
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  if(CMAKE_SYSTEM_PROCESSOR MATCHES "ARM64")
    if(CMAKE_VERSION VERSION_LESS "3.26")
      message(WARNING "We can't build assembly language for ARM64 until CMake 3.26")
      set(LANGUAGE_ASM_AVAILABLE NO)
    else()
      set(LANGUAGE_ASM_DIALECT ASM_MARMASM)
    endif()
  else()
    set(LANGUAGE_ASM_DIALECT ASM_MASM)
  endif()
  set(LANGUAGE_ASM_EXT asm)
endif()

if(LANGUAGE_ASM_AVAILABLE)
  enable_language(${LANGUAGE_ASM_DIALECT})
endif()

# Use C++17.
set(LANGUAGE_MIN_CXX_STANDARD 17)

# Unset CMAKE_CXX_STANDARD if it's too low and in the CMakeCache.txt
if($CACHE{CMAKE_CXX_STANDARD} AND $CACHE{CMAKE_CXX_STANDARD} LESS ${LANGUAGE_MIN_CXX_STANDARD})
  message(WARNING "Resetting cache value for CMAKE_CXX_STANDARD to ${LANGUAGE_MIN_CXX_STANDARD}")
  unset(CMAKE_CXX_STANDARD CACHE)
endif()

# Allow manually specified CMAKE_CXX_STANDARD if it's greater than the minimum
# required C++ version
if(DEFINED CMAKE_CXX_STANDARD AND CMAKE_CXX_STANDARD LESS ${LANGUAGE_MIN_CXX_STANDARD})
  message(FATAL_ERROR "Requested CMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD} which is less than the minimum C++ standard ${LANGUAGE_MIN_CXX_STANDARD}")
endif()

set(CMAKE_CXX_STANDARD ${LANGUAGE_MIN_CXX_STANDARD} CACHE STRING "C++ standard to conform to")
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS NO)

# First include general CMake utilities.
include(CodiraUtils)
include(CheckSymbolExists)
include(CMakeDependentOption)
include(CheckLanguage)
include(GNUInstallDirs)
include(CodiraCompilerCapability)
include(FetchContent)

# Enable Codira for the host compiler build if we have the language. It is
# optional until we have a bootstrap story.
check_language(Codira)
if(CMAKE_Codira_COMPILER)
  # we are not interested in logging any Codira module used
  # when configuring the build system -- those are not useful
  # since they will not contribute to the build of the compiler itself
  unset(ENV{LANGUAGE_LOADED_MODULE_TRACE_FILE})

  enable_language(Codira)
  set(DEFAULT_LANGUAGE_MIN_RUNTIME_VERSION "${CMAKE_Codira_COMPILER_VERSION}")
else()
  message(WARNING "Codira compiler not found on path.
  Cannot build compiler sources written in Codira.
  If this is unexpected, please pass the path to the languagec binary by defining the `CMAKE_Codira_COMPILER` variable.")
  set(DEFAULT_LANGUAGE_MIN_RUNTIME_VERSION)
endif()

# A convenience pattern to match Darwin platforms. Example:
#  if(LANGUAGE_HOST_VARIANT MATCHES "${LANGUAGE_DARWIN_VARIANTS}")
#     ...
#  endif()
set(LANGUAGE_DARWIN_VARIANTS "^(macosx|iphoneos|iphonesimulator|appletvos|appletvsimulator|watchos|watchsimulator)")
set(LANGUAGE_DARWIN_EMBEDDED_VARIANTS "^(iphoneos|iphonesimulator|appletvos|appletvsimulator|watchos|watchsimulator)")

# A convenient list to match Darwin SDKs. Example:
#  if("${LANGUAGE_HOST_VARIANT_SDK}" IN_LIST LANGUAGE_DARWIN_PLATFORMS)
#    ...
#  endif()
set(LANGUAGE_DARWIN_PLATFORMS "IOS" "IOS_SIMULATOR" "TVOS" "TVOS_SIMULATOR" "WATCHOS" "WATCHOS_SIMULATOR" "OSX" "XROS" "XROS_SIMULATOR")

set(LANGUAGE_APPLE_PLATFORMS ${LANGUAGE_DARWIN_PLATFORMS})
if(LANGUAGE_FREESTANDING_FLAVOR STREQUAL "apple")
  list(APPEND LANGUAGE_APPLE_PLATFORMS "FREESTANDING")
  if(LANGUAGE_FREESTANDING_IS_DARWIN)
    list(APPEND LANGUAGE_DARWIN_PLATFORMS "FREESTANDING")
  endif()
endif()

# If LANGUAGE_HOST_VARIANT_SDK not given, try to detect from the CMAKE_SYSTEM_NAME.
if(LANGUAGE_HOST_VARIANT_SDK)
  set(LANGUAGE_HOST_VARIANT_SDK_default "${LANGUAGE_HOST_VARIANT_SDK}")
else()
  if("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
    set(LANGUAGE_HOST_VARIANT_SDK_default "LINUX")
  elseif("${CMAKE_SYSTEM_NAME}" STREQUAL "FreeBSD")
    set(LANGUAGE_HOST_VARIANT_SDK_default "FREEBSD")
  elseif("${CMAKE_SYSTEM_NAME}" STREQUAL "OpenBSD")
    set(LANGUAGE_HOST_VARIANT_SDK_default "OPENBSD")
  elseif("${CMAKE_SYSTEM_NAME}" STREQUAL "CYGWIN")
    set(LANGUAGE_HOST_VARIANT_SDK_default "CYGWIN")
  elseif("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
    set(LANGUAGE_HOST_VARIANT_SDK_default "WINDOWS")
  elseif("${CMAKE_SYSTEM_NAME}" STREQUAL "Haiku")
    set(LANGUAGE_HOST_VARIANT_SDK_default "HAIKU")
  elseif("${CMAKE_SYSTEM_NAME}" STREQUAL "Android")
    set(LANGUAGE_HOST_VARIANT_SDK_default "ANDROID")
  elseif("${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
    set(LANGUAGE_HOST_VARIANT_SDK_default "OSX")
  elseif("${CMAKE_SYSTEM_NAME}" STREQUAL "WASI")
    set(LANGUAGE_HOST_VARIANT_SDK_default "WASI")
  elseif("${CMAKE_SYSTEM_NAME}" STREQUAL "Emscripten")
    set(LANGUAGE_HOST_VARIANT_SDK_default "EMSCRIPTEN")
  else()
    message(FATAL_ERROR "Unable to detect SDK for host system: ${CMAKE_SYSTEM_NAME}")
  endif()
endif()

# If LANGUAGE_HOST_VARIANT_ARCH not given, try to detect from the CMAKE_SYSTEM_PROCESSOR.
if(LANGUAGE_HOST_VARIANT_ARCH)
  set(LANGUAGE_HOST_VARIANT_ARCH_default "${LANGUAGE_HOST_VARIANT_ARCH}")
else()
  if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64|amd64")
    set(LANGUAGE_HOST_VARIANT_ARCH_default "x86_64")
  elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|ARM64|arm64")
    if(LANGUAGE_HOST_VARIANT_SDK_default STREQUAL "OSX")
      set(LANGUAGE_HOST_VARIANT_ARCH_default "arm64")
    else()
      set(LANGUAGE_HOST_VARIANT_ARCH_default "aarch64")
    endif()
  elseif("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "ppc64")
    set(LANGUAGE_HOST_VARIANT_ARCH_default "powerpc64")
  elseif("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "ppc")
    set(LANGUAGE_HOST_VARIANT_ARCH_default "powerpc")
  elseif("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "ppc64le")
    set(LANGUAGE_HOST_VARIANT_ARCH_default "powerpc64le")
  elseif("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "s390x")
    set(LANGUAGE_HOST_VARIANT_ARCH_default "s390x")
  elseif("${CMAKE_SYSTEM_PROCESSOR}" MATCHES "armv5|armv5te")
    set(LANGUAGE_HOST_VARIANT_ARCH_default "armv5")
  # FIXME: Only matches v6l/v7l - by far the most common variants
  elseif("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "armv6l")
    set(LANGUAGE_HOST_VARIANT_ARCH_default "armv6")
  elseif("${CMAKE_SYSTEM_PROCESSOR}" MATCHES "armv7l|armv7-a")
    set(LANGUAGE_HOST_VARIANT_ARCH_default "armv7")
  elseif("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "IA64")
    set(LANGUAGE_HOST_VARIANT_ARCH_default "itanium")
  elseif("${CMAKE_SYSTEM_PROCESSOR}" MATCHES "(x86|i686)")
    set(LANGUAGE_HOST_VARIANT_ARCH_default "i686")
  elseif("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "wasm32")
    set(LANGUAGE_HOST_VARIANT_ARCH_default "wasm32")
  else()
    message(FATAL_ERROR "Unrecognized architecture on host system: ${CMAKE_SYSTEM_PROCESSOR}")
  endif()
endif()

set(LANGUAGE_HOST_VARIANT_SDK "${LANGUAGE_HOST_VARIANT_SDK_default}" CACHE STRING
    "Deployment sdk for Codira host tools (the compiler).")
set(LANGUAGE_HOST_VARIANT_ARCH "${LANGUAGE_HOST_VARIANT_ARCH_default}" CACHE STRING
    "Deployment arch for Codira host tools (the compiler).")

#
# User-configurable options that control the inclusion and default build
# behavior for components which may not strictly be necessary (tools, examples,
# and tests).
#
# This is primarily to support building smaller or faster project files.
#

option(LANGUAGE_APPEND_VC_REV
  "Embed the version control system revision in Codira"
  TRUE)

option(LANGUAGE_INCLUDE_TOOLS
    "Generate build targets for language tools"
    TRUE)

option(LANGUAGE_BUILD_REMOTE_MIRROR
    "Build the Codira Remote Mirror Library"
    TRUE)

option(LANGUAGE_BUILD_DYNAMIC_STDLIB
    "Build dynamic variants of the Codira standard library"
    TRUE)

option(LANGUAGE_BUILD_STATIC_STDLIB
    "Build static variants of the Codira standard library"
    FALSE)

option(LANGUAGE_STDLIB_STATIC_PRINT
    "Build compile-time evaluated vprintf()"
    FALSE)

option(LANGUAGE_STDLIB_ENABLE_UNICODE_DATA
    "Include Unicode data files in the standard library.
    NOTE: Disabling this will cause many String methods to crash."
    TRUE)

option(LANGUAGE_BUILD_CLANG_OVERLAYS
  "Build Codira overlays for the clang builtin modules"
  TRUE)

# The SDK overlay is provided by the SDK itself on Darwin platforms.
if(LANGUAGE_HOST_VARIANT_SDK IN_LIST LANGUAGE_DARWIN_PLATFORMS)
  set(LANGUAGE_BUILD_DYNAMIC_SDK_OVERLAY_default FALSE)
else()
  set(LANGUAGE_BUILD_DYNAMIC_SDK_OVERLAY_default TRUE)
endif()

option(LANGUAGE_BUILD_DYNAMIC_SDK_OVERLAY
    "Build dynamic variants of the Codira SDK overlay"
    "${LANGUAGE_BUILD_DYNAMIC_SDK_OVERLAY_default}")

option(LANGUAGE_BUILD_STATIC_SDK_OVERLAY
    "Build static variants of the Codira SDK overlay"
    FALSE)

option(LANGUAGE_BUILD_STDLIB_EXTRA_TOOLCHAIN_CONTENT
    "If not building stdlib, controls whether to build 'stdlib/toolchain' content"
    TRUE)

option(LANGUAGE_BUILD_STDLIB_CXX_MODULE
  "If not building stdlib, controls whether to build the Cxx module"
  TRUE)

# In many cases, the CMake build system needs to determine whether to include
# a directory, or perform other actions, based on whether the stdlib or SDK is
# being built at all -- statically or dynamically. Please note that these
# flags are not related to the deprecated build-script-impl arguments
# 'build-language-stdlib' and 'build-language-sdk-overlay'. These are not flags that
# the build script should be able to set.
if(LANGUAGE_BUILD_DYNAMIC_STDLIB OR LANGUAGE_BUILD_STATIC_STDLIB)
  set(LANGUAGE_BUILD_STDLIB TRUE)
else()
  set(LANGUAGE_BUILD_STDLIB FALSE)
endif()

if(LANGUAGE_BUILD_DYNAMIC_SDK_OVERLAY OR LANGUAGE_BUILD_STATIC_SDK_OVERLAY)
  set(LANGUAGE_BUILD_SDK_OVERLAY TRUE)
else()
  set(LANGUAGE_BUILD_SDK_OVERLAY FALSE)
endif()

option(LANGUAGE_BUILD_PERF_TESTSUITE
    "Create in-tree targets for building language performance benchmarks."
    FALSE)

option(LANGUAGE_INCLUDE_TESTS "Create targets for building/running tests." TRUE)

option(LANGUAGE_INCLUDE_TEST_BINARIES
  "Create targets for building/running test binaries even if LANGUAGE_INCLUDE_TESTS is disabled"
  TRUE)

option(LANGUAGE_INCLUDE_DOCS
    "Create targets for building docs."
    TRUE)

set(_language_include_apinotes_default FALSE)
if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  set(_language_include_apinotes_default TRUE)
endif()

option(LANGUAGE_INCLUDE_APINOTES
  "Create targets for installing the remaining apinotes in the built toolchain."
  ${_language_include_apinotes_default})

#
# Miscellaneous User-configurable options.
#
# TODO: Please categorize these!
#

if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE "Debug" CACHE STRING
      "Build type for Codira [Debug, RelWithDebInfo, Release, MinSizeRel]"
      FORCE)
  message(STATUS "No build type was specified, will default to ${CMAKE_BUILD_TYPE}")
endif()

set(LANGUAGE_ANALYZE_CODE_COVERAGE FALSE CACHE STRING
    "Build Codira with code coverage instrumenting enabled [FALSE, NOT-MERGED, MERGED]")

include(${CMAKE_CURRENT_LIST_DIR}/cmake/CodiraVersion.cmake)

set(LANGUAGE_VENDOR "" CACHE STRING
    "The vendor name of the Codira compiler")
set(LANGUAGE_COMPILER_VERSION "" CACHE STRING
    "The internal version of the Codira compiler")
set(CLANG_COMPILER_VERSION "" CACHE STRING
    "The internal version of the Clang compiler")
set(LANGUAGE_TOOLCHAIN_VERSION "" CACHE STRING
    "The Codira compiler tag")

option(LANGUAGE_DISABLE_DEAD_STRIPPING
      "Turn off Darwin-specific dead stripping for Codira host tools." FALSE)

set(LANGUAGE_TOOLS_ENABLE_LTO OFF CACHE STRING "Build Codira tools with LTO. One
    must specify the form of LTO by setting this to one of: 'full', 'thin'. This
    option only affects the tools that run on the host (the compiler), and has
    no effect on the target libraries (the standard library and the runtime).")

option(LANGUAGE_TOOLS_LD64_LTO_CODEGEN_ONLY_FOR_SUPPORTING_TARGETS
    "When building ThinLTO using ld64 on Darwin, controls whether to opt out of
    LLVM IR optimizations when linking targets that will get
    little benefit from it (e.g. tools for bootstrapping or
    debugging Codira)"
    FALSE)

option(LANGUAGE_ENABLE_LANGUAGE_IN_LANGUAGE "Enable Codira sources in Codira compiler" ON)

if(LANGUAGE_ENABLE_LANGUAGE_IN_LANGUAGE)
  set(BOOTSTRAPPING_MODE HOSTTOOLS CACHE STRING [=[
  How to build the language compiler modules. Possible values are
      HOSTTOOLS:     build with a pre-installed toolchain
      BOOTSTRAPPING: build with a 2-stage bootstrapping process
      BOOTSTRAPPING-WITH-HOSTLIBS:   build with a 2-stage bootstrapping process,
                     but the compiler links against the host system language libs (macOS only)
      CROSSCOMPILE:  cross-compiledwith a native host compiler, provided in
                     `LANGUAGE_NATIVE_LANGUAGE_TOOLS_PATH` (non-Darwin only)
      CROSSCOMPILE-WITH-HOSTLIBS:    build with a bootstrapping-with-hostlibs compiled
                                     compiler, provided in `LANGUAGE_NATIVE_LANGUAGE_TOOLS_PATH`
  ]=])
else()
  set(BOOTSTRAPPING_MODE OFF)
  set(LANGUAGE_BUILD_LANGUAGE_SYNTAX OFF)
endif()

option(BRIDGING_MODE [=[
How language-C++ bridging code is compiled:
    INLINE:       uses full language C++ interop and briding functions are inlined
    PURE:         uses limited C++ interp an bridging functions are not inlined
    DEFAULT:      based on the build configuration
]=] DEFAULT)

option(LANGUAGE_USE_SYMLINKS "Use symlinks instead of copying binaries" ${CMAKE_HOST_UNIX})
set(LANGUAGE_COPY_OR_SYMLINK "copy_if_different")
set(LANGUAGE_COPY_OR_SYMLINK_DIR "copy_directory")
if(LANGUAGE_USE_SYMLINKS)
  set(LANGUAGE_COPY_OR_SYMLINK "create_symlink")
  set(LANGUAGE_COPY_OR_SYMLINK_DIR "create_symlink")
endif()

# The following only works with the Ninja generator in CMake >= 3.0.
set(LANGUAGE_PARALLEL_LINK_JOBS "" CACHE STRING
  "Define the maximum number of linker jobs for language.")

option(LANGUAGE_FORCE_OPTIMIZED_TYPECHECKER "Override the optimization setting of
  the type checker so that it always compiles with optimization. This eases
  debugging after type checking occurs by speeding up type checking" FALSE)

# Allow building Codira with Clang's Profile Guided Optimization
if(LANGUAGE_PROFDATA_FILE AND EXISTS ${LANGUAGE_PROFDATA_FILE})
  if(NOT CMAKE_C_COMPILER_ID MATCHES Clang)
    message(FATAL_ERROR "LANGUAGE_PROFDATA_FILE can only be specified when compiling with clang")
  endif()
  add_definitions("-fprofile-instr-use=${LANGUAGE_PROFDATA_FILE}")
endif()

set(LANGUAGE_TOOLS_INSTALL_DIR "${CMAKE_INSTALL_BINDIR}" CACHE PATH
  "Path for binary subdirectory to use during installation.
  Used by add_language_tool_symlink in AddCodira.cmake so that toolchain_install_symlink generates the installation script properly.")

#
# User-configurable Codira Standard Library specific options.
#
# TODO: Once the stdlib/compiler builds are split, this should be sunk into the
# stdlib cmake.
#

set(LANGUAGE_STDLIB_BUILD_TYPE "${CMAKE_BUILD_TYPE}" CACHE STRING
    "Build type for the Codira standard library and SDK overlays [Debug, RelWithDebInfo, Release, MinSizeRel]")
# Allow the user to specify the standard library CMAKE_MSVC_RUNTIME_LIBRARY
# value.  The following values are valid:
#   - MultiThreaded (/MT)
#   - MultiThreadedDebug (/MTd)
#   - MultiThreadedDLL (/MD)
#   - MultiThreadedDebugDLL (/MDd)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(LANGUAGE_STDLIB_MSVC_RUNTIME_LIBRARY_default MultiThreadedDebugDLL)
else()
  set(LANGUAGE_STDLIB_MSVC_RUNTIME_LIBRARY_default MultiThreadedDLL)
endif()
set(LANGUAGE_STDLIB_MSVC_RUNTIME_LIBRARY
  ${LANGUAGE_STDLIB_MSVC_RUNTIME_LIBRARY_default}
  CACHE STRING "MSVC Runtime Library for the standard library")

if("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows" AND BOOTSTRAPPING_MODE STREQUAL "HOSTTOOLS" AND
    CMAKE_BUILD_TYPE STREQUAL "Debug")
  # Building with the host Codira toolchain requires linking just-built binaries
  # against the host Codira runtime. In debug builds, that means linking a debug
  # binary against a release binary. The MSVC linker does not normally permit
  # this, since debug builds enable bounds-checked C++ iterators by default,
  # which are not ABI-compatible with regular iterators. Let's instruct MSVC to
  # disable bounds-checked iterators to make it possible to do a debug build of
  # the Codira compiler with a host toolchain.
  add_definitions(-D_ITERATOR_DEBUG_LEVEL=0)
endif()

if(BRIDGING_MODE STREQUAL "DEFAULT" OR NOT BRIDGING_MODE)
  if(CMAKE_BUILD_TYPE STREQUAL "Debug" OR "${LANGUAGE_HOST_VARIANT_SDK}" STREQUAL "WINDOWS" OR (CMAKE_Codira_COMPILER AND CMAKE_Codira_COMPILER_VERSION VERSION_LESS 5.8))
    # In debug builds, to workaround a problem with LLDB's `po` command (rdar://115770255).
    # If the host Codira version is less than 5.8, use pure mode to workaround a C++ interop compiler crash.
    set(BRIDGING_MODE "PURE")
  else()
    set(BRIDGING_MODE "INLINE")
  endif()
endif()

is_build_type_optimized("${LANGUAGE_STDLIB_BUILD_TYPE}" language_optimized)
if(language_optimized)
  set(LANGUAGE_STDLIB_ASSERTIONS_default FALSE)
else()
  set(LANGUAGE_STDLIB_ASSERTIONS_default TRUE)
endif()
option(LANGUAGE_STDLIB_ASSERTIONS
    "Enable internal checks for the Codira standard library (useful for debugging the library itself, does not affect checks required for safety)"
    "${LANGUAGE_STDLIB_ASSERTIONS_default}")

option(LANGUAGE_STDLIB_ENABLE_STRICT_AVAILABILITY
       "Enable strict availability; this will cause things to break at desk or in CI if the host OS is a lower version than some `@availability` annotations in the runtime code"
       FALSE)

option(LANGUAGE_BUILD_RUNTIME_WITH_HOST_COMPILER
       "Use the host compiler and not the internal clang to build the language runtime"
       FALSE)

option(LANGUAGE_RUN_TESTS_WITH_HOST_COMPILER
       "Run tests against the host compiler and not the just built language"
       FALSE)

set(LANGUAGE_SDKS "" CACHE STRING
    "If non-empty, limits building target binaries only to specified SDKs (despite other SDKs being available)")

set(LANGUAGE_PRIMARY_VARIANT_SDK "" CACHE STRING
    "Primary SDK for target binaries")
set(LANGUAGE_PRIMARY_VARIANT_ARCH "" CACHE STRING
    "Primary arch for target binaries")

set(LANGUAGE_NATIVE_TOOLCHAIN_TOOLS_PATH "" CACHE STRING
    "Path to the directory that contains LLVM tools that are executable on the build machine")

set(LANGUAGE_NATIVE_CLANG_TOOLS_PATH "" CACHE STRING
    "Path to the directory that contains Clang tools that are executable on the build machine")

set(LANGUAGE_NATIVE_LANGUAGE_TOOLS_PATH "" CACHE STRING
   "Path to the directory that contains Codira tools that are executable on the build machine")

set(LANGUAGE_WASI_SYSROOT_PATH "" CACHE PATH
  "Path to the directory that contains WASI Sysroot")

option(LANGUAGE_STDLIB_ENABLE_SIB_TARGETS
       "Should we generate sib targets for the stdlib or not?"
       FALSE)

option(LANGUAGE_STDLIB_BUILD_SYMBOL_GRAPHS
       "Whether to build symbol graphs for the stdlib, for use in documentation."
       FALSE)


set(LANGUAGE_DARWIN_SUPPORTED_ARCHS "" CACHE STRING
  "Semicolon-separated list of architectures to configure on Darwin platforms. \
If left empty all default architectures are configured.")

set(LANGUAGE_DARWIN_MODULE_ARCHS "" CACHE STRING
  "Semicolon-separated list of architectures to configure Codira module-only \
targets on Darwin platforms. These targets are in addition to the full \
library targets.")

set(LANGUAGE_MIN_RUNTIME_VERSION "${DEFAULT_LANGUAGE_MIN_RUNTIME_VERSION}" CACHE STRING
  "Specify the minimum version of the runtime that we target when building \
the compiler itself. This is used on non-Darwin platforms to ensure \
that it's possible to build the compiler using host tools.")

#
# User-configurable Linux specific options.
#

set(LANGUAGE_MUSL_PATH "/usr/local/musl" CACHE STRING
  "Path to the directory that contains the Musl headers and libraries. \
This is only required if we have been asked to build the Musl SDK, and \
defaults to the default install location for Musl.")

set(LANGUAGE_SDK_LINUX_STATIC_ARCHITECTURES "" CACHE STRING
  "The architectures to configure when using the static Linux SDK.")

set(LANGUAGE_SDK_LINUX_ARCHITECTURES "" CACHE STRING
  "The architectures to configure when using the Linux SDK.")

#
# User-configurable Android specific options.
#

set(LANGUAGE_ANDROID_API_LEVEL "" CACHE STRING
  "Version number for the Android API")

set(LANGUAGE_ANDROID_NDK_PATH "" CACHE STRING
  "Path to the directory that contains the Android NDK tools that are executable on the build machine")
set(LANGUAGE_ANDROID_DEPLOY_DEVICE_PATH "" CACHE STRING
  "Path on an Android device where build products will be pushed. These are used when running the test suite against the device")

#
# User-configurable WebAssembly specific options.
#

option(LANGUAGE_ENABLE_WASI_THREADS
  "Build the Standard Library with WASI threads support"
  FALSE)

#
# User-configurable Darwin-specific options.
#
option(LANGUAGE_EMBED_BITCODE_SECTION
    "If non-empty, embeds LLVM bitcode binary sections in the standard library and overlay binaries for supported platforms"
    FALSE)

option(LANGUAGE_EMBED_BITCODE_SECTION_HIDE_SYMBOLS
  "If non-empty, when embedding the LLVM bitcode binary sections into the relevant binaries, pass in -bitcode_hide_symbols. Does nothing if LANGUAGE_EMBED_BITCODE_SECTION is set to false."
  FALSE)

if("${LANGUAGE_HOST_VARIANT_SDK}" MATCHES "(OSX|IOS*|TVOS*|WATCHOS*)")
  set(LANGUAGE_RUNTIME_CRASH_REPORTER_CLIENT_default TRUE)
else()
  set(LANGUAGE_RUNTIME_CRASH_REPORTER_CLIENT_default FALSE)
endif()

option(LANGUAGE_RUNTIME_CRASH_REPORTER_CLIENT
    "Whether to enable CrashReporter integration"
    "${LANGUAGE_RUNTIME_CRASH_REPORTER_CLIENT_default}")

set(LANGUAGE_DARWIN_XCRUN_TOOLCHAIN "XcodeDefault" CACHE STRING
    "The name of the toolchain to pass to 'xcrun'")

set(LANGUAGE_DARWIN_STDLIB_INSTALL_NAME_DIR "/usr/lib/language" CACHE STRING
    "The directory of the install_name for standard library dylibs")

#
# User-configurable OpenBSD-specific options.
#

option(LANGUAGE_OPENBSD_BTCFI
  "Emit branch target identification instructions and sign return addresses when available"
  FALSE)
if(LANGUAGE_OPENBSD_BTCFI)
  add_definitions("-DLANGUAGE_OPENBSD_BTCFI")
endif()

# We don't want to use the same install_name_dir as the standard library which
# will be installed in /usr/lib/language. These private libraries should continue
# to use @rpath for now.
set(LANGUAGE_DARWIN_STDLIB_PRIVATE_INSTALL_NAME_DIR "@rpath" CACHE STRING
    "The directory of the install_name for the private standard library dylibs")

option(LANGUAGE_ALLOW_LINKING_LANGUAGE_CONTENT_IN_DARWIN_TOOLCHAIN
  "Adds search paths for libraries in the toolchain
   when building Codira programs.
   This is needed to support Apple internal configurations."
  FALSE)

set(LANGUAGE_DARWIN_DEPLOYMENT_VERSION_OSX "13.0" CACHE STRING
    "Minimum deployment target version for OS X")

set(LANGUAGE_DARWIN_DEPLOYMENT_VERSION_IOS "16.0" CACHE STRING
    "Minimum deployment target version for iOS")

set(LANGUAGE_DARWIN_DEPLOYMENT_VERSION_TVOS "16.0" CACHE STRING
    "Minimum deployment target version for tvOS")

set(LANGUAGE_DARWIN_DEPLOYMENT_VERSION_WATCHOS "6.0" CACHE STRING
    "Minimum deployment target version for watchOS")

set(LANGUAGE_DARWIN_DEPLOYMENT_VERSION_XROS "1.0" CACHE STRING
    "Minimum deployment target version for xrOS")

#
#  Compatibility library deployment versions
#

set(COMPATIBILITY_MINIMUM_DEPLOYMENT_VERSION_OSX "10.9")
set(COMPATIBILITY_MINIMUM_DEPLOYMENT_VERSION_IOS "7.0")
set(COMPATIBILITY_MINIMUM_DEPLOYMENT_VERSION_TVOS "9.0")
set(COMPATIBILITY_MINIMUM_DEPLOYMENT_VERSION_WATCHOS "2.0")
set(COMPATIBILITY_MINIMUM_DEPLOYMENT_VERSION_XROS "1.0")
set(COMPATIBILITY_MINIMUM_DEPLOYMENT_VERSION_MACCATALYST "13.1")

#
# User-configurable debugging options.
#

option(LANGUAGE_SIL_VERIFY_ALL
    "Run SIL verification after each transform when building Codira files in the build process"
    FALSE)

option(LANGUAGE_SIL_VERIFY_ALL_MACOS_ONLY
    "Run SIL verification after each transform when building the macOS stdlib"
    FALSE)

option(LANGUAGE_EMIT_SORTED_SIL_OUTPUT
    "Sort SIL output by name to enable diffing of output"
    FALSE)

if(LANGUAGE_STDLIB_ASSERTIONS)
  set(LANGUAGE_RUNTIME_CLOBBER_FREED_OBJECTS_default TRUE)
else()
  set(LANGUAGE_RUNTIME_CLOBBER_FREED_OBJECTS_default FALSE)
endif()

option(LANGUAGE_RUNTIME_CLOBBER_FREED_OBJECTS
    "Overwrite memory for deallocated Codira objects"
    "${LANGUAGE_RUNTIME_CLOBBER_FREED_OBJECTS_default}")

option(LANGUAGE_STDLIB_SIL_DEBUGGING
    "Compile the Codira standard library with -sil-based-debuginfo to enable debugging and profiling on SIL level"
    FALSE)

option(LANGUAGE_CHECK_INCREMENTAL_COMPILATION
    "Check if incremental compilation works when compiling the Codira libraries"
    FALSE)

option(LANGUAGE_ENABLE_ARRAY_COW_CHECKS
    "Compile the stdlib with Array COW checks enabled (only relevant for assert builds)"
    FALSE)

option(LANGUAGE_REPORT_STATISTICS
    "Create json files which contain internal compilation statistics"
    FALSE)

# Only Darwin platforms enable ObjC interop by default.
if("${LANGUAGE_HOST_VARIANT_SDK}" MATCHES "(OSX|IOS*|TVOS*|WATCHOS*|XROS*)")
  set(LANGUAGE_STDLIB_ENABLE_OBJC_INTEROP_default TRUE)
else()
  set(LANGUAGE_STDLIB_ENABLE_OBJC_INTEROP_default FALSE)
endif()

# Used by stdlib/toolchain as well, so this cannot be in stdlib/CMakeLists.txt
option(LANGUAGE_STDLIB_ENABLE_OBJC_INTEROP
       "Should stdlib be built with Obj-C interop."
       "${LANGUAGE_STDLIB_ENABLE_OBJC_INTEROP_default}")

set(LANGUAGE_DEBUGINFO_NON_LTO_ARGS "-g" CACHE STRING
    "Compiler options to use when building the compiler in debug or debuginfo mode. These do not apply when linking with LTO")

#
# User-configurable experimental options.  Do not use in production builds.
#

set(LANGUAGE_EXPERIMENTAL_EXTRA_FLAGS "" CACHE STRING
    "Extra flags to pass when compiling language files.  Use this option *only* for one-off experiments")

set(LANGUAGE_EXPERIMENTAL_EXTRA_REGEXP_FLAGS "" CACHE STRING
  "A list of [module_regexp1;flags1;module_regexp2;flags2,...] which can be used to apply specific flags to modules that match a cmake regexp. It always applies the first regexp that matches.")

set(LANGUAGE_EXPERIMENTAL_EXTRA_NEGATIVE_REGEXP_FLAGS "" CACHE STRING
    "A list of [module_regexp1;flags1;module_regexp2;flags2,...] which can be used to apply specific flags to modules that do not match a cmake regexp. It always applies the first regexp that does not match. The reason this is necessary is that cmake does not provide negative matches in the regex. Instead you have to use NOT in the if statement requiring a separate variable.")

option(LANGUAGE_RUNTIME_ENABLE_LEAK_CHECKER
  "Should the runtime be built with support for non-thread-safe leak detecting entrypoints"
  FALSE)

option(LANGUAGE_ENABLE_RUNTIME_FUNCTION_COUNTERS
  "Enable runtime function counters and expose the API."
  FALSE)

option(LANGUAGE_ENABLE_STDLIBCORE_EXCLUSIVITY_CHECKING
  "Build stdlibCore with exclusivity checking enabled"
  FALSE)

option(LANGUAGE_STDLIB_ENABLE_DEBUG_PRECONDITIONS_IN_RELEASE
  "Enable _debugPrecondition checks in the stdlib in Release configurations"
  FALSE)

option(LANGUAGE_ENABLE_EXPERIMENTAL_DIFFERENTIABLE_PROGRAMMING
  "Enable experimental Codira differentiable programming features"
  FALSE)

option(LANGUAGE_IMPLICIT_CONCURRENCY_IMPORT
  "Implicitly import the Codira concurrency module"
  TRUE)

option(LANGUAGE_ENABLE_EXPERIMENTAL_CONCURRENCY
  "Enable build of the Codira concurrency module"
  FALSE)

option(LANGUAGE_ENABLE_EXPERIMENTAL_CXX_INTEROP
  "Enable experimental C++ interop modules"
  TRUE)

option(LANGUAGE_ENABLE_CXX_INTEROP_LANGUAGE_BRIDGING_HEADER
  "Install the <language/bridging> C++ interoperability header alongside compiler"
  TRUE)

option(LANGUAGE_ENABLE_EXPERIMENTAL_DISTRIBUTED
  "Enable experimental distributed actors and functions"
  FALSE)

option(LANGUAGE_ENABLE_EXPERIMENTAL_STRING_PROCESSING
  "Enable experimental string processing"
  FALSE)

option(LANGUAGE_ENABLE_EXPERIMENTAL_OBSERVATION
  "Enable build of the Codira observation module"
  FALSE)

option(LANGUAGE_STDLIB_ENABLE_STRICT_CONCURRENCY_COMPLETE
  "Build the stdlib with -strict-concurrency=complete"
  FALSE)

option(LANGUAGE_ENABLE_SYNCHRONIZATION
  "Enable build of the Codira Synchronization module"
  FALSE)

option(LANGUAGE_ENABLE_RUNTIME_MODULE
  "Build the Codira Runtime module"
  FALSE)

option(LANGUAGE_ENABLE_VOLATILE
  "Enable build of the Codira Volatile module"
  FALSE)

option(LANGUAGE_ENABLE_DISPATCH
  "Enable use of libdispatch"
  TRUE)

option(LANGUAGE_ENABLE_GLOBAL_ISEL_ARM64
  "Enable global isel on arm64"
  FALSE)

option(LANGUAGE_ENABLE_EXPERIMENTAL_PARSER_VALIDATION
  "Enable experimental CodiraParser validation by default"
  FALSE)

cmake_dependent_option(LANGUAGE_BUILD_SOURCEKIT
  "Build SourceKit" TRUE
  "LANGUAGE_ENABLE_DISPATCH" FALSE)
cmake_dependent_option(LANGUAGE_ENABLE_SOURCEKIT_TESTS
  "Enable running SourceKit tests" TRUE
  "LANGUAGE_BUILD_SOURCEKIT" FALSE)

option(LANGUAGE_THREADING_PACKAGE
  "Override the threading package used for the build.  This can either be a
   single package name, or a semicolon separated sequence of sdk:package pairs.
   Valid package names are 'pthreads', 'darwin', 'linux', 'win32', 'c11', 'none'
   or the empty string for the SDK default.")

option(LANGUAGE_THREADING_HAS_DLSYM
  "Enable the use of the dlsym() function.  This gets used to provide TSan
   support on some platforms."
  TRUE)

option(LANGUAGE_ENABLE_MACCATALYST
  "Build the Standard Library and overlays with MacCatalyst support"
  FALSE)

option(LANGUAGE_ENABLE_BACKTRACING
  "Build backtracing runtime support"
  FALSE)

set(LANGUAGE_DARWIN_DEPLOYMENT_VERSION_MACCATALYST "14.5" CACHE STRING
  "Minimum deployment target version for macCatalyst")

#
# End of user-configurable options.
#

set(LANGUAGE_BUILT_STANDALONE FALSE)
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")
  set(LANGUAGE_BUILT_STANDALONE TRUE)
endif()

if(LANGUAGE_BUILT_STANDALONE)
  project(Codira C CXX ${LANGUAGE_ASM_DIALECT})
endif()

if(MSVC OR "${CMAKE_SIMULATE_ID}" STREQUAL "MSVC")
  include(ClangClCompileRules)
elseif(UNIX)
  include(UnixCompileRules)
endif()

if(CMAKE_C_COMPILER_ID MATCHES Clang)
  add_compile_options($<$<OR:$<COMPILE_LANGUAGE:C>,$<COMPILE_LANGUAGE:CXX>>:-Werror=gnu>)
endif()

# Make some warnings errors as they are commonly occurring and flood the build
# with unnecessary noise.
if(CMAKE_C_COMPILER_ID MATCHES Clang)
  add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Werror=c++98-compat-extra-semi>)
endif()

option(LANGUAGE_BUILD_LANGUAGE_SYNTAX
  "Enable building language syntax"
  FALSE)

option(LANGUAGE_BUILD_REGEX_PARSER_IN_COMPILER
    "Build the Codira regex parser as part of the compiler."
    TRUE)
if(LANGUAGE_BUILD_REGEX_PARSER_IN_COMPILER AND NOT LANGUAGE_BUILD_LANGUAGE_SYNTAX)
  message(WARNING "Force setting LANGUAGE_BUILD_REGEX_PARSER_IN_COMPILER=OFF because Codira parser integration is disabled")
  set(LANGUAGE_BUILD_REGEX_PARSER_IN_COMPILER OFF)
endif()

set(LANGUAGE_BUILD_HOST_DISPATCH FALSE)
if(LANGUAGE_ENABLE_DISPATCH AND NOT CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  # Only build libdispatch for the host if the host tools are being built and
  # specifically if these two libraries that depend on it are built.
  if(LANGUAGE_INCLUDE_TOOLS AND LANGUAGE_BUILD_SOURCEKIT)
    set(LANGUAGE_BUILD_HOST_DISPATCH TRUE)
  endif()

  if(LANGUAGE_BUILD_HOST_DISPATCH)
    if(NOT EXISTS "${LANGUAGE_PATH_TO_LIBDISPATCH_SOURCE}")
      message(SEND_ERROR "SourceKit requires libdispatch on non-Darwin hosts.  Please specify LANGUAGE_PATH_TO_LIBDISPATCH_SOURCE")
    endif()
  endif()
endif()

file(STRINGS "utils/availability-macros.def" LANGUAGE_STDLIB_AVAILABILITY_DEFINITIONS)
list(FILTER LANGUAGE_STDLIB_AVAILABILITY_DEFINITIONS EXCLUDE REGEX "^\\s*(#.*)?$")

#
# Include CMake modules
#

include(CheckCXXSourceRuns)
include(CMakeParseArguments)
include(CMakePushCheckState)

# Print out path and version of any installed commands
message(STATUS "CMake (${CMAKE_COMMAND}) Version: ${CMAKE_VERSION}")
execute_process(COMMAND ${CMAKE_MAKE_PROGRAM} --version
  OUTPUT_VARIABLE _CMAKE_MAKE_PROGRAM_VERSION
  OUTPUT_STRIP_TRAILING_WHITESPACE)
message(STATUS "CMake Make Program (${CMAKE_MAKE_PROGRAM}) Version: ${_CMAKE_MAKE_PROGRAM_VERSION}")
message(STATUS "C Compiler (${CMAKE_C_COMPILER}) Version: ${CMAKE_C_COMPILER_VERSION}")
message(STATUS "C++ Compiler (${CMAKE_CXX_COMPILER}) Version: ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "Assembler (${CMAKE_${LANGUAGE_ASM_DIALECT}_COMPILER}) Version: ${CMAKE_${LANGUAGE_ASM_DIALECT}_COMPILER_VERSION}")
if (CMAKE_Codira_COMPILER)
  message(STATUS "Codira Compiler (${CMAKE_Codira_COMPILER}) Version: ${CMAKE_Codira_COMPILER_VERSION}")

  # Check if the current Codira compiler has implicit _StringProcessing module.
  language_supports_implicit_module("string-processing"
    LANGUAGE_SUPPORTS_DISABLE_IMPLICIT_STRING_PROCESSING_MODULE_IMPORT)
  message(STATUS "  Implicit 'string-processing' import: ${LANGUAGE_SUPPORTS_DISABLE_IMPLICIT_STRING_PROCESSING_MODULE_IMPORT}")

  language_get_package_cmo_support(
    Codira_COMPILER_PACKAGE_CMO_SUPPORT)
  message(STATUS "  Package CMO: ${Codira_COMPILER_PACKAGE_CMO_SUPPORT}")
else()
  message(STATUS "Codira Compiler (None).")
endif()

set(THREADS_PREFER_PTHREAD_FLAG YES)
include(FindThreads)

if(LANGUAGE_PATH_TO_CMARK_BUILD)
  execute_process(COMMAND ${LANGUAGE_PATH_TO_CMARK_BUILD}/src/cmark --version
    OUTPUT_VARIABLE _CMARK_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE)
  message(STATUS "CMark Version: ${_CMARK_VERSION}")
elseif(LANGUAGE_INCLUDE_TOOLS)
  find_package(cmark-gfm CONFIG REQUIRED)
endif()
message(STATUS "")

# Check if a prebuilt clang path was passed in, as this variable will be
# assigned if not, in CodiraSharedCMakeConfig.
if("${LANGUAGE_NATIVE_CLANG_TOOLS_PATH}" STREQUAL "")
  set(LANGUAGE_PREBUILT_CLANG FALSE)
else()
  set(LANGUAGE_PREBUILT_CLANG TRUE)
endif()

# Also mark if we have a prebuilt language before we do anything.
if("${LANGUAGE_NATIVE_LANGUAGE_TOOLS_PATH}" STREQUAL "")
  set(LANGUAGE_PREBUILT_LANGUAGE FALSE)
else()
  set(LANGUAGE_PREBUILT_LANGUAGE TRUE)
endif()

include(CodiraSharedCMakeConfig)

# NOTE: We include this before CodiraComponents as it relies on some LLVM CMake
# functionality.
# Support building Codira as a standalone project, using LLVM as an
# external library.
if(LANGUAGE_BUILT_STANDALONE)
  language_common_standalone_build_config(LANGUAGE)
else()
  language_common_unified_build_config(LANGUAGE)
endif()

include(CodiraComponents)
include(CodiraHandleGybSources)
include(CodiraSetIfArchBitness)
include(AddCodira)
include(CodiraConfigureSDK)
include(CodiraComponents)
include(CodiraList)
include(AddPureCodira)

# Configure language include, install, build components.
language_configure_components()

# lipo is used to create universal binaries.
include(CodiraToolchainUtils)
if(NOT LANGUAGE_LIPO)
  find_toolchain_tool(LANGUAGE_LIPO "${LANGUAGE_DARWIN_XCRUN_TOOLCHAIN}" lipo)
endif()

get_filename_component(LANGUAGE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR} REALPATH)
set(LANGUAGE_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}")
set(LANGUAGE_CMAKE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")
set(LANGUAGE_MAIN_INCLUDE_DIR "${LANGUAGE_SOURCE_DIR}/include")
set(LANGUAGE_SHIMS_INCLUDE_DIR "${LANGUAGE_SOURCE_DIR}/stdlib/public/CodiraShims")
set(LANGUAGE_INCLUDE_DIR "${CMAKE_CURRENT_BINARY_DIR}/include")

if (NOT BOOTSTRAPPING_MODE AND LANGUAGE_ENABLE_LANGUAGE_IN_LANGUAGE)
  message(FATAL_ERROR "turning off bootstrapping is not supported anymore")
endif()

set(LANGUAGE_RUNTIME_OUTPUT_INTDIR "${CMAKE_BINARY_DIR}/${CMAKE_CFG_INTDIR}/bin")
set(LANGUAGE_LIBRARY_OUTPUT_INTDIR "${CMAKE_BINARY_DIR}/${CMAKE_CFG_INTDIR}/lib")

if(LANGUAGE_ENABLE_LANGUAGE_IN_LANGUAGE)
  if(NOT LANGUAGE_NATIVE_LANGUAGE_TOOLS_PATH)
    # This is the normal case. We are not cross-compiling.
    set(LANGUAGE_NATIVE_LANGUAGE_TOOLS_PATH "${LANGUAGE_RUNTIME_OUTPUT_INTDIR}")
    set(LANGUAGE_EXEC_FOR_LANGUAGE_MODULES "${CMAKE_Codira_COMPILER}")
    if(NOT LANGUAGE_EXEC_FOR_LANGUAGE_MODULES)
      message(WARNING "BOOTSTRAPPING set to OFF because no Codira compiler is defined")
      set(BOOTSTRAPPING_MODE "OFF")
    endif()
  elseif(BOOTSTRAPPING_MODE MATCHES "BOOTSTRAPPING.*")
    # If cross-compiling, we don't have to bootstrap. We can just use the previously
    # built native languagec to build the language compiler modules.
    message(STATUS "Building language modules with previously built tools instead of bootstrapping")
    set(LANGUAGE_EXEC_FOR_LANGUAGE_MODULES "${LANGUAGE_NATIVE_LANGUAGE_TOOLS_PATH}/languagec")
    if(BOOTSTRAPPING_MODE STREQUAL "BOOTSTRAPPING-WITH-HOSTLIBS")
      set(BOOTSTRAPPING_MODE "CROSSCOMPILE-WITH-HOSTLIBS")
    elseif(BOOTSTRAPPING_MODE STREQUAL "BOOTSTRAPPING")
      set(BOOTSTRAPPING_MODE "CROSSCOMPILE")
    else()
      set(BOOTSTRAPPING_MODE "HOSTTOOLS")
    endif()
  elseif(BOOTSTRAPPING_MODE STREQUAL "HOSTTOOLS" OR LANGUAGE_BUILD_LANGUAGE_SYNTAX)
    # We are building using a pre-installed host toolchain but not bootstrapping
    # the Codira modules. This happens when building using 'build-tooling-libs'
    # where we haven't built a new Codira compiler. Use the Codira compiler from the
    # pre-installed host toolchain to build the Codira modules.
    set(LANGUAGE_EXEC_FOR_LANGUAGE_MODULES "${CMAKE_Codira_COMPILER}")
  endif()
endif()

if(LANGUAGE_INCLUDE_TOOLS AND LANGUAGE_BUILD_LANGUAGE_SYNTAX AND LANGUAGE_ENABLE_LANGUAGE_IN_LANGUAGE)
  # Only "HOSTTOOLS" is supported in Linux when Codira parser integration is enabled.
  if(LANGUAGE_HOST_VARIANT_SDK MATCHES "LINUX|OPENBSD|FREEBSD" AND NOT BOOTSTRAPPING_MODE STREQUAL "HOSTTOOLS")
    message(WARNING "Force setting BOOTSTRAPPING=HOSTTOOLS because Codira parser integration is enabled")
    set(BOOTSTRAPPING_MODE "HOSTTOOLS")
    if(NOT CMAKE_Codira_COMPILER)
      message(SEND_ERROR "No Codira compiler found.\n"
        "Tell CMake where to find the Codira compiler by setting either the "
        "environment variable \"LANGUAGEC\" or the CMake cache entry "
        "CMAKE_Codira_COMPILER to the full path of the compiler, or to the "
        "compiler name if it is in the PATH")
    endif()
  endif()
  add_definitions(-DLANGUAGE_BUILD_LANGUAGE_SYNTAX)
endif()

if(BOOTSTRAPPING_MODE MATCHES "HOSTTOOLS|.*-WITH-HOSTLIBS")
  if(LANGUAGE_ENABLE_ARRAY_COW_CHECKS)
    message(STATUS "array COW checks disabled when building the language modules with host libraries")
    set(LANGUAGE_ENABLE_ARRAY_COW_CHECKS FALSE)
  endif()
endif()

# This setting causes all CMakeLists.txt to automatically have
# ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CURRENT_SOURCE_DIR} as an
# include_directories path. This is done for developer
# convenience. Additionally, LLVM/Clang build with this option enabled, so we
# should match them unless it is removed from LLVM/Clang as well.
#
# *NOTE* Even though these directories are added to the include path for a
# specific CMakeLists.txt, these include paths are not propagated down to
# subdirectories.
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# We'll need this once we have generated headers
include_directories(BEFORE
  ${LANGUAGE_MAIN_INCLUDE_DIR}
  ${LANGUAGE_INCLUDE_DIR}
  ${LANGUAGE_SHIMS_INCLUDE_DIR}
  )

# Configuration flags passed to all of our invocations of gyb.  Try to
# avoid making up new variable names here if you can find a CMake
# variable that will do the job.
set(LANGUAGE_GYB_FLAGS
    "-DunicodeGraphemeBreakPropertyFile=${LANGUAGE_SOURCE_DIR}/utils/UnicodeData/GraphemeBreakProperty.txt"
    "-DunicodeGraphemeBreakTestFile=${LANGUAGE_SOURCE_DIR}/utils/UnicodeData/GraphemeBreakTest.txt")

# Directory to use as the Clang module cache when building Codira source files.
set(LANGUAGE_MODULE_CACHE_PATH
    "${CMAKE_BINARY_DIR}/${CMAKE_CFG_INTDIR}/module-cache")

# Check what linux distribution is being used.
# This can be used to determine the default linker to use.
if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.22")
  cmake_host_system_information(RESULT DISTRO_NAME  QUERY DISTRIB_PRETTY_NAME)
endif()

# Which default linker to use. Prefer TOOLCHAIN_USE_LINKER if it set, otherwise use
# our own defaults. This should only be possible in a unified (not stand alone)
# build environment.
include(GoldVersion)

if(TOOLCHAIN_USE_LINKER)
  set(LANGUAGE_USE_LINKER_default "${TOOLCHAIN_USE_LINKER}")
elseif(LANGUAGE_HOST_VARIANT_SDK STREQUAL "ANDROID")
  set(LANGUAGE_USE_LINKER_default "lld")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows" AND NOT CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
  set(LANGUAGE_USE_LINKER_default "lld")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  set(LANGUAGE_USE_LINKER_default "")
elseif(DISTRO_NAME STREQUAL "Amazon Linux 2023")
  set(LANGUAGE_USE_LINKER_default "lld")
elseif(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
	set(LANGUAGE_USE_LINKER_default "lld")
else()
  get_gold_version(gold_version)
  if(NOT gold_version)
    message(STATUS "GNU Gold not found; using lld instead")
    set(LANGUAGE_USE_LINKER_default "lld")
  elseif(gold_version VERSION_LESS "2.36")
    message(STATUS "GNU Gold is too old (${gold_version}); using lld instead")
    set(LANGUAGE_USE_LINKER_default "lld")
  else()
    message(STATUS "Using GNU Gold ${gold_version}")
    set(LANGUAGE_USE_LINKER_default "gold")
  endif()
endif()
set(LANGUAGE_USE_LINKER ${LANGUAGE_USE_LINKER_default} CACHE STRING
    "Build Codira with a non-default linker")

include(CheckLinkerFlag)

# Apple's linker complains about duplicate libraries, which CMake likes to do
# to support ELF platforms. To silence that warning, we can use
# -no_warn_duplicate_libraries, but only in versions of the linker that
# support that flag.
if(NOT TOOLCHAIN_USE_LINKER AND ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  check_linker_flag(C "-Wl,-no_warn_duplicate_libraries" LANGUAGE_LINKER_SUPPORTS_NO_WARN_DUPLICATE_LIBRARIES)
else()
  set(LANGUAGE_LINKER_SUPPORTS_NO_WARN_DUPLICATE_LIBRARIES OFF CACHE INTERNAL "")
endif()

#
# Enable additional warnings.
#
language_common_cxx_warnings()

# Set sanitizer options for Codira compiler.
language_common_sanitizer_config()

# Check if we're build with MSVC or Clang-cl, as these compilers have similar command line arguments.
if("${CMAKE_C_COMPILER_ID}" STREQUAL "MSVC" OR "${CMAKE_CXX_SIMULATE_ID}" STREQUAL "MSVC")
  set(LANGUAGE_COMPILER_IS_MSVC_LIKE TRUE)
endif()

#
# Display a message if the threading package has been overridden
#

if(LANGUAGE_THREADING_PACKAGE)
  message(STATUS "")
  message(STATUS "Threading package override enabled")
  foreach(elt ${LANGUAGE_THREADING_PACKAGE})
    string(REPLACE ":" ";" elt_list "${elt}")
    list(LENGTH elt_list elt_list_len)
    if(elt_list_len EQUAL 1)
      set(elt_sdk "Global")
      list(GET elt_list 0 elt_package)
    elseif(elt_list_len EQUAL 2)
      list(GET elt_list 0 elt_sdk)
      list(GET elt_list 1 elt_package)
      string(TOUPPER "${elt_sdk}" elt_sdk)
    else()
      message(FATAL_ERROR "Bad threading override \"${elt}\" - LANGUAGE_THREADING_PACKAGE must be a semicolon separated list of package or sdk:package pairs.")
    endif()
    string(TOLOWER "${elt_package}" elt_package)
    message(STATUS "  ${elt_sdk}: ${elt_package}")
  endforeach()
  message(STATUS "")
endif()

#
# Configure SDKs.
#

# FIXME: the parameters we specify in LANGUAGE_SDKS are lacking architecture specifics,
# so we need to hard-code it. For example, the SDK for Android is just 'ANDROID',
# and we have to specify LANGUAGE_SDK_ANDROID_ARCHITECTURES separately.
# The iOS SDKs all have their architectures hardcoded because they are just specified by name (e.g. 'IOS' or 'WATCHOS').
# We can't cross-compile the standard library for another linux architecture,
# because the SDK list would just be 'LINUX' and we couldn't disambiguate it from the host.
#
# To fix it, we would need to append the architecture to the SDKs,
# for example: 'OSX-x86_64;IOS-armv7;...etc'.
# We could easily do that - we have all of that information in build-script-impl.
# Darwin targets cheat and use `xcrun`.

if("${LANGUAGE_HOST_VARIANT_SDK}" STREQUAL "LINUX")

  set(LANGUAGE_HOST_VARIANT "linux" CACHE STRING
      "Deployment OS for Codira host tools (the compiler) [linux].")

  is_sdk_requested(LINUX language_build_linux)
  if(language_build_linux)
    if("${LANGUAGE_SDK_LINUX_ARCHITECTURES}" STREQUAL "")
      set(LANGUAGE_SDK_LINUX_ARCHITECTURES "${LANGUAGE_HOST_VARIANT_ARCH}")
    endif()

    configure_sdk_unix("Linux" "${LANGUAGE_SDK_LINUX_ARCHITECTURES}")
  endif()

  is_sdk_requested(LINUX_STATIC language_build_linux_static)
  if(language_build_linux_static)
    if("${LANGUAGE_MUSL_PATH}" STREQUAL "")
      message(FATAL_ERROR "You must set LANGUAGE_MUSL_PATH to point to the Musl libraries and headers.  Specifically, we expect to find Musl at <LANGUAGE_MUSL_PATH>/<arch> for each requested architecture.")
    endif()

    if("${LANGUAGE_SDK_LINUX_STATIC_ARCHITECTURES}" STREQUAL "")
      set(LANGUAGE_SDK_LINUX_STATIC_ARCHITECTURES "aarch64;x86_64")
    endif()

    configure_sdk_unix("Linux_Static" "${LANGUAGE_SDK_LINUX_STATIC_ARCHITECTURES}")
  endif()

  is_sdk_requested(FREESTANDING language_build_freestanding)
  if(language_build_freestanding AND (LANGUAGE_FREESTANDING_FLAVOR STREQUAL "linux"))
    # TODO
    # configure_sdk_unix("FREESTANDING" "${LANGUAGE_HOST_VARIANT_ARCH}")
  endif()

  # Default is Linux SDK for host
  set(LANGUAGE_PRIMARY_VARIANT_SDK_default  "${LANGUAGE_HOST_VARIANT_SDK}")
  set(LANGUAGE_PRIMARY_VARIANT_ARCH_default "${LANGUAGE_HOST_VARIANT_ARCH}")

elseif("${LANGUAGE_HOST_VARIANT_SDK}" STREQUAL "FREEBSD")

  set(LANGUAGE_HOST_VARIANT "freebsd" CACHE STRING
      "Deployment OS for Codira host tools (the compiler) [freebsd].")

  configure_sdk_unix("FreeBSD" "${LANGUAGE_HOST_VARIANT_ARCH}")
  set(LANGUAGE_PRIMARY_VARIANT_SDK_default  "${LANGUAGE_HOST_VARIANT_SDK}")
  set(LANGUAGE_PRIMARY_VARIANT_ARCH_default "${LANGUAGE_HOST_VARIANT_ARCH}")

elseif("${LANGUAGE_HOST_VARIANT_SDK}" STREQUAL "OPENBSD")

  set(LANGUAGE_HOST_VARIANT "openbsd" CACHE STRING
      "Deployment OS for Codira host tools (the compiler) [openbsd].")

  configure_sdk_unix("OpenBSD" "${LANGUAGE_HOST_VARIANT_ARCH}")
  set(LANGUAGE_PRIMARY_VARIANT_SDK_default  "${LANGUAGE_HOST_VARIANT_SDK}")
  set(LANGUAGE_PRIMARY_VARIANT_ARCH_default "${LANGUAGE_HOST_VARIANT_ARCH}")

elseif("${LANGUAGE_HOST_VARIANT_SDK}" STREQUAL "CYGWIN")

  set(LANGUAGE_HOST_VARIANT "cygwin" CACHE STRING
      "Deployment OS for Codira host tools (the compiler) [cygwin].")

  configure_sdk_unix("Cygwin" "${LANGUAGE_HOST_VARIANT_ARCH}")
  set(LANGUAGE_PRIMARY_VARIANT_SDK_default "${LANGUAGE_HOST_VARIANT_SDK}")
  set(LANGUAGE_PRIMARY_VARIANT_ARCH_default "${LANGUAGE_HOST_VARIANT_ARCH}")

elseif("${LANGUAGE_HOST_VARIANT_SDK}" STREQUAL "WINDOWS")

  set(LANGUAGE_HOST_VARIANT "windows" CACHE STRING
      "Deployment OS for Codira host tools (the compiler) [windows].")

  configure_sdk_windows("Windows" "msvc" "${LANGUAGE_HOST_VARIANT_ARCH}")
  set(LANGUAGE_PRIMARY_VARIANT_SDK_default  "${LANGUAGE_HOST_VARIANT_SDK}")
  set(LANGUAGE_PRIMARY_VARIANT_ARCH_default "${LANGUAGE_HOST_VARIANT_ARCH}")

elseif("${LANGUAGE_HOST_VARIANT_SDK}" STREQUAL "HAIKU")

  set(LANGUAGE_HOST_VARIANT "haiku" CACHE STRING
      "Deployment OS for Codira host tools (the compiler) [haiku].")

  configure_sdk_unix("Haiku" "${LANGUAGE_HOST_VARIANT_ARCH}")
  set(LANGUAGE_PRIMARY_VARIANT_SDK_default  "${LANGUAGE_HOST_VARIANT_SDK}")
  set(LANGUAGE_PRIMARY_VARIANT_ARCH_default "${LANGUAGE_HOST_VARIANT_ARCH}")

elseif("${LANGUAGE_HOST_VARIANT_SDK}" STREQUAL "ANDROID")

  set(LANGUAGE_HOST_VARIANT "android" CACHE STRING
      "Deployment OS for Codira host tools (the compiler) [android]")

  set(LANGUAGE_ANDROID_NATIVE_SYSROOT "/data/data/com.termux/files" CACHE STRING
      "Path to Android sysroot, default initialized to the Termux app's layout")

  if("${LANGUAGE_SDK_ANDROID_ARCHITECTURES}" STREQUAL "")
    set(LANGUAGE_SDK_ANDROID_ARCHITECTURES ${LANGUAGE_HOST_VARIANT_ARCH})
  endif()

  configure_sdk_unix("Android" "${LANGUAGE_SDK_ANDROID_ARCHITECTURES}")
  set(LANGUAGE_PRIMARY_VARIANT_SDK_default  "${LANGUAGE_HOST_VARIANT_SDK}")
  set(LANGUAGE_PRIMARY_VARIANT_ARCH_default "${LANGUAGE_HOST_VARIANT_ARCH}")

elseif("${LANGUAGE_HOST_VARIANT_SDK}" STREQUAL "WASI")
  set(LANGUAGE_HOST_VARIANT "wasi" CACHE STRING
      "Deployment OS for Codira host tools (the compiler) [wasi]")

  configure_sdk_unix("WASI" "wasm32")
  set(LANGUAGE_PRIMARY_VARIANT_SDK_default  "${LANGUAGE_HOST_VARIANT_SDK}")
  set(LANGUAGE_PRIMARY_VARIANT_ARCH_default "${LANGUAGE_HOST_VARIANT_ARCH}")

elseif("${LANGUAGE_HOST_VARIANT_SDK}" STREQUAL "EMSCRIPTEN")

  set(LANGUAGE_HOST_VARIANT "emscripten" CACHE STRING
      "Deployment OS for Codira host tools (the compiler) [emscripten]")

  configure_sdk_unix("Emscripten" "wasm32")
  set(LANGUAGE_PRIMARY_VARIANT_SDK_default  "${LANGUAGE_HOST_VARIANT_SDK}")
  set(LANGUAGE_PRIMARY_VARIANT_ARCH_default "${LANGUAGE_HOST_VARIANT_ARCH}")

elseif("${LANGUAGE_HOST_VARIANT_SDK}" MATCHES "(OSX|IOS*|TVOS*|WATCHOS*|XROS*)")

  set(LANGUAGE_HOST_VARIANT "macosx" CACHE STRING
      "Deployment OS for Codira host tools (the compiler) [macosx, iphoneos].")

  # Display Xcode toolchain version.
  # The SDK configuration below prints each SDK version.
  execute_process(
    COMMAND "xcodebuild" "-version"
    OUTPUT_VARIABLE xcode_version
    OUTPUT_STRIP_TRAILING_WHITESPACE)
  string(REPLACE "\n" ", " xcode_version "${xcode_version}")
  message(STATUS "${xcode_version}")
  message(STATUS "")

  include(DarwinSDKs)

  # FIXME: guess target variant based on the host.
  # if(LANGUAGE_HOST_VARIANT MATCHES "^macosx")
  #   set(LANGUAGE_PRIMARY_VARIANT_GUESS "OSX-R")
  # elseif(LANGUAGE_HOST_VARIANT MATCHES "^iphoneos")
  #   set(LANGUAGE_PRIMARY_VARIANT_GUESS "IOS-R")
  # else()
  #   message(FATAL_ERROR "Unknown LANGUAGE_HOST_VARIANT '${LANGUAGE_HOST_VARIANT}'")
  # endif()
  #
  # set(LANGUAGE_PRIMARY_VARIANT ${LANGUAGE_PRIMARY_VARIANT_GUESS} CACHE STRING
  #    "[OSX-DA, OSX-RA, OSX-R, IOS-DA, IOS-RA, IOS-R, IOS_SIMULATOR-DA, IOS_SIMULATOR-RA, IOS_SIMULATOR-R]")
  #
  # Primary variant is always OSX; even on iOS hosts.
  set(LANGUAGE_PRIMARY_VARIANT_SDK_default "OSX")
  set(LANGUAGE_PRIMARY_VARIANT_ARCH_default "${CMAKE_HOST_SYSTEM_PROCESSOR}")

endif()

if("${LANGUAGE_PRIMARY_VARIANT_SDK}" STREQUAL "")
  set(LANGUAGE_PRIMARY_VARIANT_SDK "${LANGUAGE_PRIMARY_VARIANT_SDK_default}")
endif()
if("${LANGUAGE_PRIMARY_VARIANT_ARCH}" STREQUAL "")
  set(LANGUAGE_PRIMARY_VARIANT_ARCH "${LANGUAGE_PRIMARY_VARIANT_ARCH_default}")
endif()

# Should we cross-compile the standard library for Android?
is_sdk_requested(ANDROID language_build_android)
if(language_build_android AND NOT "${LANGUAGE_HOST_VARIANT_SDK}" STREQUAL "ANDROID")
  if ("${LANGUAGE_ANDROID_NDK_PATH}" STREQUAL "")
    message(FATAL_ERROR "You must set LANGUAGE_ANDROID_NDK_PATH to cross-compile the Codira runtime for Android")
  endif()
  if (NOT ("${CMAKE_HOST_SYSTEM_NAME}" STREQUAL "Darwin" OR "${CMAKE_HOST_SYSTEM_NAME}" STREQUAL "Linux"))
    message(FATAL_ERROR "A Darwin or Linux host is required to build the Codira runtime for Android")
  endif()

  if("${LANGUAGE_SDK_ANDROID_ARCHITECTURES}" STREQUAL "")
    set(LANGUAGE_SDK_ANDROID_ARCHITECTURES armv7;aarch64)
  endif()
  configure_sdk_unix("Android" "${LANGUAGE_SDK_ANDROID_ARCHITECTURES}")
endif()

# Should we cross-compile the standard library for Windows?
is_sdk_requested(WINDOWS language_build_windows)
if(language_build_windows AND NOT "${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
  if("${LANGUAGE_SDK_WINDOWS_ARCHITECTURES}" STREQUAL "")
    set(LANGUAGE_SDK_WINDOWS_ARCHITECTURES aarch64;armv7;i686;x86_64)
  endif()
  configure_sdk_windows("Windows" "msvc" "${LANGUAGE_SDK_WINDOWS_ARCHITECTURES}")
endif()

# Should we cross-compile the standard library for WASI?
is_sdk_requested(WASI language_build_wasm)
if(language_build_wasm AND NOT "${LANGUAGE_HOST_VARIANT_SDK}" STREQUAL "WASI")
  configure_sdk_unix(WASI wasm32)
endif()

if("${LANGUAGE_SDKS}" STREQUAL "")
  set(LANGUAGE_SDKS "${LANGUAGE_CONFIGURED_SDKS}")
endif()

list_subtract("${LANGUAGE_SDKS}" "${LANGUAGE_CONFIGURED_SDKS}" unknown_sdks)

precondition(unknown_sdks NEGATE MESSAGE "Unknown SDKs: ${unknown_sdks}")
precondition(LANGUAGE_CONFIGURED_SDKS MESSAGE "No SDKs selected.")
precondition(LANGUAGE_HOST_VARIANT_SDK MESSAGE "No SDK for host tools.")
precondition(LANGUAGE_HOST_VARIANT_ARCH MESSAGE "No arch for host tools")

set(LANGUAGE_PRIMARY_VARIANT_SUFFIX
    "-${LANGUAGE_SDK_${LANGUAGE_PRIMARY_VARIANT_SDK}_LIB_SUBDIR}-${LANGUAGE_PRIMARY_VARIANT_ARCH}")

# Clear universal library names to prevent adding duplicates
foreach(sdk ${LANGUAGE_SDKS})
  unset(UNIVERSAL_LIBRARY_NAMES_${LANGUAGE_SDK_${sdk}_LIB_SUBDIR} CACHE)
endforeach()

if(LANGUAGE_PARALLEL_LINK_JOBS)
  if(NOT CMAKE_MAKE_PROGRAM MATCHES "ninja")
    message(WARNING "Job pooling is only available with Ninja generators.")
  else()
    set_property(GLOBAL APPEND PROPERTY JOB_POOLS language_link_job_pool=${LANGUAGE_PARALLEL_LINK_JOBS})
    set(CMAKE_JOB_POOL_LINK language_link_job_pool)
  endif()
endif()

# Set the CMAKE_OSX_* variables in a way that minimizes conflicts.
if("${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin" AND NOT CMAKE_CROSSCOMPILING)
  set(CMAKE_OSX_SYSROOT "${LANGUAGE_SDK_${LANGUAGE_HOST_VARIANT_SDK}_PATH}")
  set(CMAKE_OSX_ARCHITECTURES "")
  set(CMAKE_OSX_DEPLOYMENT_TARGET "")
endif()

language_get_host_triple(LANGUAGE_HOST_TRIPLE)
set(LANGUAGE_HOST_MODULE_TRIPLE "${LANGUAGE_SDK_${LANGUAGE_HOST_VARIANT_SDK}_ARCH_${LANGUAGE_HOST_VARIANT_ARCH}_MODULE}")
set(LANGUAGE_HOST_LIBRARIES_DEST_DIR "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/language/host")

if(LANGUAGE_INCLUDE_TOOLS)
  message(STATUS "Building host Codira tools for ${LANGUAGE_HOST_VARIANT_SDK} ${LANGUAGE_HOST_VARIANT_ARCH}")
  message(STATUS "  Build type:     ${CMAKE_BUILD_TYPE}")
  message(STATUS "  Assertions:     ${TOOLCHAIN_ENABLE_ASSERTIONS}")
  message(STATUS "  LTO:            ${LANGUAGE_TOOLS_ENABLE_LTO}")
  message(STATUS "  Bootstrapping:  ${BOOTSTRAPPING_MODE}")
  message(STATUS "  C++ Bridging:   ${BRIDGING_MODE}")
  message(STATUS "  Codira parser:   ${LANGUAGE_BUILD_LANGUAGE_SYNTAX}")
  message(STATUS "")
else()
  message(STATUS "Not building host Codira tools")
  message(STATUS "")
endif()

if(LANGUAGE_BUILD_STDLIB OR LANGUAGE_BUILD_SDK_OVERLAY)
  message(STATUS "Building Codira standard library and overlays for SDKs: ${LANGUAGE_SDKS}")
  message(STATUS "  Build type:               ${LANGUAGE_STDLIB_BUILD_TYPE}")
  message(STATUS "  Assertions:               ${LANGUAGE_STDLIB_ASSERTIONS}")
  message(STATUS "  Strict availability:      ${LANGUAGE_STDLIB_ENABLE_STRICT_AVAILABILITY}")
  message(STATUS "")

  message(STATUS "Building Codira runtime with:")
  message(STATUS "  Leak Detection Checker Entrypoints: ${LANGUAGE_RUNTIME_ENABLE_LEAK_CHECKER}")
  message(STATUS "")

  message(STATUS "Differentiable Programming Support: ${LANGUAGE_ENABLE_EXPERIMENTAL_DIFFERENTIABLE_PROGRAMMING}")
  message(STATUS "Concurrency Support:                ${LANGUAGE_ENABLE_EXPERIMENTAL_CONCURRENCY}")
  message(STATUS "Distributed Support:                ${LANGUAGE_ENABLE_EXPERIMENTAL_DISTRIBUTED}")
  message(STATUS "String Processing Support:          ${LANGUAGE_ENABLE_EXPERIMENTAL_STRING_PROCESSING}")
  message(STATUS "Backtracing Support:                ${LANGUAGE_ENABLE_BACKTRACING}")
  message(STATUS "Unicode Support:                    ${LANGUAGE_STDLIB_ENABLE_UNICODE_DATA}")
  message(STATUS "Observation Support:                ${LANGUAGE_ENABLE_EXPERIMENTAL_OBSERVATION}")
  message(STATUS "Synchronization Support:            ${LANGUAGE_ENABLE_SYNCHRONIZATION}")
  message(STATUS "Volatile Support:                   ${LANGUAGE_ENABLE_VOLATILE}")
  message(STATUS "Pointer Bounds Support:             ${LANGUAGE_ENABLE_EXPERIMENTAL_POINTER_BOUNDS}")
  message(STATUS "Runtime Support:                    ${LANGUAGE_ENABLE_RUNTIME_MODULE}")
  message(STATUS "")
else()
  message(STATUS "Not building Codira standard library, SDK overlays, and runtime")
  message(STATUS "")
endif()

if(LANGUAGE_BUILD_LIBEXEC)
  message(STATUS "Building Codira auxiliary executables for SDKs: ${LANGUAGE_SDKS}")
  message(STATUS "")
endif()

if(LANGUAGE_BUILD_REMOTE_MIRROR)
  message(STATUS "Building Codira Remote Mirror for SDKs: ${LANGUAGE_SDKS}")
  message(STATUS "")
endif()

#
# Find required dependencies.
#

find_package(Python3 3.6 COMPONENTS Interpreter REQUIRED)

#
# Find optional dependencies.
#

if(TOOLCHAIN_ENABLE_LIBXML2)
  find_package(LibXml2 REQUIRED)
else()
  find_package(LibXml2)
endif()

if(TOOLCHAIN_ENABLE_LIBEDIT)
  find_package(LibEdit REQUIRED)
else()
  find_package(LibEdit)
endif()

if(LibEdit_FOUND)
  cmake_push_check_state()
  list(APPEND CMAKE_REQUIRED_INCLUDES ${LibEdit_INCLUDE_DIRS})
  list(APPEND CMAKE_REQUIRED_LIBRARIES ${LibEdit_LIBRARIES})
  check_symbol_exists(el_wgets "histedit.h" HAVE_EL_WGETS)
  if(HAVE_EL_WGETS)
    set(LibEdit_HAS_UNICODE YES)
  else()
    set(LibEdit_HAS_UNICODE NO)
  endif()
  cmake_pop_check_state()
endif()

check_symbol_exists(wait4 "sys/wait.h" HAVE_WAIT4)

check_symbol_exists(proc_pid_rusage "libproc.h" HAVE_PROC_PID_RUSAGE)
if(HAVE_PROC_PID_RUSAGE)
    list(APPEND CMAKE_REQUIRED_LIBRARIES proc)
endif()

if (TOOLCHAIN_ENABLE_DOXYGEN)
  message(STATUS "Doxygen: enabled")
endif()

if(LANGUAGE_ENABLE_DISPATCH)
  include(Libdispatch)
endif()

# Add all of the subdirectories, where we actually do work.
add_subdirectory(include)
if(LANGUAGE_INCLUDE_TOOLS)
  add_subdirectory(lib)
endif()

###############
# PLEASE READ #
###############
#
# We have to include stdlib/ before tools/.
# Do not move add_subdirectory(stdlib) after add_subdirectory(tools)!
#
# We must include stdlib/ before tools/ because stdlib/CMakeLists.txt
# declares the language-stdlib-* set of targets. These targets will then
# implicitly depend on any targets declared with IS_STDLIB.
#
# https://github.com/apple/language/issues/48534
if(LANGUAGE_BUILD_STDLIB)
  add_subdirectory(stdlib)
else()
  set(LANGUAGE_STDLIB_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/stdlib")
  # Some of the things below depend on the threading library
  add_subdirectory(stdlib/public/Threading)

  if(LANGUAGE_BUILD_STDLIB_EXTRA_TOOLCHAIN_CONTENT)
    add_subdirectory(stdlib/toolchain)

    if(LANGUAGE_BUILD_STDLIB_CXX_MODULE)
      add_subdirectory(stdlib/public/Cxx)
    endif()
  endif()

  # Some tools (e.g. language-reflection-dump) rely on a host languageRemoteInspection,
  # so ensure we build that when building tools.
  if(LANGUAGE_INCLUDE_TOOLS OR LANGUAGE_BUILD_STDLIB_CXX_MODULE)
    add_subdirectory(stdlib/public/CodiraShims/language/shims)
  endif()

  # We might want to build Remote Mirror separately
  if(LANGUAGE_BUILD_REMOTE_MIRROR)
    add_subdirectory(stdlib/public/LLVMSupport)
    add_subdirectory(stdlib/public/Demangling)
    add_subdirectory(stdlib/public/RemoteInspection)
    add_subdirectory(stdlib/public/CodiraRemoteMirror)
  endif()

  # We might also want to build the things in libexec seaprately
  if(LANGUAGE_BUILD_LIBEXEC)
    add_subdirectory(stdlib/public/libexec)
  endif()
endif()

if(LANGUAGE_INCLUDE_APINOTES)
  add_subdirectory(apinotes)
endif()

if(LANGUAGE_INCLUDE_TOOLS)
  add_subdirectory(CodiraCompilerSources)

  # Always include this after including stdlib/!
  # Refer to the large comment above the add_subdirectory(stdlib) call.
  # https://github.com/apple/language/issues/48534
  add_subdirectory(tools)

  if(LANGUAGE_NATIVE_LANGUAGE_TOOLS_PATH)
    # Localization targets are configured in a way that assume the language
    # frontend is being built, so trying to include them for other builds
    # (like stdlib) fail!
    #
    # Diagnostics information is only useful for the frontend compiler
    # anyway, so let's only include it if the compiler is being built,
    # which at the moment seems like if LANGUAGE_INCLUDE_TOOLS is defined.
    add_subdirectory(localization)
  endif()
endif()

add_subdirectory(utils)

add_subdirectory(userdocs)

if ("${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
  if(LANGUAGE_BUILD_PERF_TESTSUITE)
    add_subdirectory(benchmark)
  endif()
endif()

if(LANGUAGE_INCLUDE_TESTS)
  add_subdirectory(test)
  add_subdirectory(unittests)
endif()
if(LANGUAGE_INCLUDE_DOCS)
  add_subdirectory(docs)
endif()

add_subdirectory(cmake/modules)

language_install_in_component(FILES "LICENSE.txt"
                           DESTINATION "share/language"
                           COMPONENT license)

# New standard library build
option(LANGUAGE_ENABLE_NEW_RUNTIME_BUILD "Build Codira runtimes with new build system" OFF)
if(LANGUAGE_ENABLE_NEW_RUNTIME_BUILD)
  message(STATUS "New runtime build enabled")

  add_custom_target(PopulateRuntimeSourceDir
    COMMAND "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_SOURCE_DIR}/Runtimes/Resync.cmake"
    COMMENT "Copying sources into new runtime build")

  foreach(sdk ${LANGUAGE_SDKS})
    foreach(arch ${LANGUAGE_SDK_${sdk}_ARCHITECTURES})
      # Provide a mechanism to skip building one of these runtimes
      if(LANGUAGE_SKIP_${sdk}_${arch}_RUNTIME)
        message(STATUS "Skipping: ${arch}-${sdk} runtime build")
        continue()
      endif()
      set(stdlib_target "language-stdlib-${LANGUAGE_SDK_${sdk}_LIB_SUBDIR}-${arch}")
      if(${LANGUAGE_SDK_${sdk}_${arch}_CACHE})
        set(stdlib_cache_file_flag -C ${LANGUAGE_SDK_${sdk}_${arch}_CACHE})
      endif()

      set(stdlib_target_triple ${LANGUAGE_SDK_${sdk}_ARCH_${arch}_TRIPLE})
      if(LANGUAGE_SDK_${sdk}_DEPLOYMENT_VERSION)
        string(APPEND stdlib_target_triple ${LANGUAGE_SDK_${sdk}_DEPLOYMENT_VERSION})
        set(stdlib_deployment_version_flag -DCMAKE_OSX_DEPLOYMENT_TARGET=${LANGUAGE_SDK_${sdk}_DEPLOYMENT_VERSION})
      endif()

      if(sdk STREQUAL "OSX" AND LANGUAGE_SDK_${sdk}_DEPLOYMENT_VERSION VERSION_LESS "10.15")
        set(build_concurrency NO)
      else()
        set(build_concurrency YES)
      endif()

      ExternalProject_Add("${stdlib_target}-core"
        SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Runtimes/Core"
        # TODO: Add this once we're ready to start swapping out the libraries
        #       for testing
        # INSTALL_DIR "${CMAKE_BINARY_DIR}/"

        DEPENDS PopulateRuntimeSourceDir
        # To ensure incremental builds work as expected
        BUILD_ALWAYS 1
        CMAKE_ARGS
          -DCMAKE_INSTALL_LIBDIR:FILEPATH=lib
          # Compiler will see mismatched language modules and fail initial checks
          -DCMAKE_Codira_COMPILER_WORKS:BOOLEAN=YES
          -DBUILD_SHARED_LIBS:BOOLEAN=YES # TODO: Make this configurable
          ${stdlib_cache_file_flag}
          ${stdlib_deployment_version_flag}
          -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
          -DCMAKE_INSTALL_PREFIX:FILEPATH=<INSTALL_DIR>
          -DCMAKE_Codira_COMPILER:FILEPATH=$<IF:$<BOOL:${CMAKE_CROSSCOMPILING}>,${CMAKE_Codira_COMPILER},$<PATH:REPLACE_FILENAME,$<TARGET_FILE:language-frontend>,languagec>>
          -DCMAKE_C_COMPILER:FILEPATH=${CMAKE_C_COMPILER}
          -DCMAKE_CXX_COMPILER:FILEPATH=${CMAKE_CXX_COMPILER}
          -DCMAKE_Codira_COMPILER_TARGET:STRING=${stdlib_target_triple}
          -DCMAKE_C_COMPILER_TARGET:STRING=${stdlib_target_triple}
          -DCMAKE_CXX_COMPILER_TARGET:STRING=${stdlib_target_triple}
          -DCMAKE_COLOR_DIAGNOSTICS:BOOLEAN=${CMAKE_COLOR_DIAGNOSTICS}
          -DCMAKE_MAKE_PROGRAM=${CMAKE_MAKE_PROGRAM}
          -DCodiraCore_INSTALL_NESTED_SUBDIR=YES
          -DCodiraCore_ENABLE_CONCURRENCY=${build_concurrency})
      if(NOT ${CMAKE_CROSSCOMPILING})
        add_dependencies("${stdlib_target}-core" language-frontend)
      endif()

      ExternalProject_Get_Property("${stdlib_target}-core" INSTALL_DIR)

      ExternalProject_Add("${stdlib_target}-Supplemental"
        SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Runtimes/Supplemental"
        DEPENDS "${stdlib_target}-core"
        INSTALL_DIR "${INSTALL_DIR}"
        INSTALL_COMMAND ""
        LIST_SEPARATOR "|"
        # To ensure incremental builds work as expected
        BUILD_ALWAYS 1
        CMAKE_ARGS
          -DCodira_ENABLE_RUNTIMES=StringProcessing
          -DBUILD_SHARED_LIBS=YES
          -DCMAKE_Codira_COMPILER_WORKS:BOOLEAN=YES
          -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
          -DCMAKE_INSTALL_PREFIX:FILEPATH=${INSTALL_DIR}
          -DCMAKE_Codira_COMPILER:FILEPATH=$<IF:$<BOOL:${CMAKE_CROSSCOMPILING}>,${CMAKE_Codira_COMPILER},$<PATH:REPLACE_FILENAME,$<TARGET_FILE:language-frontend>,languagec>>
          -DCMAKE_C_COMPILER:FILEPATH=${CMAKE_C_COMPILER}
          -DCMAKE_CXX_COMPILER:FILEPATH=${CMAKE_CXX_COMPILER}
          -DCMAKE_Codira_COMPILER_TARGET:STRING=${stdlib_target_triple}
          -DCMAKE_C_COMPILER_TARGET:STRING=${stdlib_target_triple}
          -DCMAKE_CXX_COMPILER_TARGET:STRING=${stdlib_target_triple}
          -DCMAKE_COLOR_DIAGNOSTICS:BOOLEAN=${CMAKE_COLOR_DIAGNOSTICS}
          -DCMAKE_MAKE_PROGRAM=${CMAKE_MAKE_PROGRAM}
          -DCMAKE_FIND_PACKAGE_PREFER_CONFIG=TRUE)
    endforeach()
  endforeach()
endif()
