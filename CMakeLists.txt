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

cmake_minimum_required(VERSION 3.16.5)
project(codira)

# Set default version if not provided externally
if(NOT DEFINED CODE_SDK_VERSION)
    set(CODE_SDK_VERSION "26.0.0" CACHE STRING "Codira SDK Version")
endif()

# Add path for custom CMake modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CODENATIVE_BACKEND "codenative")

# ---------------------------------------------------------------------------
# Repo root vs compiler source directory
#
# This CMakeLists.txt lives at  <repo_root>/compiler/CMakeLists.txt
# so CMAKE_SOURCE_DIR == <repo_root>/compiler/
#
# Shared directories (vm/, stdx/, runtime/, build output) live one level up
# at the repo root.  Pass -DCODIRA_REPO_ROOT=<path> to override, or let the
# default walk up automatically.
# ---------------------------------------------------------------------------
if(NOT DEFINED CODIRA_REPO_ROOT)
    get_filename_component(CODIRA_REPO_ROOT "${CMAKE_SOURCE_DIR}/.." ABSOLUTE)
endif()
message(STATUS "Compiler source dir : ${CMAKE_SOURCE_DIR}")
message(STATUS "Repo root           : ${CODIRA_REPO_ROOT}")

# ---------------------------------------------------------------------------
# VM directory - all third-party / vendored dependencies live under
#   <repo_root>/vm/
#
#   vm/core        - LLVM project (compiler backend)
#   vm/boundscheck - bounds-checking function library
#   vm/flatbuffers - FlatBuffers serialisation library
#   vm/libxml2     - libxml2
#   vm/gtest       - GoogleTest (only when CODIRA_BUILD_TESTS=ON)
#   vm/mingw       - MinGW cross-compilation dependencies (Windows targets)
#
# Sister directories at the repo root (informational; not managed here):
#   stdx/          - standard library extensions
#   runtime/       - codira runtime
#   compiler/      - this CMakeLists.txt + compiler sources
#
# Override vm root with  -DCODIRA_VM_DIR=/absolute/path  if needed.
# ---------------------------------------------------------------------------
if(NOT DEFINED CODIRA_VM_DIR)
    set(CODIRA_VM_DIR "${CODIRA_REPO_ROOT}/vm")
endif()

set(CODIRA_CODENATIVE_SOURCE_DIR "${CODIRA_VM_DIR}/core"
    CACHE PATH "LLVM project source (vm/core)")
set(CODIRA_BOUNDSCHECK_DIR       "${CODIRA_VM_DIR}/boundscheck")
set(CODIRA_FLATBUFFERS_DIR       "${CODIRA_VM_DIR}/flatbuffers")
set(CODIRA_XML2_SOURCE_DIR       "${CODIRA_VM_DIR}/libxml2"
    CACHE PATH "libxml2 source (vm/libxml2)")
set(CODIRA_GTEST_DIR             "${CODIRA_VM_DIR}/gtest")

message(STATUS "VM directory          : ${CODIRA_VM_DIR}")
message(STATUS "  core  (LLVM)        : ${CODIRA_CODENATIVE_SOURCE_DIR}")
message(STATUS "  boundscheck         : ${CODIRA_BOUNDSCHECK_DIR}")
message(STATUS "  flatbuffers         : ${CODIRA_FLATBUFFERS_DIR}")
message(STATUS "  libxml2             : ${CODIRA_XML2_SOURCE_DIR}")
message(STATUS "  gtest               : ${CODIRA_GTEST_DIR}")

# ---------------------------------------------------------------------------
# Auto-fetch any vm/* dependency that is missing from disk
# ---------------------------------------------------------------------------
include(FetchContent)

# LLVM (vm/core)
if(NOT IS_DIRECTORY "${CODIRA_CODENATIVE_SOURCE_DIR}")
    message(STATUS "vm/core not found - fetching LLVM project...")
    FetchContent_Declare(llvm_core
        GIT_REPOSITORY https://github.com/llvm/llvm-project.git
        GIT_TAG        llvmorg-17.0.6
        SOURCE_DIR     "${CODIRA_CODENATIVE_SOURCE_DIR}"
        GIT_SHALLOW    TRUE
    )
    FetchContent_Populate(llvm_core)
endif()

# boundscheck (vm/boundscheck)
if(NOT IS_DIRECTORY "${CODIRA_BOUNDSCHECK_DIR}")
    message(STATUS "vm/boundscheck not found - fetching bounds-checking-function...")
    FetchContent_Declare(boundscheck_fetch
        GIT_REPOSITORY https://github.com/NeXTHub-Corporation/bounds-checking-function.git
        GIT_TAG        main
        SOURCE_DIR     "${CODIRA_BOUNDSCHECK_DIR}"
        GIT_SHALLOW    TRUE
    )
    FetchContent_Populate(boundscheck_fetch)
endif()
# Always (re-)write our integration CMakeLists so it stays current.
configure_file(
    "${CMAKE_SOURCE_DIR}/cmake/BoundsCheck.cmake"
    "${CODIRA_BOUNDSCHECK_DIR}/CMakeLists.txt"
    COPYONLY
)

# FlatBuffers (vm/flatbuffers)
if(CODIRA_DOWNLOAD_FLATBUFFERS AND NOT IS_DIRECTORY "${CODIRA_FLATBUFFERS_DIR}")
    message(STATUS "vm/flatbuffers not found - fetching FlatBuffers...")
    FetchContent_Declare(flatbuffers_fetch
        GIT_REPOSITORY https://github.com/google/flatbuffers.git
        GIT_TAG        v23.5.26
        SOURCE_DIR     "${CODIRA_FLATBUFFERS_DIR}"
        GIT_SHALLOW    TRUE
    )
    FetchContent_Populate(flatbuffers_fetch)
endif()

# libxml2 (vm/libxml2)
if(NOT IS_DIRECTORY "${CODIRA_XML2_SOURCE_DIR}")
    message(STATUS "vm/libxml2 not found - fetching libxml2...")
    FetchContent_Declare(libxml2_fetch
        GIT_REPOSITORY https://github.com/GNOME/libxml2.git
        GIT_TAG        v2.12.5
        SOURCE_DIR     "${CODIRA_XML2_SOURCE_DIR}"
        GIT_SHALLOW    TRUE
    )
    FetchContent_Populate(libxml2_fetch)
endif()

# GoogleTest (vm/gtest) - only when tests are enabled
if(CODIRA_BUILD_TESTS AND NOT IS_DIRECTORY "${CODIRA_GTEST_DIR}")
    message(STATUS "vm/gtest not found - fetching GoogleTest...")
    FetchContent_Declare(gtest_fetch
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        v1.14.0
        SOURCE_DIR     "${CODIRA_GTEST_DIR}"
        GIT_SHALLOW    TRUE
    )
    FetchContent_Populate(gtest_fetch)
endif()

# ---------------------------------------------------------------------------
# Build type
# ---------------------------------------------------------------------------
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "No build type selected, default to Debug")
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type (default Debug)" FORCE)
endif()

string(TOUPPER "${CMAKE_BUILD_TYPE}" uppercase_CMAKE_BUILD_TYPE)

if(CMAKE_BUILD_TYPE AND NOT
    ("${uppercase_CMAKE_BUILD_TYPE}" MATCHES
     "^(DEBUG|RELEASE|RELWITHDEBINFO|MINSIZEREL|MINSIZERELWITHDEBINFO)$"))
    message(FATAL_ERROR "Invalid value for CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
endif()

if(CMAKE_BUILD_TYPE
   AND ("${uppercase_CMAKE_BUILD_TYPE}" MATCHES "^(RELEASE)$")
   AND NOT CMAKE_ENABLE_ASSERT)
    add_compile_definitions(CODIRA_CHIR_WFC_OFF)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_SKIP_INSTALL_ALL_DEPENDENCY TRUE)

# ---------------------------------------------------------------------------
# Options
# ---------------------------------------------------------------------------
option(CMAKE_ENABLE_ASSERT               "Enable assert and checking"                        OFF)
option(CODIRA_CODEGEN_CODENATIVE_BACKEND  "Build for the CODENATIVE backend"                  ON)
option(CODIRA_BUILD_TESTS                "Build codira tests"                                 ON)
option(CODIRA_BUILD_CODEC                "Build codira compiler"                              ON)
option(CODIRA_SKIP_BUILD_CLANG_RT        "Do not build clang_rt (cross-compile only)"         OFF)
option(CODIRA_BUILD_STD_SUPPORT          "Build codira dependency of libast"                  ON)
option(CODIRA_ENABLE_CCACHE              "Build codira with ccache"                           OFF)
option(CODIRA_DOWNLOAD_FLATBUFFERS       "Download FlatBuffers into vm/flatbuffers"           ON)
option(COMPILER_EXPLORER_RACE_FIX        "Enable patched behaviour specific to CE"            OFF)
option(CODIRA_DISABLE_STACK_GROW_FEATURE  "Disable stack grow feature for codenative BE"      OFF)
option(CODIRA_ENABLE_COMPILER_TSAN
    "Enable tsan for compiler and tools (relwithdebinfo or debug, Linux only)"               OFF)
option(CODIRA_GENERATE_UNICODE_TABLE
    "Regenerate unicode data tables"                                                         OFF)
option(CODIRA_WRITE_PROFILE
    "`--profile-compile-time` and `--profile-compile-memory` write profiling output"        OFF)
option(CODIRA_ENABLE_ASAN_COV
    "Build with asan and sanitize-coverage (codec_fuzz / lsp_test)"                         OFF)
option(CODIRA_VISIBLE_OPTIONS_ONLY
    "Only expose options visible to end users"                                               ON)

set(CODIRA_LLVM_BUILD_TYPE   Default CACHE STRING "Build type for LLVM")
set(CODIRA_CODEDB_BUILD_TYPE Default CACHE STRING "Build type for codedb")

message(STATUS "Build type for the current project : ${CMAKE_BUILD_TYPE}")
if(CODIRA_LLVM_BUILD_TYPE MATCHES "^Default$")
    set(CODIRA_LLVM_BUILD_TYPE Release)
endif()
message(STATUS "Build type for LLVM                : ${CODIRA_LLVM_BUILD_TYPE}")
if(CODIRA_CODEDB_BUILD_TYPE MATCHES "^Default$")
    set(CODIRA_CODEDB_BUILD_TYPE ${CMAKE_BUILD_TYPE})
endif()
message(STATUS "Build type for codedb (excl. LLVM) : ${CODIRA_CODEDB_BUILD_TYPE}")

if(CODIRA_BUILD_CODEDB
   AND CODIRA_CODEDB_BUILD_TYPE MATCHES "^Debug$"
   AND NOT CODIRA_LLVM_BUILD_TYPE MATCHES "^Debug$")
    message(WARNING
        "Build type of codedb differs from LLVM; some parts may not be debuggable.")
endif()

# ---------------------------------------------------------------------------
# Compiler TSAN guard
# ---------------------------------------------------------------------------
if(CODIRA_ENABLE_COMPILER_TSAN)
    if(NOT CODIRA_BUILD_CODEC)
        message(FATAL_ERROR
            "CODIRA_ENABLE_COMPILER_TSAN is only available while building CODEC")
    endif()
    if(MINGW)
        message(FATAL_ERROR
            "CODIRA_ENABLE_COMPILER_TSAN does not support Windows targets currently")
    endif()
endif()

if(CODIRA_ENABLE_SANITIZE_OPTION)
    add_compile_definitions(CODIRA_ENABLE_SANITIZE_OPTION)
endif()

# ---------------------------------------------------------------------------
# Sanitizer support
# ---------------------------------------------------------------------------
set(CODIRA_ASAN_SUPPORT              OFF)
set(CODIRA_TSAN_SUPPORT              OFF)
set(CODIRA_HWASAN_SUPPORT            OFF)
set(CODIRA_SANITIZER_SUPPORT_ENABLED OFF)
set(CODIRA_SANITIZER_SUPPORT "" CACHE STRING
    "Enable codira sanitizer support (asan | tsan | hwasan)")

if("${CODIRA_SANITIZER_SUPPORT}" STREQUAL "asan")
    set(CODIRA_ASAN_SUPPORT ON)
    set(CODIRA_SANITIZER_SUPPORT_ENABLED ON)
elseif("${CODIRA_SANITIZER_SUPPORT}" STREQUAL "tsan")
    set(CODIRA_TSAN_SUPPORT ON)
    set(CODIRA_SANITIZER_SUPPORT_ENABLED ON)
elseif("${CODIRA_SANITIZER_SUPPORT}" STREQUAL "hwasan")
    set(CODIRA_HWASAN_SUPPORT ON)
    set(CODIRA_SANITIZER_SUPPORT_ENABLED ON)
endif()

if(CODIRA_SANITIZER_SUPPORT_ENABLED)
    if(NOT CODIRA_CODEGEN_CODENATIVE_BACKEND)
        message(FATAL_ERROR
            "Codira sanitizer support requires the codenative backend")
    endif()
    if(CODIRA_ENABLE_ASAN OR CODIRA_ENABLE_ASAN_COV)
        message(FATAL_ERROR
            "Codira sanitizer support is not compatible with an asan build")
    endif()
    if(CODIRA_BUILD_CODEC)
        message(FATAL_ERROR
            "Codira sanitizer support is not compatible with building codec")
    endif()
    if(NOT CODIRA_BUILD_STD_SUPPORT)
        message(FATAL_ERROR
            "Please enable CODIRA_BUILD_STD_SUPPORT for sanitizer support builds")
    endif()
endif()

if(CODIRA_ENABLE_HWASAN)
    add_compile_options(-mllvm -hwasan-globals=0)
endif()

if(CODIRA_ASAN_SUPPORT)
    if(WIN32)
        message(FATAL_ERROR "Codira asan support is only available on Linux")
    endif()
    # Use add_compile_options so asan instrumentation is applied without
    # implicitly linking the asan runtime via add_link_options (GCC caveat).
    add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
    set(SANITIZER_SUBPATH /asan)
elseif(CODIRA_TSAN_SUPPORT)
    if(CMAKE_CROSSCOMPILING)
        message(FATAL_ERROR
            "Codira tsan support is only available on native Linux/Windows builds")
    endif()
    if(CODIRA_SKIP_BUILD_CLANG_RT)
        message(FATAL_ERROR "tsan support requires a locally built compiler-rt")
    endif()
    set(SANITIZER_SUBPATH /tsan)
elseif(CODIRA_HWASAN_SUPPORT)
    if(WIN32)
        message(FATAL_ERROR "Codira hwasan support is only available on Linux")
    endif()
    add_compile_options(-fsanitize=hwaddress -fno-omit-frame-pointer)
    set(SANITIZER_SUBPATH /hwasan)
endif()

set(CODIRA_CODE_SANCOV_USE_8BIT          OFF)
set(CODIRA_BUILD_STDLIB_WITH_CODE_SANCOV OFF)

if(CODIRA_BUILD_STDLIB_WITH_CODE_SANCOV)
    if(CMAKE_CROSSCOMPILING OR WIN32)
        message(FATAL_ERROR
            "Sanitizer Coverage support is only available on native Linux builds")
    endif()
    if(NOT CODIRA_CODEGEN_CODENATIVE_BACKEND)
        message(FATAL_ERROR
            "Sanitizer Coverage support requires the codenative backend")
    endif()
endif()

# ---------------------------------------------------------------------------
# CCache
# ---------------------------------------------------------------------------
if(CODIRA_ENABLE_CCACHE
   OR CMAKE_BUILD_TYPE MATCHES Debug
   OR CMAKE_BUILD_TYPE MATCHES RelWithDebInfo)
    include(cmake/optional/CodiraCCache.cmake)
endif()

# ---------------------------------------------------------------------------
# Compile definitions
# ---------------------------------------------------------------------------
if(CODIRA_CODEGEN_CODENATIVE_BACKEND)
    add_compile_definitions(CODIRA_CODEGEN_CODENATIVE_BACKEND)
endif()

if(CODIRA_WRITE_PROFILE)
    # Enables --profile-compile-time and --profile-compile-memory options.
    add_compile_definitions(CODIRA_WRITE_PROFILE)
endif()

if(CODIRA_BUILD_TESTS)
    add_compile_definitions(CODIRA_BUILD_TESTS)
endif()

if(CODE_SDK_VERSION)
    add_definitions(-DCODE_SDK_VERSION="${CODE_SDK_VERSION}")
endif()

if(CMAKE_ENABLE_ASSERT)
    add_compile_definitions(CMAKE_ENABLE_ASSERT)
endif()

# ---------------------------------------------------------------------------
# Target triple
# ---------------------------------------------------------------------------
if(NOT TRIPLE)
    string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}-${CMAKE_SYSTEM_NAME}-gnu" TRIPLE)
endif()

if(MINGW)
    set(TRIPLE x86_64-w64-mingw32)
endif()

if(NOT TARGET_TRIPLE_DIRECTORY_PREFIX)
    if(IOS AND IOS_PLATFORM MATCHES "SIMULATOR")
        set(EXTRA_OS_SUFFIX _simulator)
    endif()
    string(REPLACE "-" "_"
        TARGET_TRIPLE_DIRECTORY_PREFIX
        "${CMAKE_SYSTEM_NAME}${EXTRA_OS_SUFFIX}_${CMAKE_SYSTEM_PROCESSOR}")
    string(TOLOWER "${TARGET_TRIPLE_DIRECTORY_PREFIX}"
        TARGET_TRIPLE_DIRECTORY_PREFIX)
endif()

if(CMAKE_CROSSCOMPILING)
    message(STATUS
        "CROSS COMPILING from "
        "${CMAKE_HOST_SYSTEM_PROCESSOR}-${CMAKE_HOST_SYSTEM_NAME} to ${TRIPLE}")
endif()

# Always build clang_rt for native (non-cross) builds
if(NOT CMAKE_CROSSCOMPILING)
    set(CODIRA_SKIP_BUILD_CLANG_RT OFF)
endif()

include(SetupAr)
message(STATUS "Building with target=${TRIPLE}")

# ---------------------------------------------------------------------------
# Platform specifics
# ---------------------------------------------------------------------------
if(MACOS)
    include(ReadDarwinSDKInfo)
    set(CMAKE_OSX_DEPLOYMENT_TARGET 12.0.0)
    set(CODIRA_DISABLE_STACK_GROW_FEATURE ON)
endif()

if(CODIRA_INCLUDE)
    foreach(include_path ${CODIRA_INCLUDE})
        include_directories(${include_path})
    endforeach()
endif()

if(NOT (CMAKE_BUILD_TYPE MATCHES Debug))
    add_definitions(-DNDEBUG)
endif()

if(CMAKE_BUILD_TYPE MATCHES Release)
    add_definitions(-DRELEASE)
endif()

# ---------------------------------------------------------------------------
# GCC toolchain passthrough (clang only)
# BUILD_GCC_TOOLCHAIN applies to codec, stdlib, and BE simultaneously.
# ---------------------------------------------------------------------------
if(BUILD_GCC_TOOLCHAIN
   AND (CMAKE_C_COMPILER_ID   STREQUAL "Clang"
        OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang"))
    message(STATUS
        "Passing --gcc-toolchain=${BUILD_GCC_TOOLCHAIN} to clang")
    add_compile_options(--gcc-toolchain=${BUILD_GCC_TOOLCHAIN})
    add_link_options(--gcc-toolchain=${BUILD_GCC_TOOLCHAIN})
endif()

foreach(libpath ${CODIRA_TARGET_LIB})
    add_link_options("-L${libpath}")
endforeach()

add_compile_options(${CXX_SYSTEM_INCLUDE_CONFIGURATION_FLAG})

if(CODIRA_LINK_JOB_POOL AND CMAKE_GENERATOR MATCHES "Ninja")
    set_property(GLOBAL APPEND PROPERTY JOB_POOLS
        link_job_pool=${CODIRA_LINK_JOB_POOL})
    set(CMAKE_JOB_POOL_LINK link_job_pool)
endif()

if(CODIRA_CODEGEN_CODENATIVE_BACKEND)
    include_directories(${CMAKE_SOURCE_DIR}/include)
else()
    include_directories(${CMAKE_SOURCE_DIR}/includeVM)
    include_directories(AFTER ${CMAKE_SOURCE_DIR}/include)
endif()

# ---------------------------------------------------------------------------
# Output directories
# ---------------------------------------------------------------------------
if(WIN32)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG   ${CMAKE_BINARY_DIR}/lib)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG   ${CMAKE_BINARY_DIR}/lib)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${CMAKE_BINARY_DIR}/bin)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/lib)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/lib)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin)
endif()
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if(CODIRA_VERSION)
    add_compile_definitions(VERSION_TAIL="${CODIRA_VERSION}")
endif()

string(TOLOWER "${CMAKE_SYSTEM_NAME}" TARGET_OS)
add_compile_definitions(__${TARGET_OS}__)

# ---------------------------------------------------------------------------
# Demangler (external project)
# ---------------------------------------------------------------------------
if(CODIRA_CODEGEN_CODENATIVE_BACKEND AND CODIRA_BUILD_CODEC)
    include(ExternalProject)
    set(CODIRA_DEMANGLER_CMAKE_ARGS
        -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
        -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
        -DCMAKE_SYSROOT=${CMAKE_SYSROOT}
        -DBUILD_GCC_TOOLCHAIN=${BUILD_GCC_TOOLCHAIN}
        -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/codira-demangler
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE})

    set(CODIRA_DEMANGLER_CONFIGURE_COMMAND
        ${CMAKE_COMMAND} <SOURCE_DIR> -G${CMAKE_GENERATOR}
        ${CODIRA_DEMANGLER_CMAKE_ARGS})

    ExternalProject_Add(codira-demangler
        SOURCE_DIR        ${CMAKE_SOURCE_DIR}/demangler
        CONFIGURE_COMMAND ${CODIRA_DEMANGLER_CONFIGURE_COMMAND}
        BUILD_COMMAND     ${CMAKE_COMMAND} --build .
        INSTALL_COMMAND   ${CMAKE_COMMAND} --install .)
endif()

# ---------------------------------------------------------------------------
# boundscheck - sourced from vm/boundscheck
# ---------------------------------------------------------------------------
if(CODIRA_CODEGEN_CODENATIVE_BACKEND)
    add_subdirectory(
        ${CODIRA_BOUNDSCHECK_DIR}
        ${CMAKE_BINARY_DIR}/vm/boundscheck)
    install(
        TARGETS boundscheck
        RUNTIME DESTINATION
            runtime/lib/${TARGET_TRIPLE_DIRECTORY_PREFIX}_${CODENATIVE_BACKEND}${SANITIZER_SUBPATH}
        LIBRARY DESTINATION
            runtime/lib/${TARGET_TRIPLE_DIRECTORY_PREFIX}_${CODENATIVE_BACKEND}${SANITIZER_SUBPATH}
    )
    install(TARGETS boundscheck-static
        DESTINATION
            lib/${TARGET_TRIPLE_DIRECTORY_PREFIX}_${CODENATIVE_BACKEND}${SANITIZER_SUBPATH})
    if(WIN32 AND CODIRA_BUILD_CODEDB)
        install(TARGETS boundscheck RUNTIME DESTINATION vm/core/bin)
    endif()
    install(FILES include/codira/MetaTransformation/MetaTransform.h
        DESTINATION include/)
endif()

# ---------------------------------------------------------------------------
# Subdirectories
# ---------------------------------------------------------------------------
# compiler-local subdirectories (all paths relative to CMAKE_SOURCE_DIR = compiler/)
if(CODIRA_CODEGEN_CODENATIVE_BACKEND)
    add_subdirectory(schema)
    add_subdirectory(src)
endif()
if(CODIRA_BUILD_CODEC)
    add_subdirectory(utils)
endif()

# ---------------------------------------------------------------------------
# Tests - prefer vm/gtest, fall back to system GTest
# ---------------------------------------------------------------------------
if(CODIRA_BUILD_TESTS)
    if(IS_DIRECTORY "${CODIRA_GTEST_DIR}")
        # Prevent overriding parent project compiler/linker settings on Windows
        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
        add_subdirectory(
            ${CODIRA_GTEST_DIR}
            ${CMAKE_BINARY_DIR}/vm/gtest
            EXCLUDE_FROM_ALL)
        set(GTest_FOUND TRUE)
    else()
        find_package(GTest)
        if(NOT GTest_FOUND)
            message(WARNING
                "GTest not found and vm/gtest is absent. Unit tests disabled.")
        endif()
    endif()
endif()

if(CODIRA_BUILD_CODEC AND CODIRA_BUILD_TESTS AND GTest_FOUND)
    enable_testing()
    add_subdirectory(unittests)
endif()

install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/codira DESTINATION include/)