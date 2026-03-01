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

add_custom_target(checked_tests COMMENT "Running tests with checkers")
add_dependencies(checked_tests ark)

# Unit tests checker.rb script.
if (PANDA_WITH_TESTS AND PANDA_TARGET_AMD64)
    set(CHECKER_UNIT_TESTS_DIR "${CMAKE_CURRENT_BINARY_DIR}/unit_tests")
    file(MAKE_DIRECTORY ${CHECKER_UNIT_TESTS_DIR})
    add_custom_target(checker_unit_tests COMMENT "Checker tests"
        COMMAND /usr/bin/env ruby ${CMAKE_CURRENT_SOURCE_DIR}/checker_tests.rb
        WORKING_DIRECTORY ${CHECKER_UNIT_TEST_DIR}
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/checker_tests.rb)
    add_dependencies(tests checker_unit_tests)
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

    # Events work only in Debug mode
    if (NOT DEFINED ARG_SUPPORT_RELEASE AND NOT ${CMAKE_BUILD_TYPE} MATCHES "Debug")
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

    if (NOT ${CMAKE_BUILD_TYPE} MATCHES "Debug")
        set(RELEASE_OPT "--release")
    endif()

    # ARCHITECTURE need to skip PAOC tests for arm32
    if (PANDA_TARGET_AMD64)
        set(ARCHITECTURE "x64")
    elseif (PANDA_TARGET_ARM64)
        set(ARCHITECTURE "arm64")
    else()
        set(ARCHITECTURE "arm32")
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
                                         ${RELEASE_OPT}
                      WORKING_DIRECTORY ${TEST_DIR}
                      COMMENT "Running ${TEST_NAME} checked test"
                      DEPENDS ${BINARY_FILE} ${EXT_FILES})

    add_dependencies(checked_tests ${TEST_NAME})

    foreach(AUX_FILE ${ARG_AUX_FILES})
        get_filename_component(AUX_NAME "${AUX_FILE}" NAME_WE)
        set(AUX_NAME "${AUX_NAME}.checked")
        add_dependencies(${TEST_NAME} ${AUX_NAME})
    endforeach()

endfunction()

# Disable checked tests with sanitizers for arm64, since it takes too long time running on qemu.
if (PANDA_TARGET_AMD64 OR (PANDA_TARGET_ARM64 AND NOT PANDA_ENABLE_ADDRESS_SANITIZER AND NOT PANDA_ENABLE_THREAD_SANITIZER))
    panda_add_checked_test(FILE ${CMAKE_CURRENT_SOURCE_DIR}/aot.pa)
    panda_add_checked_test(FILE ${CMAKE_CURRENT_SOURCE_DIR}/const_array_test.pa)
    panda_add_checked_test(FILE ${CMAKE_CURRENT_SOURCE_DIR}/disable_intrinsics.pa)
    panda_add_checked_test(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ldarray_obj.pa)
    panda_add_checked_test(FILE ${CMAKE_CURRENT_SOURCE_DIR}/float_zero.pa)
    panda_add_checked_test(FILE ${CMAKE_CURRENT_SOURCE_DIR}/inline_external.pa)
    panda_add_checked_test(FILE ${CMAKE_CURRENT_SOURCE_DIR}/irreducible_loop_test.pa)
    panda_add_checked_test(FILE ${CMAKE_CURRENT_SOURCE_DIR}/monitor.pa)
    panda_add_checked_test(FILE ${CMAKE_CURRENT_SOURCE_DIR}/basics_aot.pa)
    panda_add_checked_test(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ref_check_elim_test.pa)
    panda_add_checked_test(NAME verify_aot_tests_file1 FILE ${CMAKE_CURRENT_SOURCE_DIR}/verify_aot_tests/file1/test.pa)
    panda_add_checked_test(NAME verify_aot_tests_file2 FILE ${CMAKE_CURRENT_SOURCE_DIR}/verify_aot_tests/file2/test.pa)
    if (NOT PANDA_PRODUCT_BUILD)
        panda_add_checked_test(FILE ${CMAKE_CURRENT_SOURCE_DIR}/tlab_test.pa)
    endif()
    panda_add_checked_test(FILE ${CMAKE_CURRENT_SOURCE_DIR}/aot_cha.pa AUX_FILES basics_aot.pa)
    panda_add_checked_test(FILE ${CMAKE_CURRENT_SOURCE_DIR}/inline.pa AUX_FILES inline_external.pa)
    panda_add_checked_test(FILE ${CMAKE_CURRENT_SOURCE_DIR}/verify_aot_tests/verify_aot_test.pa AUX_FILES verify_aot_tests_file1 verify_aot_tests_file2)
    panda_add_checked_test(FILE ${CMAKE_CURRENT_SOURCE_DIR}/zero_const_in_save_state.pa)
endif()

panda_add_checked_test(FILE ${CMAKE_CURRENT_SOURCE_DIR}/implicit_nullcheck_tests.pa)
panda_add_checked_test(FILE ${CMAKE_CURRENT_SOURCE_DIR}/parameter_test.pa SUPPORT_RELEASE true)
panda_add_checked_test(FILE ${CMAKE_CURRENT_SOURCE_DIR}/stack_overflow.pa SUPPORT_RELEASE true)

panda_add_checked_test(FILE ${CMAKE_CURRENT_SOURCE_DIR}/disasm_and_log_demo.pa)
if (PANDA_COMPILER_TARGET_AARCH64 AND PANDA_COMPILER_TARGET_AARCH32)
    panda_add_checked_test(FILE ${CMAKE_CURRENT_SOURCE_DIR}/cross_peephole.pa)
endif()
panda_add_checked_test(FILE ${CMAKE_CURRENT_SOURCE_DIR}/ifcvt.pa)
#Test for issues 1376 and 1413
panda_add_checked_test(FILE ${CMAKE_CURRENT_SOURCE_DIR}/remove_redundant_checks.pa)
panda_add_checked_test(FILE ${CMAKE_CURRENT_SOURCE_DIR}/deoptimize_compare.pa)

panda_add_checked_test(FILE ${CMAKE_CURRENT_SOURCE_DIR}/cast_bool.pa)
panda_add_checked_test(FILE ${CMAKE_CURRENT_SOURCE_DIR}/compare_lenarray_with_zero.pa)
