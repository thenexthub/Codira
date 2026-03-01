#===----------------------------------------------------------------------===#
#   Copyright (c) NeXTHub Corporation. All Rights Reserved.
#   DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
#   Author: Tunjay Akbarli
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at:
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
#
#   Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
#   Middletown, DE 19709, New Castle County, USA.
#===----------------------------------------------------------------------===#

function(panda_set_flag flag)
    set(${flag} 1 PARENT_SCOPE)
    add_definitions("-D${flag}")
endfunction()

# For each CMake variable name, add a corresponding preprocessor definition
# if variable evaluates to True.
function(panda_promote_to_definitions)
    foreach(var_name ${ARGV})
        if(${var_name})
            add_definitions("-D${var_name}")
        endif()
    endforeach()
endfunction()

set(CMAKE_BUILD_RPATH_USE_ORIGIN TRUE)
add_compile_definitions(PANDA_CMAKE_SDK)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    panda_set_flag(PANDA_TARGET_LINUX)
    panda_set_flag(PANDA_TARGET_UNIX)
    if (NOT PANDA_ENABLE_ADDRESS_SANITIZER)
        panda_set_flag(PANDA_USE_FUTEX)
    endif()
elseif(CMAKE_SYSTEM_NAME STREQUAL "OHOS")
    panda_set_flag(PANDA_TARGET_OHOS)
    panda_set_flag(PANDA_TARGET_UNIX)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Android")
    panda_set_flag(PANDA_TARGET_MOBILE)
    panda_set_flag(PANDA_TARGET_UNIX)
    if (NOT PANDA_ENABLE_ADDRESS_SANITIZER)
        panda_set_flag(PANDA_USE_FUTEX)
    endif()
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    panda_set_flag(PANDA_TARGET_WINDOWS)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    panda_set_flag(PANDA_TARGET_MACOS)
    panda_set_flag(PANDA_TARGET_UNIX)
else()
    message(FATAL_ERROR "Platform ${CMAKE_SYSTEM_NAME} is not supported")
endif()

if(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" OR CMAKE_SYSTEM_PROCESSOR STREQUAL "AMD64")
    if(NOT PANDA_CROSS_AMD64_X86)
        panda_set_flag(PANDA_TARGET_AMD64)
    else()
        panda_set_flag(PANDA_TARGET_X86)
    endif()
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "i[356]86")
    panda_set_flag(PANDA_TARGET_X86)
elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64")
    panda_set_flag(PANDA_TARGET_ARM64)
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "arm")
    panda_set_flag(PANDA_TARGET_ARM32)
    panda_promote_to_definitions(
        PANDA_TARGET_ARM32_ABI_SOFT
        PANDA_TARGET_ARM32_ABI_SOFTFP
        PANDA_TARGET_ARM32_ABI_HARD
    )
    if((PANDA_TARGET_ARM32_ABI_SOFT AND (PANDA_TARGET_ARM32_ABI_SOFTFP OR PANDA_TARGET_ARM32_ABI_HARD)) OR
       (PANDA_TARGET_ARM32_ABI_SOFTFP AND (PANDA_TARGET_ARM32_ABI_SOFT OR PANDA_TARGET_ARM32_ABI_HARD)) OR
       (PANDA_TARGET_ARM32_ABI_HARD AND (PANDA_TARGET_ARM32_ABI_SOFT OR PANDA_TARGET_ARM32_ABI_SOFTFP)))
        message(FATAL_ERROR "Only one PANDA_TARGET_ARM32_ABI_* should be set.
            PANDA_TARGET_ARM32_ABI_SOFT = ${PANDA_TARGET_ARM32_ABI_SOFT}
            PANDA_TARGET_ARM32_ABI_SOFTFP = ${PANDA_TARGET_ARM32_ABI_SOFTFP}
            PANDA_TARGET_ARM32_ABI_HARD = ${PANDA_TARGET_ARM32_ABI_HARD}")
    elseif(NOT (PANDA_TARGET_ARM32_ABI_SOFT OR PANDA_TARGET_ARM32_ABI_SOFTFP OR PANDA_TARGET_ARM32_ABI_HARD))
        message(FATAL_ERROR "PANDA_TARGET_ARM32_ABI_* is not set")
    endif()
else()
    message(FATAL_ERROR "Processor ${CMAKE_SYSTEM_PROCESSOR} is not supported")
endif()

if(PANDA_TARGET_AMD64 OR PANDA_TARGET_ARM64)
    panda_set_flag(PANDA_TARGET_64)
elseif(PANDA_TARGET_X86 OR PANDA_TARGET_ARM32)
    panda_set_flag(PANDA_TARGET_32)
else()
    message(FATAL_ERROR "Unknown bitness of the target platform")
endif()

if (PANDA_TRACK_INTERNAL_ALLOCATIONS)
    message(STATUS "Track internal allocations")
    add_definitions(-DTRACK_INTERNAL_ALLOCATIONS=${PANDA_TRACK_INTERNAL_ALLOCATIONS})
endif()

if (PANDA_TARGET_ARM64 AND NOT PANDA_CI_TESTING_MODE STREQUAL "Nightly")
    if (PANDA_ENABLE_ADDRESS_SANITIZER OR PANDA_ENABLE_THREAD_SANITIZER)
        panda_set_flag(PANDA_ARM64_TESTS_WITH_SANITIZER)
    endif()
endif()


# Enable global register variables usage only for clang >= 9.0.0 and gcc >= 8.0.0.
# Clang 8.0.0 doesn't support all necessary options -ffixed-<reg>. Gcc 7.5.0 freezes
# when compiling release interpreter.
#
# Also calling conventions of functions that use global register variables are different:
# clang stores and restores registers that are used for global variables in the prolog
# and epilog of such functions and gcc doesn't do it. So it's necessary to inline all
# function that refers to global register variables to interpreter loop.

# For this reason we disable global register variables usage for clang debug builds as
# ALWAYS_INLINE macro expands to nothing in this mode and we cannot guarantee that all
# necessary function will be inlined.
#
if(PANDA_TARGET_ARM64 AND ((CMAKE_CXX_COMPILER_ID STREQUAL "Clang"
                           AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.0.0
                           AND CMAKE_BUILD_TYPE MATCHES Release)
                           OR
                           (CMAKE_CXX_COMPILER_ID STREQUAL "GNU"
                           AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 8.0.0)))
    panda_set_flag(PANDA_ENABLE_GLOBAL_REGISTER_VARIABLES)
endif()

if(CMAKE_BUILD_TYPE MATCHES Debug)
    # Additional debug information about fp in each frame
    add_compile_options(-fno-omit-frame-pointer)
endif()

# for correct work with gdb-9
# See https://discourse.llvm.org/t/gdb-10-1-cant-read-clangs-dwarf-v5/6035
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 14)
    if ("${CMAKE_BUILD_TYPE}" MATCHES "^(RelWithDebInfo|FastVerify|Debug|DebugDetailed)$")
        add_compile_options(-gdwarf-4)
    endif()
endif()

# Enable LTO only for Android aarch64 due to bug for Android armv7:
# https://github.com/android/ndk/issues/721
if (PANDA_TARGET_MOBILE AND PANDA_TARGET_ARM64)
    set(PANDA_ENABLE_LTO true)
    set(PANDA_LLVM_REGALLOC pbqp)
endif()

if (PANDA_PGO_INSTRUMENT OR PANDA_PGO_OPTIMIZE)
    if (NOT PANDA_TARGET_MOBILE OR NOT PANDA_TARGET_ARM64)
        message(FATAL_ERROR "PGO supported only for Android aarch64")
    endif()

    set(PANDA_ENABLE_LTO true)
endif()

# TODO(v.cherkashi): Remove PANDA_TARGET_MOBILE_WITH_MANAGED_LIBS when the managed libs are separated form the Panda
if(PANDA_TARGET_MOBILE_WITH_MANAGED_LIBS)
    add_definitions(-DPANDA_TARGET_MOBILE_WITH_MANAGED_LIBS=1)
    if(PANDA_TARGET_MOBILE)
        panda_set_flag(PANDA_TARGET_MOBILE_WITH_NATIVE_LIBS)
    endif()
else()
    add_definitions(-DPANDA_TARGET_MOBILE_WITH_MANAGED_LIBS=0)
endif()

if(PANDA_TARGET_MOBILE_WITH_NATIVE_LIBS)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-gnu-statement-expression")
    if(MOBILE_NATIVE_LIBS_SOURCE_PATH)
        include_directories(${MOBILE_NATIVE_LIBS_SOURCE_PATH}/libc)
    else()
        message(FATAL_ERROR "MOBILE_NATIVE_LIBS_SOURCE_PATH is not set")
    endif()
endif()

if(NOT DEFINED PANDA_USE_64_BIT_POINTER OR NOT PANDA_USE_64_BIT_POINTER)
    panda_set_flag(PANDA_32_BIT_MANAGED_POINTER)
endif()

if(PANDA_TARGET_LINUX)
    execute_process(COMMAND grep PRETTY_NAME= /etc/os-release
                    OUTPUT_VARIABLE PANDA_TARGET_LINUX_DISTRO
                    OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if(PANDA_TARGET_LINUX_DISTRO MATCHES "Ubuntu")
        panda_set_flag(PANDA_TARGET_LINUX_UBUNTU)
    endif()

    if(PANDA_TARGET_LINUX_DISTRO MATCHES "Ubuntu 18\\.04")
        panda_set_flag(PANDA_TARGET_LINUX_UBUNTU_18_04)
    elseif(PANDA_TARGET_LINUX_DISTRO MATCHES "Ubuntu 20\\.04")
        panda_set_flag(PANDA_TARGET_LINUX_UBUNTU_20_04)
    elseif(PANDA_TARGET_LINUX_DISTRO MATCHES "Ubuntu 22\\.04")
        panda_set_flag(PANDA_TARGET_LINUX_UBUNTU_22_04)
    endif()
    execute_process(COMMAND uname -a
                    OUTPUT_VARIABLE PANDA_TARGET_WSL_DISTRO
                    OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if(PANDA_TARGET_WSL_DISTRO MATCHES "Microsoft")
        panda_set_flag(PANDA_TARGET_WSL)
    endif()
    if(PANDA_TARGET_WSL_DISTRO MATCHES "WSL")
        panda_set_flag(PANDA_TARGET_GENERAL_WSL)
    endif()
endif()

set(PANDA_WITH_RUNTIME    true)
set(PANDA_WITH_COMPILER   true)
set(PANDA_WITH_TOOLCHAIN  true)
if (NOT DEFINED PANDA_WITH_BENCHMARKS)
    set(PANDA_WITH_BENCHMARKS true)
endif()
set(PANDA_DEFAULT_LIB_TYPE "SHARED")

option(PANDA_WITH_TESTS "Enable test targets" true)
option(PANDA_WITH_BYTECODE_OPTIMIZER "Enable bytecode optimizer" true)
option(PANDA_COMPILER_DEBUG_INFO "Support DWARF debug information in Compiler (JIT/AOT/IRtoC)" OFF)
option(PANDA_ENABLE_CCACHE "Enable ccache" true)
if (NOT DEFINED ES2PANDA_PATH)
    file(REAL_PATH ${PANDA_ROOT}/tools/es2panda ES2PANDA_PATH)
endif()

if(PANDA_TARGET_WINDOWS)
    set(PANDA_WITH_BENCHMARKS false)
    set(PANDA_DEFAULT_LIB_TYPE "STATIC")
    # introduced for GetDynamicTimeZoneInformation
    add_definitions(-D_WIN32_WINNT=0x600)
endif()

if(PANDA_TARGET_MACOS)
    set(PANDA_DEFAULT_LIB_TYPE "STATIC")
    #introduced for "std::filesystem::create_directories"
    add_compile_options(-mmacosx-version-min=10.15)
endif()

if(PANDA_TARGET_OHOS)
    set(PANDA_WITH_BENCHMARKS false)
    add_definitions(-D__MUSL__)

    if(PANDA_TARGET_ARM64 OR PANDA_TARGET_ARM32)
        # WORKAROUND:
        #   Disable '-Wunused-command-line-argument' error to avoid compilation error:
        #   clang++: error: argument unused during compilation: '--gcc-toolchain=<path/to/toolchain>/llvm' [-Werror,-Wunused-command-line-argument]
        add_compile_options(-Wno-unused-command-line-argument)

        # -Wno-deprecated-declarations needs for third_parties and ecmascript plugin
        add_compile_options(-Wno-deprecated-declarations)
    endif()
endif()

if(CMAKE_BUILD_TYPE STREQUAL Debug)
    add_definitions(-DPANDA_ENABLE_SLOW_DEBUG)
    add_definitions(-D_GLIBCXX_ASSERTIONS)
endif()

if(CMAKE_BUILD_TYPE STREQUAL FastVerify)
    add_definitions(-DPANDA_FAST_VERIFY)
    add_definitions(-D_GLIBCXX_ASSERTIONS)
endif()

# The define is set for the build which will be delivered to customers.
# Currently this build doesn't contain dependencies to debug libraries
# (like libdwarf.so)
option(PANDA_PRODUCT_BUILD "Build which will be delivered to customers" false)

if(PANDA_PRODUCT_BUILD AND NOT (CMAKE_BUILD_TYPE STREQUAL "Release"))
    message(FATAL_ERROR "PANDA_PRODUCT_BUILD must be run with the Release build type only!")
endif()

if(NOT PANDA_PRODUCT_BUILD AND PANDA_WITH_HIDDEN_SYMBOLS)
    message(FATAL_ERROR "PANDA_WITH_HIDDEN_SYMBOLS must be run with PANDA_PRODUCT_BUILD only!")
endif()

option(PANDA_WITH_HIDDEN_SYMBOLS "Build with -fvisibility=hidden options for some libraries" ${PANDA_PRODUCT_BUILD})

if (("${CMAKE_BUILD_TYPE}" STREQUAL "Debug" OR
     "${CMAKE_BUILD_TYPE}" STREQUAL "DebugDetailed" OR
     "${CMAKE_BUILD_TYPE}" STREQUAL "FastVerify" OR
     "${CMAKE_BUILD_TYPE}" STREQUAL "RelWithDebInfo") AND
     (NOT PANDA_TARGET_WINDOWS) AND
     (NOT PANDA_ENABLE_ADDRESS_SANITIZER) AND
     (NOT PANDA_ENABLE_UNDEFINED_BEHAVIOR_SANITIZER) AND
     (NOT PANDA_ENABLE_THREAD_SANITIZER))
    # Windows do not have elf and dwarf libraries
    # Sanitizers do not work properly with gdb
    set(PANDA_COMPILER_DEBUG_INFO ON)
endif()

# libdwarf-dev lib (version: 20180129-1) in Ubuntu 18.04 has memory leaks
# TODO(asidorov): delete the workaround when the problem is fixed
if (PANDA_ENABLE_ADDRESS_SANITIZER AND PANDA_TARGET_LINUX_UBUNTU_18_04)
    set(PANDA_COMPILER_DEBUG_INFO OFF)
endif()

# TODO: Ensure libdwarf is available when building with OHOS toolchain
if (PANDA_TARGET_OHOS)
    set(PANDA_COMPILER_DEBUG_INFO OFF)
endif()

if (PANDA_PRODUCT_BUILD)
    set(PANDA_COMPILER_DEBUG_INFO OFF)
endif()

if ((NOT DEFINED PANDA_MINIMAL_VIXL) AND PANDA_PRODUCT_BUILD)
    # VIXL aarch64 with Encoder only (with no Decoder or Simulator provided)
    set(PANDA_MINIMAL_VIXL true)
endif()

panda_promote_to_definitions(
    PANDA_PRODUCT_BUILD
    PANDA_WITH_HIDDEN_SYMBOLS
    PANDA_WITH_COMPILER
    PANDA_WITH_BYTECODE_OPTIMIZER
    PANDA_COMPILER_DEBUG_INFO
    PANDA_MINIMAL_VIXL
    INTRUSIVE_TESTING
)

option(PANDA_CROSS_COMPILER          "Enable compiler cross-compilation support" ON)
option(PANDA_COMPILER_TARGET_X86     "Build x86-backend")
option(PANDA_COMPILER_TARGET_X86_64  "Build x86_64-backend" ON)
option(PANDA_COMPILER_TARGET_AARCH32 "Build aarch32-backend" ON)
option(PANDA_COMPILER_TARGET_AARCH64 "Build aarch64-backend" ON)
# User-specified cross-toolchains:
option(PANDA_CROSS_X86_64_TOOLCHAIN_FILE "Absolute path to X86_64 target toolchain" OFF)
option(PANDA_CROSS_AARCH64_TOOLCHAIN_FILE "Absolute path to AARCH64 target toolchain" OFF)
option(PANDA_CROSS_AARCH32_TOOLCHAIN_FILE "Absolute path to AARCH32 target toolchain" OFF)

# true if current target supports JIT/AOT native compilation
# TODO (asidorov, runtime): replace all uses of this option by PANDA_WITH_COMPILER
set(PANDA_COMPILER_ENABLE TRUE)

if (PANDA_TARGET_AMD64)
    if (PANDA_CROSS_COMPILER)
        if (HOST_TOOLS)
            # For host-tools build support only single-target backend (with the same toolchain):
            message(STATUS "set ${PANDA_HOST_TOOLS_TARGET_ARCH}")
            set(PANDA_COMPILER_TARGET_${PANDA_HOST_TOOLS_TARGET_ARCH} ON)
        else()
            set(PANDA_COMPILER_TARGET_X86_64 ON)
            # If `PANDA_CROSS_${arch}_TOOLCHAIN_FILE` wasn't specified, gcc-toolchain is used:
            find_program(GCC_AARCH64_CXX "aarch64-linux-gnu-g++")
            find_program(GCC_ARM_CXX "arm-linux-gnueabi-g++")

            # The option is ON by default; this 'if' allows to check if the target wasn't turned off explicitly:
            if (PANDA_COMPILER_TARGET_AARCH64)
                if (PANDA_CROSS_AARCH64_TOOLCHAIN_FILE)
                    message(STATUS "Specified AARCH64 toolchain: ${PANDA_CROSS_AARCH64_TOOLCHAIN_FILE}")
                elseif (GCC_AARCH64_CXX)
                    message(STATUS "Detected default AARCH64 toolchain")
                else()
                    set(PANDA_COMPILER_TARGET_AARCH64 OFF)
                    message(STATUS "No AARCH64 toolchain found")
                endif()
            endif()

            # The option is ON by default; this 'if' allows to check if the target wasn't turned off explicitly:
            if (PANDA_COMPILER_TARGET_AARCH32)
                if (PANDA_CROSS_AARCH32_TOOLCHAIN_FILE)
                    message(STATUS "Specified AARCH32 toolchain: ${PANDA_CROSS_AARCH32_TOOLCHAIN_FILE}")
                elseif (GCC_ARM_CXX)
                    message(STATUS "Detected default AARCH32 toolchain")
                else()
                    set(PANDA_COMPILER_TARGET_AARCH32 OFF)
                    message(STATUS "No AARCH32 toolchain found")
                endif()
            endif()
            # TODO(dkofanov): cross-values do not support x86
            set(PANDA_COMPILER_TARGET_X86 OFF)
        endif()
    else()
        set(PANDA_COMPILER_TARGET_X86       OFF)
        set(PANDA_COMPILER_TARGET_X86_64    ON)
        set(PANDA_COMPILER_TARGET_AARCH32   OFF)
        set(PANDA_COMPILER_TARGET_AARCH64   OFF)
    endif()
endif()

if (PANDA_TARGET_X86)
    set(PANDA_COMPILER_TARGET_X86       ON)
    set(PANDA_COMPILER_TARGET_X86_64    OFF)
    set(PANDA_COMPILER_TARGET_AARCH32   OFF)
    set(PANDA_COMPILER_TARGET_AARCH64   OFF)
endif()

if (PANDA_TARGET_ARM32)
    if(PANDA_TARGET_ARM32_ABI_SOFT)
        set(PANDA_COMPILER_ENABLE FALSE)
        set(PANDA_COMPILER_TARGET_X86       OFF)
        set(PANDA_COMPILER_TARGET_X86_64    OFF)
        set(PANDA_COMPILER_TARGET_AARCH32   OFF)
        set(PANDA_COMPILER_TARGET_AARCH64   OFF)
    else()
        set(PANDA_COMPILER_TARGET_X86       OFF)
        set(PANDA_COMPILER_TARGET_X86_64    OFF)
        set(PANDA_COMPILER_TARGET_AARCH32   ON)
        set(PANDA_COMPILER_TARGET_AARCH64   OFF)
    endif()
endif()

if (PANDA_TARGET_ARM64)
    set(PANDA_COMPILER_TARGET_X86       OFF)
    set(PANDA_COMPILER_TARGET_X86_64    OFF)
    set(PANDA_COMPILER_TARGET_AARCH32   OFF)
    set(PANDA_COMPILER_TARGET_AARCH64   ON)
endif()

if ((NOT PANDA_TARGET_WINDOWS) AND (NOT PANDA_ENABLE_ADDRESS_SANITIZER) AND (NOT PANDA_ENABLE_THREAD_SANITIZER))
    panda_set_flag(PANDA_USE_CUSTOM_SIGNAL_STACK)
endif()

option(PANDA_LLVM_BACKEND "Enable LLVM backend for Ark compiler" OFF)

if (NOT PANDA_LLVM_BACKEND)
    if (PANDA_LLVM_INTERPRETER)
        message(FATAL_ERROR "PANDA_LLVM_INTERPRETER can't be enabled without PANDA_LLVM_BACKEND")
    endif()

    if (PANDA_LLVM_FASTPATH)
        message(FATAL_ERROR "PANDA_LLVM_FASTPATH can't be enabled without PANDA_LLVM_BACKEND")
    endif()

    if (PANDA_LLVM_AOT)
        message(FATAL_ERROR "PANDA_LLVM_AOT can't be enabled without PANDA_LLVM_BACKEND")
    endif()
endif()

if (NOT (PANDA_TARGET_AMD64 OR PANDA_TARGET_ARM64))
    set(PANDA_LLVM_BACKEND OFF)
endif()

if (PANDA_LLVM_BACKEND)
    # PANDA_LLVM_BACKEND auto-enables LLVM_INTERPRETER, LLVM_AOT and LLVM_FASTPATH unless user
    # disabled some of them explicitly. Example:
    # -DPANDA_LLVM_BACKEND=ON -DPANDA_LLVM_FASTPATH=OFF => FASTPATH is OFF, INTERPRETER and AOT are ON
    if (NOT DEFINED PANDA_LLVM_INTERPRETER)
        set(PANDA_LLVM_INTERPRETER ON)
    endif()
    if (PANDA_TARGET_AMD64 AND NOT CMAKE_CROSSCOMPILING AND NOT HOST_TOOLS)
        # LLVM_FASTPATH is not supported for x86_64 target build
        if (NOT DEFINED PANDA_LLVM_FASTPATH)
            set(PANDA_LLVM_FASTPATH OFF)
        elseif(PANDA_LLVM_FASTPATH)
            message(FATAL_ERROR "PANDA_LLVM_FASTPATH is not supported for x86_64")
        endif()
    else()  # otherwise, it is processed similar to INTERPRETER above and AOT below
        if (NOT DEFINED PANDA_LLVM_FASTPATH)
            set(PANDA_LLVM_FASTPATH ON)
        endif()
    endif()
    if (NOT DEFINED PANDA_LLVM_AOT)
        set(PANDA_LLVM_AOT ON)
    endif()

    # LLVM_IRTOC is an internal flag telling if Irtoc compilation is necessary:
    # al least one of LLVM_FASTPATH or LLVM_INTERPRETER should be on
    if (PANDA_LLVM_FASTPATH OR PANDA_LLVM_INTERPRETER)
        panda_set_flag(PANDA_LLVM_IRTOC)
    endif()

    # PANDA_BUILD_LLVM_BACKEND is an internal flag telling if it is necessary to compile backend
    # in this particular circumstances.
    # LLVM_AOT means we need the backend. LLVM_AOT is always disabled in host_tools part.
    # LLVM_IRTOC means we need the backend unless we are in cross compilation.
    # For cross compilation case Irtoc would be processed in host_tools part.
    # For host_tools BUILD_LLVM_BACKEND is effectively equal to LLVM_IRTOC.
    if (PANDA_LLVM_AOT OR (PANDA_LLVM_IRTOC AND NOT CMAKE_CROSSCOMPILING))
        panda_set_flag(PANDA_BUILD_LLVM_BACKEND)
    endif()
endif()

panda_promote_to_definitions(
    PANDA_COMPILER_TARGET_X86
    PANDA_COMPILER_TARGET_X86_64
    PANDA_COMPILER_TARGET_AARCH32
    PANDA_COMPILER_TARGET_AARCH64
    PANDA_COMPILER_ENABLE
    PANDA_QEMU_BUILD
    PANDA_LLVM_BACKEND
    PANDA_LLVM_FASTPATH
    PANDA_LLVM_INTERPRETER
    PANDA_LLVM_AOT
)

if (PANDA_USE_PREBUILT_TARGETS)
    set(CMAKE_NO_SYSTEM_FROM_IMPORTED   ON)
endif()

if (PANDA_ETS_INTEROP_JS)
    panda_set_flag(PANDA_ETS_INTEROP_JS)
endif()

if (PANDA_WITH_SAFEPOINT_CHECKER)
    panda_set_flag(SAFEPOINT_TIME_CHECKER_ENABLED)
endif()

message(STATUS "CMAKE_BUILD_TYPE                       = ${CMAKE_BUILD_TYPE}")
message(STATUS "PANDA_TARGET_MOBILE_WITH_MANAGED_LIBS  = ${PANDA_TARGET_MOBILE_WITH_MANAGED_LIBS}")
message(STATUS "PANDA_TARGET_UNIX                      = ${PANDA_TARGET_UNIX}")
message(STATUS "PANDA_TARGET_LINUX                     = ${PANDA_TARGET_LINUX}")
message(STATUS "PANDA_TARGET_MOBILE                    = ${PANDA_TARGET_MOBILE}")
message(STATUS "PANDA_USE_FUTEX                        = ${PANDA_USE_FUTEX}")
message(STATUS "PANDA_TARGET_WINDOWS                   = ${PANDA_TARGET_WINDOWS}")
message(STATUS "PANDA_TARGET_OHOS                      = ${PANDA_TARGET_OHOS}")
message(STATUS "PANDA_TARGET_MACOS                     = ${PANDA_TARGET_MACOS}")
message(STATUS "PANDA_CROSS_COMPILER                   = ${PANDA_CROSS_COMPILER}")
message(STATUS "PANDA_CROSS_AMD64_X86                  = ${PANDA_CROSS_AMD64_X86}")
message(STATUS "PANDA_TARGET_AMD64                     = ${PANDA_TARGET_AMD64}")
message(STATUS "PANDA_TARGET_X86                       = ${PANDA_TARGET_X86}")
message(STATUS "PANDA_TARGET_ARM64                     = ${PANDA_TARGET_ARM64}")
message(STATUS "PANDA_TARGET_ARM32                     = ${PANDA_TARGET_ARM32}")
if(PANDA_TARGET_ARM32)
message(STATUS "PANDA_TARGET_ARM32_ABI_SOFT            = ${PANDA_TARGET_ARM32_ABI_SOFT}")
message(STATUS "PANDA_TARGET_ARM32_ABI_SOFTFP          = ${PANDA_TARGET_ARM32_ABI_SOFTFP}")
message(STATUS "PANDA_TARGET_ARM32_ABI_HARD            = ${PANDA_TARGET_ARM32_ABI_HARD}")
endif()
message(STATUS "PANDA_TARGET_64                        = ${PANDA_TARGET_64}")
message(STATUS "PANDA_TARGET_32                        = ${PANDA_TARGET_32}")
message(STATUS "PANDA_TARGET_CPU_FEATURES              = ${PANDA_TARGET_CPU_FEATURES}")
message(STATUS "PANDA_ENABLE_GLOBAL_REGISTER_VARIABLES = ${PANDA_ENABLE_GLOBAL_REGISTER_VARIABLES}")
message(STATUS "PANDA_ENABLE_LTO                       = ${PANDA_ENABLE_LTO}")
if(PANDA_TARGET_MOBILE)
message(STATUS "PANDA_LLVM_REGALLOC                    = ${PANDA_LLVM_REGALLOC}")
endif()
if(PANDA_TARGET_MOBILE_WITH_NATIVE_LIBS)
message(STATUS "PANDA_TARGET_MOBILE_WITH_NATIVE_LIBS   = ${PANDA_TARGET_MOBILE_WITH_NATIVE_LIBS}")
endif()
message(STATUS "PANDA_WITH_RUNTIME                     = ${PANDA_WITH_RUNTIME}")
message(STATUS "PANDA_WITH_COMPILER                    = ${PANDA_WITH_COMPILER}")
message(STATUS "PANDA_COMPILER_ENABLE                  = ${PANDA_COMPILER_ENABLE}")
message(STATUS "PANDA_WITH_TOOLCHAIN                   = ${PANDA_WITH_TOOLCHAIN}")
message(STATUS "PANDA_WITH_TESTS                       = ${PANDA_WITH_TESTS}")
message(STATUS "PANDA_WITH_BENCHMARKS                  = ${PANDA_WITH_BENCHMARKS}")
message(STATUS "PANDA_WITH_BYTECODE_OPTIMIZER          = ${PANDA_WITH_BYTECODE_OPTIMIZER}")
message(STATUS "PANDA_PGO_INSTRUMENT                   = ${PANDA_PGO_INSTRUMENT}")
message(STATUS "PANDA_PGO_OPTIMIZE                     = ${PANDA_PGO_OPTIMIZE}")
message(STATUS "PANDA_PRODUCT_BUILD                    = ${PANDA_PRODUCT_BUILD}")
message(STATUS "PANDA_QEMU_BUILD                       = ${PANDA_QEMU_BUILD}")
message(STATUS "PANDA_LLVM_BACKEND                     = ${PANDA_LLVM_BACKEND}")
message(STATUS "PANDA_LLVM_INTERPRETER                 = ${PANDA_LLVM_INTERPRETER}")
message(STATUS "PANDA_LLVM_FASTPATH                    = ${PANDA_LLVM_FASTPATH}")
message(STATUS "PANDA_LLVM_AOT                         = ${PANDA_LLVM_AOT}")
message(STATUS "PANDA_ENABLE_CCACHE                    = ${PANDA_ENABLE_CCACHE}")
message(STATUS "PANDA_USE_CUSTOM_SIGNAL_STACK          = ${PANDA_USE_CUSTOM_SIGNAL_STACK}")
message(STATUS "PANDA_USE_PREBUILT_TARGETS             = ${PANDA_USE_PREBUILT_TARGETS}")
message(STATUS "CMAKE_NO_SYSTEM_FROM_IMPORTED          = ${CMAKE_NO_SYSTEM_FROM_IMPORTED}")
message(STATUS "ES2PANDA_PATH                          = ${ES2PANDA_PATH}")
message(STATUS "PANDA_32_BIT_MANAGED_POINTER           = ${PANDA_32_BIT_MANAGED_POINTER}")
message(STATUS "PANDA_ENABLE_THREAD_SANITIZER          = ${PANDA_ENABLE_THREAD_SANITIZER}")
