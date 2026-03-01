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

cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
project(PANDA NONE)

# Add our custom configuration types to
# multi-configuration generators (i.e. Visual Studio):
if(CMAKE_CONFIGURATION_TYPES)
    list(APPEND CMAKE_CONFIGURATION_TYPES "FastVerify" "DebugDetailed")
    list(REMOVE_DUPLICATES CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_CONFIGURATION_TYPES ${CMAKE_CONFIGURATION_TYPES}
        CACHE STRING "CMake configuration types" FORCE)
endif()

enable_language(CXX)

# NB! For God's sake do not touch the command below.
# See https://gitlab.kitware.com/cmake/cmake/issues/16588.
# and https://clang.llvm.org/docs/JSONCompilationDatabase.html
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ----- Default flags ----------------------------------------------------------

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ----- Global variables -------------------------------------------------------
# Please don't use CMAKE_SOURCE_DIR and CMAKE_BINARY_DIR to allow building
# Panda as a cmake subdirectory. You can use the following variables if you
# need to refer the Panda root directories
set(PANDA_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(PANDA_BINARY_ROOT ${CMAKE_CURRENT_BINARY_DIR})
set(PANDA_THIRD_PARTY_SOURCES_DIR ${PANDA_ROOT}/third_party)
set(PANDA_THIRD_PARTY_CONFIG_DIR ${PANDA_ROOT}/cmake/third_party)
set(ARKFILE_ROOT ${PANDA_ROOT}/libarkfile)

add_definitions(-DBUILD_FOLDER="${CMAKE_CURRENT_BINARY_DIR}")

# List for accumulation of all core gtests binary paths.
# It's used by CI to archive these binaries into a single artifact
# and send it to second stage where unit tests will use them.
set_property(GLOBAL PROPERTY stash_list "")

# ----- Policies ---------------------------------------------------------------
# Allows the target_link_libraries() command to be called from any directory
# to add link dependencies and link interface libraries to targets created in
# other directories
if(POLICY CMP0079)
    cmake_policy(SET CMP0079 NEW)
endif()

# ----- Platform definitions ---------------------------------------------------
include(cmake/Definitions.cmake)

if (NOT "${CMAKE_BUILD_TYPE}" MATCHES "Release" AND NOT PANDA_TARGET_WINDOWS)
    # Needed for stacktrace printing
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -rdynamic")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -rdynamic")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic -Wall -Wextra -Werror -Wshadow -Wunused-parameter")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti -fno-exceptions")

if(UNIT_TESTS_COVERAGE)
    if(NOT USE_GCOV_TOOLS AND NOT USE_LLVM_COV_TOOLS)
        message(SEND_ERROR "Please use USE_GCOV_TOOLS or USE_LLVM_COV_TOOLS with UNIT_TESTS_COVERAGE option" )
    endif()
    include(cmake/interceptor/AddCustomTargetInterceptor.cmake)
endif()

# CC-OFFNXT(bc-40028) FP
include(cmake/toolchain/coverage/unit_tests_lcov.cmake)

if(ENABLE_UNIT_TESTS_FULL_COVERAGE)
    add_custom_target(coverage_full DEPENDS cts-assembly tests benchmarks)
    collect_coverage_for_target(
        TARGET_NAME coverage_full
        EXCLUDE_DIR_PATTERN \"/usr*\";\"*/third_party/*\";\"*/build/*\";
    )
else()
    message(STATUS "Full coverage will not be calculated (may be enabled by -DENABLE_UNIT_TESTS_FULL_COVERAGE=true ).")
endif(ENABLE_UNIT_TESTS_FULL_COVERAGE)

if(PANDA_TARGET_MACOS)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mmacosx-version-min=10.13")
endif()

set(PANDA_PGO_PROFGEN_PATH "/data/local/tmp")

if (PANDA_TARGET_MOBILE AND (PANDA_TARGET_ARM64 OR PANDA_TARGET_ARM32))
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fuse-ld=lld")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=lld")
endif()

if (PANDA_PGO_INSTRUMENT)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-generate=${PANDA_PGO_PROFGEN_PATH}")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fprofile-generate=${PANDA_PGO_PROFGEN_PATH}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fprofile-generate=${PANDA_PGO_PROFGEN_PATH}")
endif()

if (PANDA_PGO_OPTIMIZE)
    if (NOT PANDA_PGO_PROFILE_DATA)
        message(FATAL_ERROR "PANDA_PGO_PROFILE_DATA is not set")
    endif()

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-use=${PANDA_PGO_PROFILE_DATA}")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fprofile-use=${PANDA_PGO_PROFILE_DATA}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fprofile-use=${PANDA_PGO_PROFILE_DATA}")
endif()

if (PANDA_ENABLE_LTO)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -flto=thin")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -flto=thin")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -flto=thin")
endif()

if (PANDA_LLVM_REGALLOC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mllvm -regalloc=${PANDA_LLVM_REGALLOC}")
endif()

if ("${CMAKE_BUILD_TYPE}" STREQUAL "RelWithDebInfo")
    add_compile_options(-O2 -ggdb3 -fno-omit-frame-pointer)
elseif ("${CMAKE_BUILD_TYPE}" STREQUAL "FastVerify")
    add_compile_options(-O2 -ggdb3 -fno-omit-frame-pointer)
elseif ("${CMAKE_BUILD_TYPE}" STREQUAL "DebugDetailed")
    add_compile_options(-Og -ggdb3 -fno-omit-frame-pointer)
endif()

if (PANDA_THREAD_SAFETY)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wthread-safety")
endif()

include(cmake/RegisterPlugins.cmake)

# ----- Deliverable executables and libraries ----------------------------------
# Please override with per-target properties if your artifact should reside
# elsewhere, like this:
# set_target_properties(... PROPERTIES RUNTIME_OUTPUT_DIRECTORY ...)
# Applicable for tests and all "internal" artifacts.
if(NOT HOST_TOOLS)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PANDA_BINARY_ROOT}/lib)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PANDA_BINARY_ROOT}/lib)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PANDA_BINARY_ROOT}/bin)
endif()

# ----- Alias for source generating targets ------------------------------------
add_custom_target(panda_gen_files COMMENT "Generate all sources")

# ----- Panda CMake functions --------------------------------------------------
include(cmake/PandaCmakeFunctions.cmake)

if (PANDA_LLVM_BACKEND)
    if (PANDA_LLVM_INTERPRETER)
        # ----- Enable LLVM Inline modules -------------------------------------
        include(libllvmbackend/cmake/LLVMInlineModules.cmake)
    endif()
    # ----- Enable LLVM Backend compiler ---------------------------------------
    if (PANDA_BUILD_LLVM_BACKEND)
        include(libllvmbackend/cmake/LLVM.cmake)
    endif()
endif ()

# ----- Bootstrapping (for parts of platform written in managed code ) ---------
include(cmake/HostTools.cmake)

if (PANDA_ENABLE_CCACHE)
    # ----- Enable CCache ----------------------------------------------------------
    include(cmake/PandaCCache.cmake)

    # ----- Enable SCCache ---------------------------------------------------------
    include(cmake/PandaSCCache.cmake)
endif()

# ----- Documentation generation -----------------------------------------------
include(cmake/Doxygen.cmake)

# ----- Code analysis and style ------------------------------------------------
include(cmake/ClangTidy.cmake)
include(cmake/CodeStyle.cmake)

# ----- Sanitizers testing -----------------------------------------------------
include(cmake/Sanitizers.cmake)

# ----- Enable testing ---------------------------------------------------------

# Umbrella target for testing:
add_custom_target(tests COMMENT "Running all test suites")

define_property(TARGET
                PROPERTY first-level-tests-dependency
                BRIEF_DOCS "Whether the target is a first-level dependency of tests"
                FULL_DOCS  "Whether the target is a first-level dependency of tests")

add_custom_target(core_tests COMMENT "Running core test suites")
set(DEFAULT_TEST_GROUP core_tests)

# NB! ADDING THIS PROPERTY IS ALLOWED ONLY IN SPECIAL CASES. DO NOT COPY-PASTE IT.
set_target_properties(core_tests PROPERTIES first-level-tests-dependency TRUE)
add_dependencies(tests core_tests)

include(cmake/Testing.cmake)

#-----Test code quality---------------------------------------------------------
add_custom_target(es2panda-pre-test
                  COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/tools/es2panda/scripts/es2panda_pre_test.py --working-dir ${PANDA_ROOT} --build-root ${CMAKE_CURRENT_BINARY_DIR}
                  COMMENT "Run parser, runtime, ets-cts, function test cases, clang-tidy and clang-format."
                  JOB_POOL console
                )

# ----- Template Based Generator -----------------------------------------------
include(cmake/TemplateBasedGen.cmake)

# ----- Enable panda assemblies building ---------------------------------------
include(cmake/PandaAssembly.cmake)

# Some compilers use x87 fp instructions by default in 32-bit mode.
# We need to use SSE one to correspond x86_64 build.
if (PANDA_TARGET_X86)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mfpmath=sse -msse3")
endif()

# ----- Targets ----------------------------------------------------------------

execute_process(COMMAND ${PANDA_ROOT}/scripts/install-third-party
                WORKING_DIRECTORY ${PANDA_ROOT}
                RESULT_VARIABLE THIRD_PARTY_OK)
if (NOT THIRD_PARTY_OK EQUAL 0)
    message(FATAL_ERROR "Unable to install required third-party dependencies")
endif()

if(PANDA_WITH_TOOLCHAIN)
    add_subdirectory(isa)

    set(SECUREC_ROOT ${PANDA_THIRD_PARTY_SOURCES_DIR}/utils_native/base)
    add_subdirectory(${PANDA_THIRD_PARTY_CONFIG_DIR}/securec)
    set(OPENSSL_ROOT ${PANDA_THIRD_PARTY_SOURCES_DIR}/openssl)
    add_subdirectory(${PANDA_THIRD_PARTY_CONFIG_DIR}/openssl)
    set(TOOLCHAIN_ROOT ${PANDA_THIRD_PARTY_SOURCES_DIR}/arkcompiler/toolchain)
    add_subdirectory(${PANDA_THIRD_PARTY_CONFIG_DIR}/toolchain_websocket)
    add_subdirectory(libarkbase)

    set(ZLIB_ROOT ${PANDA_THIRD_PARTY_SOURCES_DIR}/zlib)
    add_subdirectory(${PANDA_THIRD_PARTY_CONFIG_DIR}/zlib)
    add_subdirectory(libziparchive)

    add_subdirectory(libarkfile)
    if(NOT PANDA_TARGET_WINDOWS)
        add_subdirectory(libarkfile/external)
    endif()

    add_subdirectory(abc2program)
    add_subdirectory(assembler)
    add_subdirectory(disassembler)

    if(PANDA_WITH_RUNTIME)
        add_subdirectory(verification)
    endif()

    if(PANDA_WITH_COMPILER)
        add_subdirectory(bytecode_optimizer)
    endif()
endif()

if(PANDA_WITH_COMPILER)
    add_compile_definitions(PANDA_WITH_IRTOC PANDA_WITH_CODEGEN)
    add_subdirectory(cross_values)
    if (PANDA_TARGET_X86 OR PANDA_TARGET_AMD64)
        set(ASMJIT_STATIC TRUE)
        set(PREV_CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-bitwise-instead-of-logical -Wno-unused-but-set-variable -Wno-deprecated-copy -Wno-unknown-warning-option")
        add_subdirectory(${PANDA_THIRD_PARTY_SOURCES_DIR}/asmjit)
        add_subdirectory(${PANDA_THIRD_PARTY_SOURCES_DIR}/zydis)
        set(CMAKE_CXX_FLAGS "${PREV_CMAKE_CXX_FLAGS}")
    endif()

    if (PANDA_BUILD_LLVM_BACKEND)
        add_subdirectory(libllvmbackend)
    endif()

    set(PREV_CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-bitwise-instead-of-logical -Wno-unknown-warning-option")
    add_subdirectory(${PANDA_THIRD_PARTY_SOURCES_DIR}/vixl)
    set(CMAKE_CXX_FLAGS "${PREV_CMAKE_CXX_FLAGS}")
    add_subdirectory(irtoc)
    add_subdirectory(compiler/optimizer/code_generator)
    add_subdirectory(compiler)
    set(IRTOC_INTRINSICS_YAML ${PANDA_ROOT}/irtoc/intrinsics.yaml)
    # Irtoc is built within the host tools in cross-compiling mode.
    if(NOT (CMAKE_CROSSCOMPILING OR PANDA_TARGET_OHOS))
        add_subdirectory(irtoc/backend)
        add_subdirectory(${PANDA_THIRD_PARTY_SOURCES_DIR}/elfio)
    endif()
else()
    panda_add_library(arkcompiler ${PANDA_DEFAULT_LIB_TYPE})
    panda_add_library(arkbytecodeopt ${PANDA_DEFAULT_LIB_TYPE})
    panda_add_library(arkaotmanager ${PANDA_DEFAULT_LIB_TYPE})
endif()

if(PANDA_WITH_RUNTIME)
    add_subdirectory(pandastdlib)

    if(NOT PANDA_TARGET_WINDOWS)
        add_subdirectory(dprof)
    endif()

    add_subdirectory(runtime)

    add_subdirectory(panda)

    add_subdirectory(verification/verifier)

    set(ICU_ROOT ${PANDA_THIRD_PARTY_SOURCES_DIR}/icu)
    add_subdirectory(${PANDA_THIRD_PARTY_CONFIG_DIR}/icu)
    add_subdirectory(${PANDA_THIRD_PARTY_CONFIG_DIR}/pcre2)
endif()

# ----- Testing ----------------------------------------------------------------

if(PANDA_WITH_TESTS)
    add_subdirectory(${PANDA_THIRD_PARTY_SOURCES_DIR}/googletest)
    set_target_properties(gtest PROPERTIES POSITION_INDEPENDENT_CODE ON)
    set_target_properties(gtest_main PROPERTIES POSITION_INDEPENDENT_CODE ON)
    set_target_properties(gmock PROPERTIES POSITION_INDEPENDENT_CODE ON)
    panda_target_compile_options(gtest PRIVATE "-Wno-implicit-float-conversion")

    option(RC_ENABLE_RTTI OFF)
    if(NOT PANDA_USE_PREBUILT_TARGETS)
    # Skip rapidcheck building when using prebuilt targets, because we don't need static
    # libraries at this stage
        add_subdirectory(${PANDA_THIRD_PARTY_SOURCES_DIR}/rapidcheck)
        add_subdirectory(${PANDA_THIRD_PARTY_SOURCES_DIR}/rapidcheck/extras/gtest)
        panda_target_compile_options(rapidcheck PUBLIC "-fexceptions" "-fno-rtti" "-fPIC")
    endif()

    add_subdirectory(tests)

    add_custom_target(tests_full COMMENT "Running all test suites and code checks")
    add_dependencies(tests_full
        tests
        cmake-checker
        test-cmake-checker
    )

    if (NOT PANDA_TARGET_MACOS)
        add_dependencies(tests_full code-style-check)
    endif()

    add_subdirectory(scripts)

    set(TYPESCRIPT_ROOT ${PANDA_THIRD_PARTY_SOURCES_DIR}/typescript)
    add_subdirectory(${PANDA_THIRD_PARTY_CONFIG_DIR}/typescript)
endif()

# ----- Aliases ----------------------------------------------------------------

add_custom_target(panda_bins COMMENT "Build all common Panda binaries")
add_dependencies(panda_bins panda pandasm ark_disasm ark_link paoc verifier aspt_converter)

# ----- Benchmarking -----------------------------------------------------------

if(PANDA_WITH_BENCHMARKS)
    # NB! Please do not merge benchmarks and tests unless you want to mess with
    # slow builds, etc. If you want some coupling, you might want to make benchmarks
    # depend on tests some day.

    add_custom_target(benchmarks COMMENT "Running all benchmark suites")
    add_subdirectory(tests/benchmarks)
endif()

# ----- Panda tools -----------------------------------------------

add_subdirectory(tools)

# ----- Plugins ----------------------------------------------------------------

add_subdirectory(plugins)
include(cmake/PostPlugins.cmake)

# ----- Platforms --------------------------------------------------------------

add_subdirectory(platforms)

# ----- Extras ----------------------------------------------------------------

add_subdirectory(extras)

# ----- Quickening tool --------------------------------------------------------

add_subdirectory(quickener)

# ----- Abc linker tool --------------------------------------------------------

if(PANDA_WITH_TOOLCHAIN)
    add_subdirectory(static_linker)
endif()

# ----- Collecting core gtests paths for CI stash ---------------------------------
# NB! This must be the last section!

# Check that the tests target has only first-level groups as dependencies.
# If you want to create a new first-level group of tests, mark it as 'first-level-tests-dependency'
# using the following command set_target_properties(<target> PROPERTIES first-level-tests-dependency TRUE)
# and don't forget to handle it on CI.
get_target_property(dependencies tests MANUALLY_ADDED_DEPENDENCIES)
foreach(dep IN LISTS dependencies)
    get_target_property(first_level_dep ${dep} first-level-tests-dependency)
    if(NOT first_level_dep)
        message(FATAL_ERROR "Target ${dep} must not be added to tests dependencies directly.")
    endif()
endforeach()

# Write to a file the list of all binaries
get_property(stash_files GLOBAL PROPERTY stash_list)
list(JOIN stash_files "\n" file_content)
file(WRITE ${PANDA_BINARY_ROOT}/core_stash_files.txt "${file_content}\n")
