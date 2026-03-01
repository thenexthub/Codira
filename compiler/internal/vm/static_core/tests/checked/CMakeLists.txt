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

add_custom_target(core_checked_tests COMMENT "Running core tests with checkers")
add_dependencies(core_checked_tests ark)

# Unit tests checker.rb script.
if (PANDA_WITH_TESTS AND PANDA_TARGET_AMD64)
    set(CHECKER_UNIT_TESTS_DIR "${CMAKE_CURRENT_BINARY_DIR}/unit_tests")
    file(MAKE_DIRECTORY ${CHECKER_UNIT_TESTS_DIR})
    add_custom_target(checker_unit_tests COMMENT "Checker tests"
        COMMAND /usr/bin/env ruby ${CMAKE_CURRENT_SOURCE_DIR}/checker_tests.rb
        WORKING_DIRECTORY ${CHECKER_UNIT_TEST_DIR}
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/checker_tests.rb)
    add_dependencies(core_tests checker_unit_tests)
endif()

function(compile_file)
    set(prefix ARG)
    set(singleValues FILE OUTPUT_FILE WORKING_DIR)
    cmake_parse_arguments(${prefix} "" "${singleValues}" "${multiValues}" ${ARGN})

    # Compile assembly file
    add_custom_command(OUTPUT "${ARG_OUTPUT_FILE}"
            COMMENT "Building ${TEST_NAME}"
            COMMAND ${PANDA_RUN_PREFIX} $<TARGET_FILE:ark_asm> --log-file ${BUILD_LOG} ${ARG_FILE} ${ARG_OUTPUT_FILE}
            DEPENDS ${assembler} "${ARG_FILE}"
            WORKING_DIRECTORY "${ARG_WORKING_DIR}")
endfunction()

function(panda_add_checked_test)
    set(prefix ARG)
    set(singleValues NAME FILE SUPPORT_RELEASE)
    set(multiValues AUX_FILES EXT_FILES)
    cmake_parse_arguments(${prefix} "" "${singleValues}" "${multiValues}" ${ARGN})

    # Events don't work in release mode
    if (NOT DEFINED ARG_SUPPORT_RELEASE AND "${CMAKE_BUILD_TYPE}" STREQUAL "Release")
        return()
    endif()

    if ("${ARG_NAME}" STREQUAL "")
        get_filename_component(TEST_NAME "${ARG_FILE}" NAME_WE)
        set(TEST_NAME "${TEST_NAME}.checked")
    else()
        set(TEST_NAME "${ARG_NAME}.checked")
    endif()
    set(CHECKER "${CMAKE_CURRENT_SOURCE_DIR}/checker.rb")
    set(TEST_DIR "${CMAKE_CURRENT_BINARY_DIR}/${TEST_NAME}")
    set(BINARY_FILE "${TEST_DIR}/test.abc")
    set(BUILD_LOG   "${TEST_DIR}/build.log")

    if (NOT DEFINED ARG_FILE)
        message(FATAL_ERROR "Mandatory FILE argument is not defined.")
    endif()

    file(MAKE_DIRECTORY "${TEST_DIR}")

    set(stdlibs "${arkstdlib_BINARY_DIR}/arkstdlib.abc")
    set(spaces  "core")

    compile_file(FILE ${ARG_FILE} OUTPUT_FILE ${BINARY_FILE} WORKING_DIR ${TEST_DIR})

    set(EXT_FILES "")
    foreach(APP_FILE ${ARG_EXT_FILES})
        get_filename_component(APP_FILE_BASE "${APP_FILE}" NAME_WE)
        set(APP_FILE_OUTPUT "${TEST_DIR}/${APP_FILE_BASE}.bin")
        compile_file(FILE ${APP_FILE} OUTPUT_FILE "${APP_FILE_OUTPUT}" WORKING_DIR ${TEST_DIR})
        # TODO(msherstennikov): pass external files via --app-panda-files (now it doesn't work)
        list(APPEND stdlibs "${APP_FILE_OUTPUT}")
        list(APPEND EXT_FILES "${APP_FILE_OUTPUT}")
    endforeach()

    string(REPLACE ";" ":" boot_stdlibs "${stdlibs}")
    string(REPLACE ";" ":" boot_spaces  "${spaces}")

    get_filename_component(TEST_TYPE "${ARG_FILE}" EXT)
    set(COMMAND_TOKEN "#!")

    set(OPTIONS "--boot-panda-files"
            "${boot_stdlibs}"
            "--load-runtimes=${boot_spaces}")

    if (NOT PANDA_CI_TESTING_MODE STREQUAL "Nightly")
        set (OPTIONS ${OPTIONS} "--compiler-check-final=true")
    endif()

    if ("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
        set(RELEASE_OPT "--release")
    endif()

    if (PANDA_LLVM_AOT)
        set(WITH_LLVM "--with-llvm")
    endif()

    # ARCHITECTURE need to skip PAOC tests for arm32
    if (PANDA_TARGET_AMD64)
        set(ARCHITECTURE "x64")
    elseif (PANDA_TARGET_ARM64)
        set(ARCHITECTURE "arm64")
    else()
        set(ARCHITECTURE "arm32")
    endif()

    if (ARK_HYBRID OR NOT PANDA_32_BIT_MANAGED_POINTER)
        set (CHECKER_BUILD_CONFIG "Hybrid")
    else()
        set (CHECKER_BUILD_CONFIG "Static")
    endif()

    # Run checker
    add_custom_target(${TEST_NAME}
                      COMMAND ${CHECKER} --source ${ARG_FILE}
                                         --panda $<TARGET_FILE:ark>
                                         --paoc $<TARGET_FILE:ark_aot>
                                         --run-prefix \"${PANDA_RUN_PREFIX}\"
                                         --test-file ${BINARY_FILE}
                                         --panda-options \"${OPTIONS}\"
                                         --paoc-options \"${OPTIONS}\"
                                         --command-token \"${COMMAND_TOKEN}\"
                                         --arch ${ARCHITECTURE}
                                         --build-config ${CHECKER_BUILD_CONFIG}
                                         ${RELEASE_OPT}
                                         ${WITH_LLVM}
                      WORKING_DIRECTORY ${TEST_DIR}
                      COMMENT "Running ${TEST_NAME} checked test"
                      DEPENDS ${BINARY_FILE} ${EXT_FILES})

    add_dependencies(core_checked_tests ${TEST_NAME})

    foreach(AUX_FILE ${ARG_AUX_FILES})
        get_filename_component(AUX_NAME "${AUX_FILE}" NAME_WE)
        set(AUX_NAME "${AUX_NAME}.checked")
        add_dependencies(${TEST_NAME} ${AUX_NAME})
    endforeach()

endfunction()
if (PANDA_COMPILER_DEBUG_INFO)
    panda_add_checked_test(FILE ${CMAKE_CURRENT_SOURCE_DIR}/source_line.pa)
endif()

# Disable checked tests with sanitizers for arm64, since it takes too long time running on qemu.
if (PANDA_TARGET_AMD64 OR (PANDA_TARGET_ARM64 AND NOT PANDA_ENABLE_ADDRESS_SANITIZER AND NOT PANDA_ENABLE_THREAD_SANITIZER))
    panda_add_checked_test(FILE ${CMAKE_CURRENT_SOURCE_DIR}/aot.pa)
    panda_add_checked_test(FILE ${CMAKE_CURRENT_SOURCE_DIR}/aot_black_list.pa)
    panda_add_checked_test(FILE ${CMAKE_CURRENT_SOURCE_DIR}/aot_black_list_method.pa)
    panda_add_checked_test(FILE ${CMAKE_CURRENT_SOURCE_DIR}/const_array_test.pa)
    panda_add_checked_test(FILE ${CMAKE_CURRENT_SOURCE_DIR}/disable_intrinsics.pa)
    panda_add_checked_test(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ldarray_obj.pa)
    panda_add_checked_test(FILE ${CMAKE_CURRENT_SOURCE_DIR}/float_zero.pa)
    panda_add_checked_test(FILE ${CMAKE_CURRENT_SOURCE_DIR}/force_an.pa)
    panda_add_checked_test(FILE ${CMAKE_CURRENT_SOURCE_DIR}/inline_external.pa)
    panda_add_checked_test(FILE ${CMAKE_CURRENT_SOURCE_DIR}/irreducible_loop_test.pa)
    panda_add_checked_test(FILE ${CMAKE_CURRENT_SOURCE_DIR}/monitor.pa)
    panda_add_checked_test(FILE ${CMAKE_CURRENT_SOURCE_DIR}/lowering_test.pa)
    panda_add_checked_test(FILE ${CMAKE_CURRENT_SOURCE_DIR}/basics_aot.pa)
    panda_add_checked_test(FILE ${CMAKE_CURRENT_SOURCE_DIR}/vcalls_aux.pa)
    panda_add_checked_test(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ref_check_elim_test.pa)
    panda_add_checked_test(FILE ${CMAKE_CURRENT_SOURCE_DIR}/isinstance_nullcheck.pa)
    panda_add_checked_test(FILE ${CMAKE_CURRENT_SOURCE_DIR}/isinstance_elimination_test.pa)
    panda_add_checked_test(FILE ${CMAKE_CURRENT_SOURCE_DIR}/checkcast_elimination_test.pa)
    panda_add_checked_test(FILE ${CMAKE_CURRENT_SOURCE_DIR}/checkcast_nullcheck.pa)
    panda_add_checked_test(FILE ${CMAKE_CURRENT_SOURCE_DIR}/inline_ic.pa)
    panda_add_checked_test(NAME verify_aot_tests_file1 FILE ${CMAKE_CURRENT_SOURCE_DIR}/verify_aot_tests/file1/test.pa)
    panda_add_checked_test(NAME verify_aot_tests_file2 FILE ${CMAKE_CURRENT_SOURCE_DIR}/verify_aot_tests/file2/test.pa)
    if (NOT PANDA_PRODUCT_BUILD)
        panda_add_checked_test(FILE ${CMAKE_CURRENT_SOURCE_DIR}/tlab_test.pa)
    endif()
    panda_add_checked_test(FILE ${CMAKE_CURRENT_SOURCE_DIR}/aot_cha.pa AUX_FILES basics_aot.pa)
    panda_add_checked_test(FILE ${CMAKE_CURRENT_SOURCE_DIR}/aot_cha_vcalls.pa AUX_FILES vcalls_aux.pa)
    panda_add_checked_test(FILE ${CMAKE_CURRENT_SOURCE_DIR}/inline_small.pa)
    panda_add_checked_test(FILE ${CMAKE_CURRENT_SOURCE_DIR}/inline.pa AUX_FILES inline_external.pa)
    panda_add_checked_test(FILE ${CMAKE_CURRENT_SOURCE_DIR}/verify_aot_tests/verify_aot_test.pa AUX_FILES verify_aot_tests_file1 verify_aot_tests_file2)
    panda_add_checked_test(FILE ${CMAKE_CURRENT_SOURCE_DIR}/zero_const_in_save_state.pa)
    panda_add_checked_test(FILE ${CMAKE_CURRENT_SOURCE_DIR}/loop-unroll-constant.pa)
    panda_add_checked_test(FILE ${CMAKE_CURRENT_SOURCE_DIR}/llvm_devirt_external.pa AUX_FILES vcalls_aux.pa)
    panda_add_checked_test(FILE ${CMAKE_CURRENT_SOURCE_DIR}/memory-coalescing.pa)
    panda_add_checked_test(FILE ${CMAKE_CURRENT_SOURCE_DIR}/memory-coalescing1.pa)
    if (PANDA_LLVM_AOT)
        panda_add_checked_test(FILE ${CMAKE_CURRENT_SOURCE_DIR}/virtual_to_static.pa)

        panda_add_checked_test(FILE ${CMAKE_CURRENT_SOURCE_DIR}/memset.pa)
        panda_add_checked_test(FILE ${CMAKE_CURRENT_SOURCE_DIR}/llvmaot-optlevel.pa)
        panda_add_checked_test(FILE ${CMAKE_CURRENT_SOURCE_DIR}/llvmaot-broken-ir.pa)
        panda_add_checked_test(FILE ${CMAKE_CURRENT_SOURCE_DIR}/llvmaot-gc-liveness.pa)
        panda_add_checked_test(FILE ${CMAKE_CURRENT_SOURCE_DIR}/llvmaot-tracking.pa)
        panda_add_checked_test(FILE ${CMAKE_CURRENT_SOURCE_DIR}/llvmaot-inlines-from-split-module.pa)
        panda_add_checked_test(FILE ${CMAKE_CURRENT_SOURCE_DIR}/llvmaot-gc-int-vs-ptr.pa)
        panda_add_checked_test(FILE ${CMAKE_CURRENT_SOURCE_DIR}/llvmaot-mem-barriers-sinking.pa)
        if (PANDA_TARGET_AMD64)
            panda_add_checked_test(FILE ${CMAKE_CURRENT_SOURCE_DIR}/fill-const-array/fill-const-array-513.pa)
        endif()
        if (PANDA_TARGET_ARM64)
            panda_add_checked_test(FILE ${CMAKE_CURRENT_SOURCE_DIR}/fill-const-array/fill-const-array-256.pa)
            panda_add_checked_test(FILE ${CMAKE_CURRENT_SOURCE_DIR}/fill-const-array/fill-const-array-257.pa)
        endif()
        panda_add_checked_test(FILE ${CMAKE_CURRENT_SOURCE_DIR}/const-array-vectorization.pa)
    endif()
endif()

panda_add_checked_test(FILE ${CMAKE_CURRENT_SOURCE_DIR}/implicit_nullcheck_tests.pa)
panda_add_checked_test(FILE ${CMAKE_CURRENT_SOURCE_DIR}/parameter_test.pa SUPPORT_RELEASE true)

# The test fails on WSL2 + cross qemu gcc aarch64
if (NOT PANDA_TARGET_GENERAL_WSL OR NOT PANDA_TARGET_AMD64) 
    panda_add_checked_test(FILE ${CMAKE_CURRENT_SOURCE_DIR}/stack_overflow.pa SUPPORT_RELEASE true)
endif()

panda_add_checked_test(FILE ${CMAKE_CURRENT_SOURCE_DIR}/disasm_and_log_demo.pa)
if (PANDA_COMPILER_TARGET_AARCH64 AND PANDA_COMPILER_TARGET_AARCH32)
    panda_add_checked_test(FILE ${CMAKE_CURRENT_SOURCE_DIR}/cross_peephole.pa)
endif()
panda_add_checked_test(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ifcvt.pa)
if (PANDA_COMPILER_TARGET_AARCH64)
    panda_add_checked_test(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ifcvt_float.pa)
    panda_add_checked_test(FILE ${CMAKE_CURRENT_SOURCE_DIR}/cleanup_sso.pa)
endif()
#Test for issues 1376 and 1413
panda_add_checked_test(FILE ${CMAKE_CURRENT_SOURCE_DIR}/remove_redundant_checks.pa)
panda_add_checked_test(FILE ${CMAKE_CURRENT_SOURCE_DIR}/deoptimize_compare.pa)

panda_add_checked_test(FILE ${CMAKE_CURRENT_SOURCE_DIR}/cast_bool.pa)
panda_add_checked_test(FILE ${CMAKE_CURRENT_SOURCE_DIR}/compare_lenarray_with_zero.pa)
panda_add_checked_test(FILE ${CMAKE_CURRENT_SOURCE_DIR}/compare_obj.pa)

if (PANDA_TARGET_ARM64)
    panda_add_checked_test(NAME core_osr FILE ${CMAKE_CURRENT_SOURCE_DIR}/osr.pa SUPPORT_RELEASE true)
    panda_add_checked_test(NAME core_osr_irtoc_test FILE ${CMAKE_CURRENT_SOURCE_DIR}/osr_irtoc_test.pa SUPPORT_RELEASE true)
    panda_add_checked_test(NAME initobj_osr FILE ${CMAKE_CURRENT_SOURCE_DIR}/initobj_osr.pa SUPPORT_RELEASE true)
endif()

if (NOT PANDA_TARGET_ARM32)
    panda_add_checked_test(FILE ${CMAKE_CURRENT_SOURCE_DIR}/memset_loop_idiom.pa)
    panda_add_checked_test(FILE ${CMAKE_CURRENT_SOURCE_DIR}/scalar_replacement.pa)
    if (("${CMAKE_BUILD_TYPE}" STREQUAL "Debug") OR ("${CMAKE_BUILD_TYPE}" STREQUAL "FastVerify"))
        panda_add_checked_test(FILE ${CMAKE_CURRENT_SOURCE_DIR}/force_unresolved_option.pa)
    endif()
endif()
panda_add_checked_test(FILE ${CMAKE_CURRENT_SOURCE_DIR}/checkcast_elimination.pa)
panda_add_checked_test(FILE ${CMAKE_CURRENT_SOURCE_DIR}/isinstance_elimination.pa)
panda_add_checked_test(FILE ${CMAKE_CURRENT_SOURCE_DIR}/combine_shifts.pa)
panda_add_checked_test(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ss_ir_builder.pa)
panda_add_checked_test(FILE ${CMAKE_CURRENT_SOURCE_DIR}/compiler_to_interpreter.pa)
panda_add_checked_test(FILE ${CMAKE_CURRENT_SOURCE_DIR}/float_intrinsic.pa SUPPORT_RELEASE true)
panda_add_checked_test(FILE ${CMAKE_CURRENT_SOURCE_DIR}/move_nullcheck_out_of_loop.pa SUPPORT_RELEASE true)
panda_add_checked_test(FILE ${CMAKE_CURRENT_SOURCE_DIR}/check_verifier.pa)
panda_add_checked_test(FILE ${CMAKE_CURRENT_SOURCE_DIR}/xor_to_compare_to_xor.pa)
panda_add_checked_test(FILE ${CMAKE_CURRENT_SOURCE_DIR}/memory-barrier.pa)
panda_add_checked_test(FILE ${CMAKE_CURRENT_SOURCE_DIR}/inline-external-constructor.pa EXT_FILES ${CMAKE_CURRENT_SOURCE_DIR}/memory-barrier.pa)

if (NOT PANDA_TARGET_ARM32)
    panda_add_checked_test(FILE ${CMAKE_CURRENT_SOURCE_DIR}/boot_an_location.pa)
endif()
