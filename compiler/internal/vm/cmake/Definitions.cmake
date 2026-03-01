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

if(CMAKE_SYSTEM_NAME STREQUAL Linux)
    panda_set_flag(PANDA_TARGET_LINUX)
    panda_set_flag(PANDA_TARGET_UNIX)
    if (NOT PANDA_ENABLE_ADDRESS_SANITIZER)
        panda_set_flag(PANDA_USE_FUTEX)
    endif()
elseif(CMAKE_SYSTEM_NAME STREQUAL OHOS)
    panda_set_flag(PANDA_TARGET_OHOS)
    panda_set_flag(PANDA_TARGET_UNIX)
elseif(CMAKE_SYSTEM_NAME STREQUAL Windows)
    panda_set_flag(PANDA_TARGET_WINDOWS)
elseif(CMAKE_SYSTEM_NAME STREQUAL Darwin)
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

if (PANDA_TARGET_MOBILE AND PANDA_TARGET_ARM64)
    set(PANDA_ENABLE_LTO true)
    set(PANDA_LLVM_REGALLOC pbqp)
endif()

if (PANDA_PGO_INSTRUMENT OR PANDA_PGO_OPTIMIZE)
    if (NOT PANDA_TARGET_MOBILE OR NOT PANDA_TARGET_ARM64)
        message(FATAL_ERROR "PGO supported only for aarch64")
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

if(PANDA_TARGET_64)
    panda_set_flag(PANDA_USE_32_BIT_POINTER)
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
option(PANDA_WITH_JAVA "Enable java plugin" ON)
option(PANDA_WITH_ECMASCRIPT "Enable ecmascript plugin" ON)
option(PANDA_WITH_ACCORD "Enable accord plugin" ON)
option(PANDA_WITH_CODIRA "Enable cangjie plugin" ON)
option(PANDA_COMPILER_CFI "Support CFI information")
option(PANDA_ENABLE_RELAYOUT_PROFILE "Enable relayout profile" OFF)

if(PANDA_TARGET_WINDOWS)
    set(PANDA_WITH_BENCHMARKS false)
    set(PANDA_DEFAULT_LIB_TYPE "STATIC")
    # Need to fix windows build for ecmascript.
    set(PANDA_WITH_ECMASCRIPT false)
endif()

if(PANDA_TARGET_MACOS)
    set(PANDA_DEFAULT_LIB_TYPE "STATIC")
    #introduced for "std::filesystem::create_directories"
    add_compile_options(-mmacosx-version-min=10.15)
endif()

if(PANDA_TARGET_OHOS)
    set(PANDA_WITH_BENCHMARKS false)
endif()

if(CMAKE_BUILD_TYPE STREQUAL Debug)
    add_definitions(-DPANDA_ENABLE_SLOW_DEBUG)
endif()

if(CMAKE_BUILD_TYPE STREQUAL FastVerify)
    add_definitions(-DPANDA_FAST_VERIFY)
endif()

# The define is set for the build which will be delivered to customers.
# Currently this build doesn't contain dependencies to debug libraries
# (like libdwarf.so)
option(PANDA_PRODUCT_BUILD "Build which will be delivered to customers" false)

# TODO: merge with PANDA_PRODUCT_BUILD
# PANDA_RELEASE_BUILD should set all relevant  flags for benchmarking
option(PANDA_RELEASE_BUILD "Build for benchmarking. Must be run with CMAKE_BUILD_TYPE=Release" false)

if(PANDA_RELEASE_BUILD AND NOT (CMAKE_BUILD_TYPE STREQUAL "Release"))
    message(FATAL_ERROR "PANDA_RELEASE_BUILD must be run with the Release build type only!")
endif()

if (PANDA_RELEASE_BUILD)
    set(PANDA_WITH_JAVA ON)
    set(PANDA_WITH_ECMASCRIPT ON)
    set(PANDA_WITH_ACCORD OFF)
    set(PANDA_WITH_CODIRA OFF)
endif()

if (PANDA_PRODUCT_BUILD OR PANDA_RELEASE_BUILD)
    # VIXL aarch64 with Encoder only (with no Decoder or Simulator provided)
    set(PANDA_MINIMAL_VIXL true)
endif()

panda_promote_to_definitions(
    PANDA_PRODUCT_BUILD
    PANDA_WITH_COMPILER
    PANDA_WITH_BYTECODE_OPTIMIZER
    PANDA_MINIMAL_VIXL
)

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
    set(PANDA_COMPILER_CFI ON)
endif()

# libdwarf-dev lib (version: 20180129-1) in Ubuntu 18.04 has memory leaks
# TODO(asidorov): delete the workaround when the problem is fixed
if (PANDA_ENABLE_ADDRESS_SANITIZER AND PANDA_TARGET_LINUX_UBUNTU_18_04)
    set(PANDA_COMPILER_CFI OFF)
endif()

# TODO: Ensure libdwarf is available when building with OHOS toolchain
if (PANDA_TARGET_OHOS)
    set(PANDA_COMPILER_CFI OFF)
endif()

if (PANDA_PRODUCT_BUILD)
    set(PANDA_COMPILER_CFI OFF)
endif()

if (PANDA_COMPILER_CFI)
    add_definitions(-DPANDA_COMPILER_CFI)
endif()

option(PANDA_CROSS_COMPILER          "Enable compiler cross-compilation support" ON)
option(PANDA_COMPILER_TARGET_X86     "Build x86-backend")
option(PANDA_COMPILER_TARGET_X86_64  "Build x86_64-backend")
option(PANDA_COMPILER_TARGET_AARCH32 "Build aarch32-backend")
option(PANDA_COMPILER_TARGET_AARCH64 "Build aarch64-backend")
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

            if (PANDA_CROSS_AARCH64_TOOLCHAIN_FILE)
                set(PANDA_COMPILER_TARGET_AARCH64 ON)
                message(STATUS "Specified AARCH64 toolchain: ${PANDA_CROSS_AARCH64_TOOLCHAIN_FILE}")
            elseif (GCC_AARCH64_CXX)
                set(PANDA_COMPILER_TARGET_AARCH64 ON)
                message(STATUS "Detected default AARCH64 toolchain")
            else()
                message(STATUS "No AARCH64 toolchain found")
            endif()

            if (PANDA_CROSS_AARCH32_TOOLCHAIN_FILE)
                set(PANDA_COMPILER_TARGET_AARCH32 ON)
                message(STATUS "Specified AARCH32 toolchain: ${PANDA_CROSS_AARCH32_TOOLCHAIN_FILE}")
            elseif (GCC_ARM_CXX)
                set(PANDA_COMPILER_TARGET_AARCH32 ON)
                message(STATUS "Detected default AARCH32 toolchain")
            else()
                message(STATUS "No AARCH32 toolchain found")
            endif()
            # TODO(dkofanov): cross-values do not support x86
            set(PANDA_COMPILER_TARGET_X86 OFF)
        endif()
    else()
        set(PANDA_COMPILER_TARGET_X86_64 ON)
    endif()
endif()

if (PANDA_TARGET_X86)
    set(PANDA_COMPILER_TARGET_X86 ON)
endif()

if (PANDA_TARGET_ARM32)
    if(PANDA_TARGET_ARM32_ABI_SOFT)
        set(PANDA_COMPILER_ENABLE FALSE)
    else()
        set(PANDA_COMPILER_TARGET_AARCH32 ON)
    endif()
endif()

if (PANDA_TARGET_ARM64)
    set(PANDA_COMPILER_TARGET_AARCH64 ON)
endif()

panda_promote_to_definitions(
    PANDA_COMPILER_TARGET_X86
    PANDA_COMPILER_TARGET_X86_64
    PANDA_COMPILER_TARGET_AARCH32
    PANDA_COMPILER_TARGET_AARCH64
    PANDA_COMPILER_ENABLE
)

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
message(STATUS "PANDA_WITH_JAVA                        = ${PANDA_WITH_JAVA}")
message(STATUS "PANDA_WITH_ECMASCRIPT                  = ${PANDA_WITH_ECMASCRIPT}")
message(STATUS "PANDA_WITH_ACCORD                      = ${PANDA_WITH_ACCORD}")
message(STATUS "PANDA_WITH_CODIRA                     = ${PANDA_WITH_CODIRA}")
message(STATUS "PANDA_ENABLE_RELAYOUT_PROFILE          = ${PANDA_ENABLE_RELAYOUT_PROFILE}")
