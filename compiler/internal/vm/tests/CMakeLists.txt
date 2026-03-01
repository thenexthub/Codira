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
# Test suite for Panda.

# Targets for running separate suites:
add_custom_target(cts-assembly COMMENT "Running compliance tests for Panda Assembly")
add_custom_target(regression COMMENT "Running panda compiler regression tests")
add_custom_target(cts-aot-tests COMMENT "Running panda aot tests")
add_custom_target(cts-jit-tests COMMENT "Running panda jit-enforce tests")
add_custom_target(irtoc-interpreter-tests COMMENT "Running irtoc interpreter tests")
add_custom_target(irtoc-opcode-suite COMMENT "Running irtoc interpreter opcode test suite")

add_dependencies(tests irtoc-interpreter-tests)
add_dependencies(irtoc-interpreter-tests irtoc-opcode-suite)
# TODO(maksenov): disable tests subdirectory for Windows build to remove this condition
if(NOT PANDA_TARGET_AMD64 OR PANDA_TARGET_WINDOWS)
  add_dependencies(tests cts-assembly regression)
  add_subdirectory(cts-coverage-tool)
else()
  add_custom_target(verifier-tests COMMENT "Regression tests for verifier")
  add_dependencies(tests cts-assembly regression cts-generator-build verifier-tests)
  add_subdirectory(cts-coverage-tool)
  add_subdirectory(cts-generator)
endif()
if (PANDA_TARGET_ARM64)
  add_custom_target(ecmascript-tests COMMENT "Compiler tests for ECMAScript")
  add_dependencies(tests ecmascript-tests)
endif()

if(PANDA_COMPILER_ENABLE)
    add_subdirectory(checked)
    add_dependencies(tests checked_tests)
endif()

# Add a single test run to the build tree.
#
# Usage:
#
#   panda_add_test_run(
#       FILE <source>
#       TARGET <target>
#       [SUBDIR <subdir>]
#       [EXPECTED_STDOUT <out_value>]
#       [SEARCH_DEBUG_STDERR <msg_value>]
#       [DEPENDS <dependency>...]
#       [RUNTIME_OPTIONS <option>...]
#       [COMPILER_OPTIONS <option>...]
#       [GC_OPTIONS <option>]
#       [ARGUMENTS <argument>...]
#       [AOT_MODE TRUE|FALSE]
#       [LANGUAGE_CONTEXT <context>]
#   )
#
# Adds a test run target <target> with <source>
#
# Options:
#
# SUBDIR
#   Subdirectory in the current binary directory that is used to store build artifacts.
#   Full path to directory with artifacts: ${CMAKE_CURRENT_BINARY_DIR}/<subdir>/<target>-execute
#
# EXPECTED_STDOUT
#   Excepted value of test's stdout
#
# SEARCH_DEBUG_STDERR
#   Message which is expected to present in stderr output
#
# DEPENDS
#   List of additional dependences (exclude assembler and interpreter)
#
# RUNTIME_OPTIONS
#   Runtime options
#
# COMPILER_OPTIONS
#   Options for compiler, given both to panda and paoc
#
# GC_OPTIONS
#  Type of a gc
#
# ARGUMENTS
#   List of arguments that will be passed to program's entry point
#
# AOT_MODE, LANGUAGE_CONTEXT
#   See panda_add_asm_file
function(panda_add_test_run)
    set(prefix ARG)
    set(singleValues FILE TARGET SUBDIR SEARCH_DEBUG_STDERR AOT_MODE LANGUAGE_CONTEXT GC_OPTIONS)
    set(multiValues EXPECTED_STDOUT DEPENDS RUNTIME_OPTIONS COMPILER_OPTIONS ARGUMENTS PRLIMIT_OPTIONS)
    cmake_parse_arguments(${prefix}
                          "${noValues}"
                          "${singleValues}"
                          "${multiValues}"
                          ${ARGN})

    if (NOT DEFINED ARG_FILE)
        message(FATAL_ERROR "Mandatory FILE argument is not defined.")
    endif()

    if (NOT DEFINED ARG_TARGET)
        message(FATAL_ERROR "Mandatory TARGET argument is not defined.")
    endif()

    if (TARGET ARG_TARGET)
        message(FATAL_ERROR "Target ${ARG_TARGET} is already defined.")
    endif()

    if(DEFINED ARG_GC_OPTIONS)
        set(ARG_RUNTIME_OPTIONS "${ARG_RUNTIME_OPTIONS}" "${ARG_GC_OPTIONS}")
    endif()

    set(output_file)
    set(error_file)

    panda_add_asm_file(
        FILE ${ARG_FILE}
        TARGET ${ARG_TARGET}-execute
        SUBDIR "${ARG_SUBDIR}"
        OUTPUT_FILE_VARIABLE output_file
        ERROR_FILE_VARIABLE error_file
        LANGUAGE_CONTEXT "${ARG_LANGUAGE_CONTEXT}"
        AOT_MODE ${ARG_AOT_MODE}
        DEPENDS "${ARG_DEPENDS}"
        RUNTIME_OPTIONS "${ARG_RUNTIME_OPTIONS}"
        COMPILER_OPTIONS "${ARG_COMPILER_OPTIONS}"
        AOT_GC_OPTION "${ARG_GC_OPTIONS}"
        ENTRY_ARGUMENTS "${ARG_ARGUMENTS}"
        PRLIMIT_OPTIONS "${ARG_PRLIMIT_OPTIONS}"
    )

    add_custom_target(${ARG_TARGET} COMMENT "Running ${ARG_TARGET} test")
    add_dependencies(${ARG_TARGET} ${ARG_TARGET}-execute)

    if (DEFINED ARG_EXPECTED_STDOUT)
        set(expected_output_file "${output_file}.expected")
        file(WRITE "${expected_output_file}" "${ARG_EXPECTED_STDOUT}")

        add_custom_target(${ARG_TARGET}-check-stdout
                          COMMENT "Check ${ARG_TARGET} stdout"
                          COMMAND ${CMAKE_COMMAND} -E compare_files "${output_file}" "${expected_output_file}"
                          DEPENDS ${ARG_TARGET}-execute)

        add_dependencies(${ARG_TARGET} ${ARG_TARGET}-check-stdout)
    endif()

    if (DEFINED ARG_SEARCH_DEBUG_STDERR AND NOT (CMAKE_BUILD_TYPE MATCHES Release))
        # TODO: Drop 'grep' usage, there must be some way like:
        #   file(READ "${error_file}" STDERR_AS_STRING)
        #   string(FIND ${STDERR_AS_STRING} ${ARG_SEARCH_DEBUG_STDERR})
        # but my cmake knowledge is not enough.

        add_custom_target(${ARG_TARGET}-check-logmsg
                          COMMENT "Check ${ARG_TARGET} log message"
                          COMMAND grep "${ARG_SEARCH_DEBUG_STDERR}" ${error_file} >/dev/null
                          DEPENDS ${ARG_TARGET}-execute)

        add_dependencies(${ARG_TARGET} ${ARG_TARGET}-check-logmsg)
    endif()

endfunction()

# Add a single test run for verifier to the build tree.
#
# Usage:
#
#   verifier_add_test_run(
#       FILE <source>
#       TARGET <target>
#       [SUBDIR <subdir>]
#       [EXPECTED_STDOUT <out_value>]
#       [SEARCH_STDERR <msg_value>]
#       [DEPENDS <dependency>...]
#       [LANGUAGE_CONTEXT <context>]
#   )
#
# Adds a test run target <target> with <source>
#
# Options:
#
# SUBDIR
#   Subdirectory in the current binary directory that is used to store build artifacts.
#   Full path to directory with artifacts: ${CMAKE_CURRENT_BINARY_DIR}/<subdir>/<target>-execute
#
# EXPECTED_STDOUT
#   Excepted value of test's stdout
#
# SEARCH_STDERR
#   Message which is expected to present in stderr output
#
# DEPENDS
#   List of additional dependences (exclude assembler and interpreter)
function(verifier_add_test_run)
    set(prefix ARG)
    set(noValues VERIFIER_FAIL_TEST)
    set(singleValues FILE TARGET SUBDIR LANGUAGE_CONTEXT)
    set(multiValues RUNTIME_OPTIONS VERIFIER_OPTIONS EXPECTED_STDOUT SEARCH_STDERR DEPENDS)
    cmake_parse_arguments(${prefix}
                          "${noValues}"
                          "${singleValues}"
                          "${multiValues}"
                          ${ARGN})

    if (NOT DEFINED ARG_FILE)
        message(FATAL_ERROR "Mandatory FILE argument is not defined.")
    endif()

    if (NOT DEFINED ARG_TARGET)
        message(FATAL_ERROR "Mandatory TARGET argument is not defined.")
    endif()

    if (TARGET ARG_TARGET)
        message(FATAL_ERROR "Target ${ARG_TARGET} is already defined.")
    endif()

    set(output_file)
    set(error_file)

    if (ARG_VERIFIER_FAIL_TEST)
        set(VERIFIER_FAIL_TEST VERIFIER_FAIL_TEST)
    else()
        set(VERIFIER_FAIL_TEST)
    endif()

    set(ARG_VERIFIER_OPTIONS "${ARG_VERIFIER_OPTIONS} --config-file=${CMAKE_CURRENT_SOURCE_DIR}/cts-generator/verifier.config")

    verifier_add_asm_file(
        FILE ${ARG_FILE}
        TARGET ${ARG_TARGET}-verify
        SUBDIR "${ARG_SUBDIR}"
        ${VERIFIER_FAIL_TEST}
        RUNTIME_OPTIONS "${ARG_RUNTIME_OPTIONS}"
        VERIFIER_OPTIONS "${ARG_VERIFIER_OPTIONS}"
        OUTPUT_FILE_VARIABLE output_file
        ERROR_FILE_VARIABLE error_file
        LANGUAGE_CONTEXT "${ARG_LANGUAGE_CONTEXT}"
        DEPENDS "${ARG_DEPENDS}"
    )

    add_custom_target(${ARG_TARGET} COMMENT "Verifying ${ARG_TARGET} test")
    add_dependencies(${ARG_TARGET} ${ARG_TARGET}-verify)

    if (DEFINED ARG_EXPECTED_STDOUT)
        set(expected_output_file "${output_file}.expected")
        file(WRITE "${expected_output_file}" "${ARG_EXPECTED_STDOUT}")

        add_custom_target(${ARG_TARGET}-check-stdout
                          COMMENT "Check ${ARG_TARGET} stdout"
                          COMMAND ${CMAKE_COMMAND} -E compare_files "${output_file}" "${expected_output_file}"
                          DEPENDS ${ARG_TARGET}-verify)

        add_dependencies(${ARG_TARGET} ${ARG_TARGET}-check-stdout)
    endif()

    if (DEFINED ARG_SEARCH_STDERR AND NOT (CMAKE_BUILD_TYPE MATCHES Release))
        # TODO: Drop 'grep' usage, there must be some way like:
        #   file(READ "${error_file}" STDERR_AS_STRING)
        #   string(FIND ${STDERR_AS_STRING} ${ARG_SEARCH_STDERR})
        # but my cmake knowledge is not enough.

        add_custom_target(${ARG_TARGET}-check-logmsg
                          COMMENT "Check ${ARG_TARGET} log message"
                          COMMAND grep -zo \"${ARG_SEARCH_STDERR}\" ${error_file} >/dev/null
                          DEPENDS ${ARG_TARGET}-verify)

        add_dependencies(${ARG_TARGET} ${ARG_TARGET}-check-logmsg)
    endif()

endfunction()


# Add a test with default and enforced jit runs
#
# Usage:
#
#   add_test_file(
#       FILE <source>
#       [RUNTIME_OPTIONS <option>...]
#       [COMPILER_OPTIONS <option>...]
#       [ARGUMENTS <argument>...]
#       [EXPECTED_STDOUT <out_value>]
#       [DEBUG_LOG_MESSAGE <msg_value>]
#       [DEBUG_LOG_MESSAGE_ENFORCED_JIT <jit_msg_value>]
#       [DISABLE_LIMIT_STD_ALLOC]
#       [SKIP_AOT]
#       [SKIP_OSR]
#       [SKIP_VERIFICATION]
#       [SKIP_INTERPRETER_IRTOC]
#       [SKIP_CROSS]
#   )
#
# Adds a test <source>
#
# Options:
#
# RUNTIME_OPTIONS
#   Runtime options
#
# COMPILER_OPTIONS
#   Options for compiler, given both to panda and paoc
#
# ARGUMENTS
#   List of arguments that will be passed to program's entry point
#
# EXPECTED_STDOUT
#   Excepted value of test's stdout
#
# DEBUG_LOG_MESSAGE
#   Message which is expected to present in stderr output with --log-level=debug
#
# DEBUG_LOG_MESSAGE_ENFORCED_JIT
#   Same as previous, but only for enforce-jit-compiler case.
#
# DISABLE_LIMIT_STD_ALLOC
#   Do not check malloc usage
#
# SKIP_AOT
#   Skip AOT run
#
# SKIP_OSR
#   Skip OSR run
#
# SKIP_VERIFICATION
#   Skip verification run
#
# SKIP_INTERPRETER_IRTOC
#   Skip run with irtoc implemented interpreter
#
# SKIP_CROSS
#   Skip cross-tests launch on host
#   Arguments: arm32, arm64
function(add_test_file)
    set(prefix ARG)
    set(noValues DISABLE_LIMIT_STD_ALLOC SKIP_AOT SKIP_OSR SKIP_VERIFICATION VERIFIER_FAIL_TEST SKIP_INTERPRETER_IRTOC)
    set(singleValues FILE DEBUG_LOG_MESSAGE DEBUG_LOG_MESSAGE_ENFORCED_JIT SKIP_CROSS)
    set(multiValues EXPECTED_STDOUT VERIFIER_OPTIONS VERIFIER_EXPECTED_STDOUT SEARCH_STDERR ARGUMENTS RUNTIME_OPTIONS
        COMPILER_OPTIONS PRLIMIT_OPTIONS)
    cmake_parse_arguments(${prefix}
                          "${noValues}"
                          "${singleValues}"
                          "${multiValues}"
                          ${ARGN})


    if (NOT DEFINED ARG_FILE)
        message(FATAL_ERROR "Mandatory FILE argument is not defined.")
    endif()

    get_filename_component(target "${ARG_FILE}" NAME)
    get_filename_component(path "${ARG_FILE}" DIRECTORY)
    get_filename_component(suite ${path} NAME)

    if (NOT ARG_DISABLE_LIMIT_STD_ALLOC)
        set(ARG_RUNTIME_OPTIONS "${ARG_RUNTIME_OPTIONS} --limit-standard-alloc=true")
    endif()

    if (PANDA_ENABLE_EVENTS)
        set(ARG_RUNTIME_OPTIONS "${ARG_RUNTIME_OPTIONS} --events-output=csv")
    endif()

    #TODO: add test suite to check with PANDA_COMPILER_TARGET
    if (((PANDA_TARGET_ARM64) OR (PANDA_TARGET_ARM32) OR (PANDA_TARGET_AMD64)) AND DEFINED ARG_DEBUG_LOG_MESSAGE_ENFORCED_JIT)
      set(CUR_RUNTIME_OPTIONS "${ARG_RUNTIME_OPTIONS}" "--log-level=debug")
      set(CUR_JIT_DEBUG_LOG_MESSAGE "${ARG_DEBUG_LOG_MESSAGE_ENFORCED_JIT}")
    elseif(DEFINED ARG_DEBUG_LOG_MESSAGE)
      set(CUR_RUNTIME_OPTIONS "${ARG_RUNTIME_OPTIONS}" "--log-level=debug")
      set(CUR_JIT_DEBUG_LOG_MESSAGE "${ARG_DEBUG_LOG_MESSAGE}")
      set(CUR_DEBUG_LOG_MESSAGE "${ARG_DEBUG_LOG_MESSAGE}")
    else()
      set(CUR_RUNTIME_OPTIONS "${ARG_RUNTIME_OPTIONS}")
    endif()

    # Enable compiler-runtime entrypoints verification. It slows down tests execution a bit. If it will be a problem,
    # consider removing this option from the cts tests.
    set(CUR_RUNTIME_OPTIONS "${CUR_RUNTIME_OPTIONS} --verify-entrypoints=true")

    set(language_context "core")

    if (ARG_VERIFIER_FAIL_TEST)
        set(VERIFIER_FAIL_TEST VERIFIER_FAIL_TEST)
    else()
        set(VERIFIER_FAIL_TEST)
    endif()

    if (NOT ARG_SKIP_VERIFICATION)
        verifier_add_test_run(
            FILE "${ARG_FILE}"
            TARGET ${target}-verifier
            SUBDIR ${suite}-verifier
            ${VERIFIER_FAIL_TEST}
            VERIFIER_OPTIONS "${ARG_VERIFIER_OPTIONS}"
            EXPECTED_STDOUT "${ARG_VERIFIER_EXPECTED_STDOUT}"
            SEARCH_STDERR "${ARG_SEARCH_STDERR}"
            LANGUAGE_CONTEXT "${language_context}"
        )
        add_dependencies(${suite} ${target}-verifier)
    endif()

    if (NOT ARG_SKIP_INTERPRETER_IRTOC AND NOT PANDA_TARGET_ARM32) # Arm32 is not supported in the Irtoc
        panda_add_test_run(
            FILE "${ARG_FILE}"
            TARGET ${target}-g1-gc-irtoc
            SUBDIR ${suite}-${gc_type}
            EXPECTED_STDOUT "${ARG_EXPECTED_STDOUT}"
            SEARCH_DEBUG_STDERR ${CUR_DEBUG_LOG_MESSAGE}
            RUNTIME_OPTIONS "${CUR_RUNTIME_OPTIONS}" "--compiler-enable-jit=false --gc-type=g1-gc --interpreter-type=irtoc"
            COMPILER_OPTIONS "${ARG_COMPILER_OPTIONS}"
            ARGUMENTS "${ARG_ARGUMENTS}"
            LANGUAGE_CONTEXT "${language_context}"
            AOT_MODE FALSE
            PRLIMIT_OPTIONS "${ARG_PRLIMIT_OPTIONS}"
        )
        add_dependencies(${suite} ${target}-g1-gc-irtoc)
    endif()

    set(gc_types "stw" "g1-gc")
    foreach (gc_type IN LISTS gc_types)
        panda_add_test_run(
            FILE "${ARG_FILE}"
            TARGET ${target}-${gc_type}-cpp
            SUBDIR ${suite}-${gc_type}
            EXPECTED_STDOUT "${ARG_EXPECTED_STDOUT}"
            SEARCH_DEBUG_STDERR ${CUR_DEBUG_LOG_MESSAGE}
            RUNTIME_OPTIONS "${CUR_RUNTIME_OPTIONS}" "--compiler-enable-jit=false --gc-type=${gc_type}"
            COMPILER_OPTIONS "${ARG_COMPILER_OPTIONS}"
            ARGUMENTS "${ARG_ARGUMENTS}"
            LANGUAGE_CONTEXT "${language_context}"
            AOT_MODE FALSE
            PRLIMIT_OPTIONS "${ARG_PRLIMIT_OPTIONS}"
        )
        add_dependencies(${suite} ${target}-${gc_type}-cpp)
    endforeach()

    # Check if JIT-compilation wasn't turned off explicitly:
    if (PANDA_COMPILER_ENABLE AND NOT CUR_RUNTIME_OPTIONS MATCHES "^.*--compiler-enable-jit=false.*$")
        panda_add_test_run(
            FILE "${ARG_FILE}"
            TARGET ${target}-cpp-enforce-jit-compiler
            SUBDIR ${suite}-enforce-jit-compiler
            EXPECTED_STDOUT "${ARG_EXPECTED_STDOUT}"
            SEARCH_DEBUG_STDERR ${CUR_JIT_DEBUG_LOG_MESSAGE}
            RUNTIME_OPTIONS "${CUR_RUNTIME_OPTIONS}" "--compiler-enable-jit=true --compiler-hotness-threshold=0 --no-async-jit=true"
            COMPILER_OPTIONS "${ARG_COMPILER_OPTIONS}"
            ARGUMENTS "${ARG_ARGUMENTS}"
            LANGUAGE_CONTEXT "${language_context}"
            AOT_MODE FALSE
            PRLIMIT_OPTIONS "${ARG_PRLIMIT_OPTIONS}"
        )
        add_dependencies(${suite} ${target}-cpp-enforce-jit-compiler)
        add_dependencies(cts-jit-tests ${target}-cpp-enforce-jit-compiler)

        if (PANDA_CI_TESTING_MODE STREQUAL "Nightly")
            panda_add_test_run(
                FILE "${ARG_FILE}"
                TARGET ${target}-cpp-enforce-jit-compiler-no-inline
                SUBDIR ${suite}-enforce-jit-compiler-no-inline
                EXPECTED_STDOUT "${ARG_EXPECTED_STDOUT}"
                SEARCH_DEBUG_STDERR ${CUR_JIT_DEBUG_LOG_MESSAGE}
                RUNTIME_OPTIONS "${CUR_RUNTIME_OPTIONS}" "--compiler-enable-jit=true --compiler-hotness-threshold=0 --no-async-jit=true  --compiler-inlining=false"
                COMPILER_OPTIONS "${ARG_COMPILER_OPTIONS}"
                ARGUMENTS "${ARG_ARGUMENTS}"
                LANGUAGE_CONTEXT "${language_context}"
                AOT_MODE FALSE
                PRLIMIT_OPTIONS "${ARG_PRLIMIT_OPTIONS}"
            )
            add_dependencies(${suite} ${target}-cpp-enforce-jit-compiler-no-inline)
            add_dependencies(cts-jit-tests ${target}-cpp-enforce-jit-compiler-no-inline)
        endif()

        if (PANDA_TARGET_ARM64 AND NOT ARG_SKIP_OSR)
            panda_add_test_run(
                FILE "${ARG_FILE}"
                TARGET ${target}-cpp-osr-jit
                SUBDIR ${suite}-osr-jit
                EXPECTED_STDOUT "${ARG_EXPECTED_STDOUT}"
                RUNTIME_OPTIONS "${CUR_RUNTIME_OPTIONS}" "--compiler-enable-jit=true --compiler-hotness-threshold=2 --compiler-enable-osr=true"
                COMPILER_OPTIONS "${ARG_COMPILER_OPTIONS}"
                ARGUMENTS "${ARG_ARGUMENTS}"
                LANGUAGE_CONTEXT "${language_context}"
                AOT_MODE FALSE
                PRLIMIT_OPTIONS "${ARG_PRLIMIT_OPTIONS}"
            )
            add_dependencies(${suite} ${target}-cpp-osr-jit)
            add_dependencies(cts-jit-tests ${target}-cpp-osr-jit)
        endif()
    endif()

    if (PANDA_TARGET_AMD64 AND PANDA_CI_TESTING_MODE STREQUAL "Nightly")
        if (NOT DEFINED ARG_SKIP_CROSS OR NOT ${ARG_SKIP_CROSS} MATCHES ".*arm32.*")
            panda_add_test_run(
                FILE "${ARG_FILE}"
                TARGET ${target}-arm32
                SUBDIR ${suite}-arm32
                EXPECTED_STDOUT "${ARG_EXPECTED_STDOUT}"
                SEARCH_DEBUG_STDERR ${CUR_DEBUG_LOG_MESSAGE}
                RUNTIME_OPTIONS "${CUR_RUNTIME_OPTIONS}" "--compiler-cross-arch=arm"
                COMPILER_OPTIONS "${ARG_COMPILER_OPTIONS}"
                ARGUMENTS "${ARG_ARGUMENTS}"
                LANGUAGE_CONTEXT "${language_context}"
                AOT_MODE FALSE
                PRLIMIT_OPTIONS "${ARG_PRLIMIT_OPTIONS}"
            )
            add_dependencies(${suite} ${target}-arm32)

            panda_add_test_run(
                FILE "${ARG_FILE}"
                TARGET ${target}-arm32-enforce-jit-compiler
                SUBDIR ${suite}-arm32-enforce-jit-compiler
                EXPECTED_STDOUT "${ARG_EXPECTED_STDOUT}"
                SEARCH_DEBUG_STDERR ${CUR_JIT_DEBUG_LOG_MESSAGE}
                RUNTIME_OPTIONS "${CUR_RUNTIME_OPTIONS}" "--compiler-hotness-threshold=0 --no-async-jit=true --compiler-cross-arch=arm"
                COMPILER_OPTIONS "${ARG_COMPILER_OPTIONS}"
                ARGUMENTS "${ARG_ARGUMENTS}"
                LANGUAGE_CONTEXT "${language_context}"
                AOT_MODE FALSE
                PRLIMIT_OPTIONS "${ARG_PRLIMIT_OPTIONS}"
            )
            add_dependencies(${suite} ${target}-arm32-enforce-jit-compiler)
        endif()

        if (NOT DEFINED ARG_SKIP_CROSS OR NOT ${ARG_SKIP_CROSS} MATCHES ".*arm64.*")
            panda_add_test_run(
                FILE "${ARG_FILE}"
                TARGET ${target}-arm64
                SUBDIR ${suite}-arm64
                EXPECTED_STDOUT "${ARG_EXPECTED_STDOUT}"
                SEARCH_DEBUG_STDERR ${CUR_DEBUG_LOG_MESSAGE}
                RUNTIME_OPTIONS "${CUR_RUNTIME_OPTIONS}" "--compiler-cross-arch=arm64"
                COMPILER_OPTIONS "${ARG_COMPILER_OPTIONS}"
                ARGUMENTS "${ARG_ARGUMENTS}"
                LANGUAGE_CONTEXT "${language_context}"
                AOT_MODE FALSE
                PRLIMIT_OPTIONS "${ARG_PRLIMIT_OPTIONS}"
            )
            add_dependencies(${suite} ${target}-arm64)

            panda_add_test_run(
                FILE "${ARG_FILE}"
                TARGET ${target}-arm64-enforce-jit-compiler
                SUBDIR ${suite}-arm64-enforce-jit-compiler
                EXPECTED_STDOUT "${ARG_EXPECTED_STDOUT}"
                SEARCH_DEBUG_STDERR ${CUR_JIT_DEBUG_LOG_MESSAGE}
                RUNTIME_OPTIONS "${CUR_RUNTIME_OPTIONS}" "--compiler-hotness-threshold=0 --no-async-jit=true --compiler-cross-arch=arm64"
                COMPILER_OPTIONS "${ARG_COMPILER_OPTIONS}"
                ARGUMENTS "${ARG_ARGUMENTS}"
                LANGUAGE_CONTEXT "${language_context}"
                AOT_MODE FALSE
                PRLIMIT_OPTIONS "${ARG_PRLIMIT_OPTIONS}"
            )
            add_dependencies(${suite} ${target}-arm64-enforce-jit-compiler)
        endif()
    endif()

    if ((PANDA_TARGET_ARM64 OR PANDA_TARGET_AMD64) AND NOT ARG_SKIP_AOT)
        foreach (gc_type IN LISTS gc_types)
            panda_add_test_run(
                FILE "${ARG_FILE}"
                TARGET ${target}-aot-${gc_type}
                SUBDIR ${suite}-aot-${gc_type}
                EXPECTED_STDOUT "${ARG_EXPECTED_STDOUT}"
                RUNTIME_OPTIONS "${CUR_RUNTIME_OPTIONS}"
                COMPILER_OPTIONS "${ARG_COMPILER_OPTIONS}"
                GC_OPTIONS "--gc-type=${gc_type}"
                ARGUMENTS "${ARG_ARGUMENTS}"
                LANGUAGE_CONTEXT "${language_context}"
                AOT_MODE TRUE
                PRLIMIT_OPTIONS "${ARG_PRLIMIT_OPTIONS}"
            )
            add_dependencies(${suite} ${target}-aot-${gc_type})
            add_dependencies(cts-aot-tests ${target}-aot-${gc_type})
        endforeach()
    endif()

endfunction()

# Add a test for verifier
#
# Usage:
#
#   add_verifier_test(
#       FILE <source>
#       [RUNTIME_OPTIONS <option>...]
#       [ARGUMENTS <argument>...]
#       [EXPECTED_STDOUT <out_value>]
#       [DEBUG_LOG_MESSAGE <msg_value>]
#   )
#
# Adds a test <source>
#
# Options:
#
# RUNTIME_OPTIONS
#   Runtime (interpreter and compiler) options
#
# VERIFER_OPTIONS
#   Verifier CLI additional options
#
# EXPECTED_STDOUT
#   Excepted value of test's stdout
#
# SEARCH_STDERR
#   Message which is expected to present in stderr output
#
# VERIFIER_FAIL_TEST
#   Force verifier do not set exit code
#
function(add_verifier_test)
    set(prefix ARG)
    set(noValues VERIFIER_FAIL_TEST)
    set(singleValues FILE)
    set(multiValues EXPECTED_STDOUT SEARCH_STDERR RUNTIME_OPTIONS VERIFIER_OPTIONS)
    cmake_parse_arguments(${prefix}
                          "${noValues}"
                          "${singleValues}"
                          "${multiValues}"
                          ${ARGN})


    if (NOT DEFINED ARG_FILE)
        message(FATAL_ERROR "Mandatory FILE argument is not defined.")
    endif()

    get_filename_component(target "${ARG_FILE}" NAME)
    get_filename_component(path "${ARG_FILE}" DIRECTORY)
    get_filename_component(suite ${path} NAME)

    if (ARG_VERIFIER_FAIL_TEST)
        set(VERIFIER_FAIL_TEST VERIFIER_FAIL_TEST)
    else()
        set(VERIFIER_FAIL_TEST)
    endif()

    set(language_context "core")

    verifier_add_test_run(
        FILE "${ARG_FILE}"
        TARGET ${target}-verifier
        SUBDIR ${suite}-verifier
        ${VERIFIER_FAIL_TEST}
        VERIFIER_OPTIONS "${ARG_VERIFIER_OPTIONS}"
        RUNTIME_OPTIONS "${ARG_RUNTIME_OPTIONS}"
        EXPECTED_STDOUT "${ARG_EXPECTED_STDOUT}"
        SEARCH_STDERR "${ARG_SEARCH_STDERR}"
        LANGUAGE_CONTEXT "${language_context}"
    )
    add_dependencies(${suite} ${target}-verifier)

endfunction()

add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/env-01.pa" ARGUMENTS -- 1 2 EXPECTED_STDOUT "1\n2\n")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/math-01.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/math-02.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/math-03.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/math-04.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/math-05.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/math-06.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/math-07.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/math-08.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/math-09.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/math-10.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/math-11.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/math-12.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/math-13.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/math-14.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/math-15.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/math-16.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/math-17.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/math-18.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/math-21.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/math-22.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/math-23.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/math-24.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/math-25.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/math-26.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/math-27.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/math-28.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/math-29.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/math-30.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/math-31.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/math-32.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/math-33.pa" SKIP_VERIFICATION)
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/math-34.pa" SKIP_VERIFICATION)
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/math-35.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/math-36.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/math-40.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/math-41.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/math-42.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/math-43.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/math-44.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/math-45.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/math-46.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/math-47.pa" EXPECTED_STDOUT "/ by zero" SKIP_INTERPRETER_IRTOC)
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/math-48.pa" EXPECTED_STDOUT "/ by zero" SKIP_INTERPRETER_IRTOC)
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/math-49.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/math-50.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/math-51.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/math-52.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/math-53.pa" COMPILER_OPTIONS --compiler-inlining=false)
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/math-54.pa" COMPILER_OPTIONS --compiler-inlining=false)
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/math-55.pa" COMPILER_OPTIONS --compiler-inlining=false)
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/math-56.pa" COMPILER_OPTIONS --compiler-inlining=false)
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/math-57.pa" COMPILER_OPTIONS --compiler-inlining=false)
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/math-58.pa" RUNTIME_OPTIONS --compiler-inlining=false)
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/math-59.pa" RUNTIME_OPTIONS --compiler-inlining=false)
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/math-60.pa" RUNTIME_OPTIONS --compiler-inlining=false)
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/math-61.pa" RUNTIME_OPTIONS --compiler-inlining=false)
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/math-62.pa" RUNTIME_OPTIONS --compiler-inlining=false)
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/math-63.pa" RUNTIME_OPTIONS --compiler-inlining=false)
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/math-64.pa" RUNTIME_OPTIONS --compiler-inlining=false)
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/math-65.pa" RUNTIME_OPTIONS --compiler-inlining=false)
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/math-66.pa" RUNTIME_OPTIONS --compiler-inlining=false)
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/math-67.pa" RUNTIME_OPTIONS --compiler-inlining=false)
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/math-68.pa" RUNTIME_OPTIONS --compiler-inlining=false)
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/op-01.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/op-02.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/op-03.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/op-04.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/op-05.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/op-06.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/op-07.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/op-08.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/op-09.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/op-10.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/op-11.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/op-12.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/op-13.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/op-14.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/op-15.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/op-16.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/op-17.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/op-18.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/op-19.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/op-20.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/op-21.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/op-22.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/op-25.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/op-26.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/op-jeq-obj.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/op-jne-obj.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/op-jeqz-obj.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/op-jnez-obj.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/literalarrays-01.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/literalarrays-02.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/literalarrays-03.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/literalarrays-04.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/literalarrays-05.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/arrays-01.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/arrays-02.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/arrays-03.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/arrays-04.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/arrays-05.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/arrays-06.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/arrays-07.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/arrays-08.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/arrays-09.pa" VERIFIER_FAIL_TEST VERIFIER_OPTIONS "--log-level=warning" SEARCH_STDERR "Verifier warning 4" SKIP_INTERPRETER_IRTOC)
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/arrays-10.pa" VERIFIER_FAIL_TEST VERIFIER_OPTIONS "--log-level=warning" SEARCH_STDERR "Verifier warning 4" SKIP_INTERPRETER_IRTOC)
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/arrays-11.pa" SKIP_INTERPRETER_IRTOC)
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/arrays-12.pa" EXPECTED_STDOUT "idx = 2; length = 1" SKIP_INTERPRETER_IRTOC)
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/arrays-13.pa" EXPECTED_STDOUT "idx = 2; length = 1" SKIP_INTERPRETER_IRTOC)
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/arrays-14.pa" EXPECTED_STDOUT "size = -1" SKIP_INTERPRETER_IRTOC)
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/exceptions-02.pa" SKIP_INTERPRETER_IRTOC)
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/exceptions-03.pa" SKIP_INTERPRETER_IRTOC)
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/exceptions-04.pa" SKIP_INTERPRETER_IRTOC)
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/far-jump-01.pa" COMPILER_OPTIONS --compiler-max-bytecode-size=40000)
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/far-jump-02.pa" COMPILER_OPTIONS --compiler-max-bytecode-size=40000)
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/far-jump-03.pa" COMPILER_OPTIONS --compiler-max-bytecode-size=40000)
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/far-jump-04.pa" COMPILER_OPTIONS --compiler-max-bytecode-size=40000)
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/far-jump-05.pa" COMPILER_OPTIONS --compiler-max-bytecode-size=40000)
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/far-jump-06.pa" COMPILER_OPTIONS --compiler-max-bytecode-size=40000)
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/far-jump-07.pa" COMPILER_OPTIONS --compiler-max-bytecode-size=40000)
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/far-jump-08.pa" COMPILER_OPTIONS --compiler-max-bytecode-size=40000)
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/far-jump-09.pa" COMPILER_OPTIONS --compiler-max-bytecode-size=40000)
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/far-jump-10.pa" COMPILER_OPTIONS --compiler-max-bytecode-size=40000)
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/far-jump-11.pa" COMPILER_OPTIONS --compiler-max-bytecode-size=40000)
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/far-jump-12.pa" COMPILER_OPTIONS --compiler-max-bytecode-size=40000)
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/far-jump-13.pa" COMPILER_OPTIONS --compiler-max-bytecode-size=40000)
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/far-jump-14.pa" COMPILER_OPTIONS --compiler-max-bytecode-size=40000)
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/far-jump-15.pa" COMPILER_OPTIONS --compiler-max-bytecode-size=40000)
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/far-jump-16.pa" COMPILER_OPTIONS --compiler-max-bytecode-size=40000)
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/far-jump-17.pa" COMPILER_OPTIONS --compiler-max-bytecode-size=40000)
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/far-jump-18.pa" COMPILER_OPTIONS --compiler-max-bytecode-size=40000)
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/far-jump-19.pa" COMPILER_OPTIONS --compiler-max-bytecode-size=40000)
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/obj-01.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/obj-02.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/obj-03.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/obj-04.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/obj-05.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/obj-06.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/obj-07.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/obj-08.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/obj-09.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/obj-10.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/obj-11.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/obj-12.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/obj-13.pa" SKIP_CROSS "arm32")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/obj-14.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/obj-15.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/obj-16.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/obj-17.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/obj-18.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/obj-23.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/obj-24.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/obj-25.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/obj-26.pa" VERIFIER_FAIL_TEST VERIFIER_OPTIONS "--log-level=warning" SEARCH_STDERR "Verifier warning 4" SKIP_INTERPRETER_IRTOC)
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/obj-27.pa" VERIFIER_FAIL_TEST VERIFIER_OPTIONS "--log-level=warning" SEARCH_STDERR "Verifier warning 4" SKIP_INTERPRETER_IRTOC)
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/obj-28.pa" EXPECTED_STDOUT "R1 cannot be cast to R2" VERIFIER_OPTIONS "--log-level=warning" SEARCH_STDERR "Possibly incompatible accumulator type" SKIP_INTERPRETER_IRTOC)
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/obj-29.pa" SKIP_INTERPRETER_IRTOC)
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/obj-30.pa" EXPECTED_STDOUT "abstract method \"R.foo\"" SKIP_INTERPRETER_IRTOC)
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/obj-31.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/obj-32.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/obj-33.pa" SKIP_CROSS "arm32" SKIP_INTERPRETER_IRTOC)
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/obj-34.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/obj-35.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/obj-36.pa" VERIFIER_OPTIONS "--log-level=warning" SEARCH_STDERR "Possibly incompatible accumulator type" SKIP_INTERPRETER_IRTOC)

add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/intrinsics-01.pa" SKIP_VERIFICATION)
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/intrinsics-02.pa" SKIP_VERIFICATION)
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/intrinsics-03.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/intrinsics-04.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/intrinsics-05.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/intrinsics-06.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/intrinsics-07.pa" SKIP_VERIFICATION)
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/intrinsics-08.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/intrinsics-09.pa" SKIP_VERIFICATION)
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/intrinsics-10.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/intrinsics-11.pa" SKIP_VERIFICATION)
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/intrinsics-12.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/intrinsics-13.pa" EXPECTED_STDOUT "23")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/intrinsics-14.pa" EXPECTED_STDOUT "24")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/intrinsics-15.pa" EXPECTED_STDOUT "26.6" SKIP_VERIFICATION)
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/intrinsics-16.pa" EXPECTED_STDOUT "26.6")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/intrinsics-17.pa" EXPECTED_STDOUT "27")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/intrinsics-18.pa" EXPECTED_STDOUT "28")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/intrinsics-19.pa" EXPECTED_STDOUT "asdf\n")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/intrinsics-20.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/intrinsics-21.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/intrinsics-22.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/intrinsics-23.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/intrinsics-24.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/intrinsics-25.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/intrinsics-26.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/intrinsics-28.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/intrinsics-29.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/intrinsics-54.pa" PRLIMIT_OPTIONS "${prlimit_options}")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/intrinsics-55.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/intrinsics-f32-01.pa")

add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/initobj-01.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/initobj-02.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/initobj-03.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/initobj-04.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/initobj-05.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/initobj-06.pa")

add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/initobj-bad-02.pa" SKIP_VERIFICATION SKIP_INTERPRETER_IRTOC)

add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/big_ark_option_value.pa" RUNTIME_OPTIONS --code-cache-size-limit=3221225472)

add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/oom-01.pa" COMPILER_OPTIONS --compiler-hotness-threshold=0 --compiler-inlining=false RUNTIME_OPTIONS --heap-size-limit=13000000 SKIP_INTERPRETER_IRTOC)
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/oom-02.pa" RUNTIME_OPTIONS --heap-size-limit=13000000 SKIP_INTERPRETER_IRTOC)

add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/tst-branch.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/tst-compare.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/cts-assembly/tst-select.pa")

add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/regression/allocate-252-params.pa")

add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/regression/bitops-bitwise-and.pa" COMPILER_OPTIONS --compiler-print-stats)
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/regression/math-partial-sums.pa" COMPILER_OPTIONS --compiler-print-stats DISABLE_LIMIT_STD_ALLOC)
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/regression/fmod.pa" COMPILER_OPTIONS --compiler-inlining=false)
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/regression/stack-4-params.pa" COMPILER_OPTIONS --compiler-inlining=false VERIFIER_FAIL_TEST SEARCH_STDERR "Bad register type")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/regression/stack-9-params.pa" COMPILER_OPTIONS --compiler-inlining=false VERIFIER_FAIL_TEST SEARCH_STDERR "Bad register type")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/regression/fcmp.pa" VERIFIER_FAIL_TEST SEARCH_STDERR "Bad register type")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/regression/irreducible_loop.pa")
if (NOT PANDA_ENABLE_ADDRESS_SANITIZER)
#TODO(mwx851039): This test may exceed 400Mb stack in CI (#4061).
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/regression/loop-unroll.pa" COMPILER_OPTIONS --compiler-inlining=false)
endif()
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/regression/loop-peeling.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/regression/shr32.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/regression/deopt_false_test.pa" COMPILER_OPTIONS --compiler-inlining=false)
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/regression/deopt_true_test.pa" COMPILER_OPTIONS --compiler-inlining=false SKIP_INTERPRETER_IRTOC)
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/regression/deopt_true_call_test.pa" COMPILER_OPTIONS --compiler-inlining=true SKIP_INTERPRETER_IRTOC)

add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/irtoc-interpreter-tests/sum.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/irtoc-interpreter-tests/signed_imms.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/irtoc-interpreter-tests/array.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/irtoc-interpreter-tests/newobj.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/irtoc-interpreter-tests/static.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/irtoc-interpreter-tests/call.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/irtoc-interpreter-tests/call1.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/irtoc-interpreter-tests/call2.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/irtoc-interpreter-tests/call.acc.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/irtoc-interpreter-tests/initobj.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/irtoc-interpreter-tests/call.virt.pa")
# Lots of runtime calls with enabled '--verify-entrypoints' option cause lots of standard allocations
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/irtoc-interpreter-tests/barriers.pa" DISABLE_LIMIT_STD_ALLOC SKIP_CROSS "arm32")

add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/irtoc-interpreter-tests/irtoc-opcode-suite/ldai.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/irtoc-interpreter-tests/irtoc-opcode-suite/and.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/irtoc-interpreter-tests/irtoc-opcode-suite/andi.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/irtoc-interpreter-tests/irtoc-opcode-suite/or.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/irtoc-interpreter-tests/irtoc-opcode-suite/ori.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/irtoc-interpreter-tests/irtoc-opcode-suite/shr.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/irtoc-interpreter-tests/irtoc-opcode-suite/shri.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/irtoc-interpreter-tests/irtoc-opcode-suite/shl.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/irtoc-interpreter-tests/irtoc-opcode-suite/shli.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/irtoc-interpreter-tests/irtoc-opcode-suite/i32toi16.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/irtoc-interpreter-tests/irtoc-opcode-suite/i32toi8.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/irtoc-interpreter-tests/irtoc-opcode-suite/i64toi32.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/irtoc-interpreter-tests/irtoc-opcode-suite/u64toi32.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/irtoc-interpreter-tests/irtoc-opcode-suite/u64tou32.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/irtoc-interpreter-tests/irtoc-opcode-suite/i32toi64.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/irtoc-interpreter-tests/irtoc-opcode-suite/u32toi64.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/irtoc-interpreter-tests/irtoc-opcode-suite/i32tou1.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/irtoc-interpreter-tests/irtoc-opcode-suite/u32tou1.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/irtoc-interpreter-tests/irtoc-opcode-suite/i64tou1.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/irtoc-interpreter-tests/irtoc-opcode-suite/u64tou1.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/irtoc-interpreter-tests/irtoc-opcode-suite/i32tof32.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/irtoc-interpreter-tests/irtoc-opcode-suite/i32tof64.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/irtoc-interpreter-tests/irtoc-opcode-suite/u32tof32.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/irtoc-interpreter-tests/irtoc-opcode-suite/u32tof64.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/irtoc-interpreter-tests/irtoc-opcode-suite/i64tof32.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/irtoc-interpreter-tests/irtoc-opcode-suite/i64tof64.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/irtoc-interpreter-tests/irtoc-opcode-suite/u64tof32.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/irtoc-interpreter-tests/irtoc-opcode-suite/u64tof64.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/irtoc-interpreter-tests/irtoc-opcode-suite/i32tou16.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/irtoc-interpreter-tests/irtoc-opcode-suite/i32tou8.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/irtoc-interpreter-tests/irtoc-opcode-suite/u32toi16.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/irtoc-interpreter-tests/irtoc-opcode-suite/u32toi8.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/irtoc-interpreter-tests/irtoc-opcode-suite/u32tou16.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/irtoc-interpreter-tests/irtoc-opcode-suite/u32tou8.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/irtoc-interpreter-tests/irtoc-opcode-suite/fadd2.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/irtoc-interpreter-tests/irtoc-opcode-suite/fadd2.64.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/irtoc-interpreter-tests/irtoc-opcode-suite/fsub2.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/irtoc-interpreter-tests/irtoc-opcode-suite/fsub2.64.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/irtoc-interpreter-tests/irtoc-opcode-suite/fmul2.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/irtoc-interpreter-tests/irtoc-opcode-suite/fmul2.64.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/irtoc-interpreter-tests/irtoc-opcode-suite/fdiv2.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/irtoc-interpreter-tests/irtoc-opcode-suite/fdiv2.64.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/irtoc-interpreter-tests/irtoc-opcode-suite/fmod2.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/irtoc-interpreter-tests/irtoc-opcode-suite/fmod2.64.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/irtoc-interpreter-tests/irtoc-opcode-suite/fneg.pa")
add_test_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/irtoc-interpreter-tests/irtoc-opcode-suite/fneg.64.pa")

if (TARGET verifier-tests)
# Verifier regression tests
# Negative (must fail)
add_verifier_test(FILE "${CMAKE_CURRENT_SOURCE_DIR}/verifier-tests/cflow_err_beyond_end_of_function.pa" VERIFIER_FAIL_TEST SEARCH_STDERR "Invalid last instruction in method, execution beyond the method code boundary.")
add_verifier_test(FILE "${CMAKE_CURRENT_SOURCE_DIR}/verifier-tests/cflow_jump_on_exc_handler.pa" VERIFIER_FAIL_TEST SEARCH_STDERR "Cflow invalid jump into exc handler")
add_verifier_test(FILE "${CMAKE_CURRENT_SOURCE_DIR}/verifier-tests/cflow_jump_on_exc_handler_from_body.pa" VERIFIER_FAIL_TEST SEARCH_STDERR "Cflow invalid jump into exc handler")
add_verifier_test(FILE "${CMAKE_CURRENT_SOURCE_DIR}/verifier-tests/cflow_jump_out_of_body.pa" VERIFIER_FAIL_TEST SEARCH_STDERR "Jump offset is out of bounds of method body")
add_verifier_test(FILE "${CMAKE_CURRENT_SOURCE_DIR}/verifier-tests/cflow_execution_may_go_beyond_the_end.pa" VERIFIER_FAIL_TEST SEARCH_STDERR "Invalid last instruction in method, execution beyond the method code boundary.")
add_verifier_test(FILE "${CMAKE_CURRENT_SOURCE_DIR}/verifier-tests/cflow_fallthrough_from_exc_handler_to_exc_handler.pa" VERIFIER_FAIL_TEST SEARCH_STDERR "Cflow body fall into exc handler")
add_verifier_test(FILE "${CMAKE_CURRENT_SOURCE_DIR}/verifier-tests/cflow_fallthrough_to_exc_handler_from_body.pa" VERIFIER_FAIL_TEST SEARCH_STDERR "Cflow body fall into exc handler")
add_verifier_test(FILE "${CMAKE_CURRENT_SOURCE_DIR}/verifier-tests/cflow_jump_from_exc_handler_to_exc_handler.pa" VERIFIER_FAIL_TEST SEARCH_STDERR "Cflow invalid jump into exc handler")
add_verifier_test(FILE "${CMAKE_CURRENT_SOURCE_DIR}/verifier-tests/wrong_accumulator_position.pa" VERIFIER_FAIL_TEST SEARCH_STDERR "Accumulator position is out of range")
# regression
add_verifier_test(FILE "${CMAKE_CURRENT_SOURCE_DIR}/verifier-tests/bug_1926.pa" VERIFIER_FAIL_TEST SEARCH_STDERR "Bad call")
add_verifier_test(FILE "${CMAKE_CURRENT_SOURCE_DIR}/verifier-tests/bug_1940.pa" VERIFIER_FAIL_TEST SEARCH_STDERR "Bad call undefined register")
add_verifier_test(FILE "${CMAKE_CURRENT_SOURCE_DIR}/verifier-tests/bug_1863.pa")
add_verifier_test(FILE "${CMAKE_CURRENT_SOURCE_DIR}/verifier-tests/bug_1745.pa")
add_verifier_test(FILE "${CMAKE_CURRENT_SOURCE_DIR}/verifier-tests/issue_1163.pa" VERIFIER_FAIL_TEST SEARCH_STDERR "Bad register type")
add_verifier_test(FILE "${CMAKE_CURRENT_SOURCE_DIR}/verifier-tests/bug_2084.pa" VERIFIER_FAIL_TEST SEARCH_STDERR "Undefined register")
add_verifier_test(FILE "${CMAKE_CURRENT_SOURCE_DIR}/verifier-tests/issue_1981.pa")
add_verifier_test(FILE "${CMAKE_CURRENT_SOURCE_DIR}/verifier-tests/bug_1697.pa" VERIFIER_FAIL_TEST SEARCH_STDERR "Bad register type")
add_verifier_test(FILE "${CMAKE_CURRENT_SOURCE_DIR}/verifier-tests/bug_1702.pa" VERIFIER_FAIL_TEST SEARCH_STDERR "Undefined register")
add_verifier_test(FILE "${CMAKE_CURRENT_SOURCE_DIR}/verifier-tests/bug_1813.pa" VERIFIER_FAIL_TEST SEARCH_STDERR "Bad register type")
add_verifier_test(FILE "${CMAKE_CURRENT_SOURCE_DIR}/verifier-tests/bug_1826.pa" VERIFIER_FAIL_TEST SEARCH_STDERR "Inconsistent register and field types")
add_verifier_test(FILE "${CMAKE_CURRENT_SOURCE_DIR}/verifier-tests/bug_1827.pa" VERIFIER_FAIL_TEST SEARCH_STDERR "Inconsistent register and field types")
add_verifier_test(FILE "${CMAKE_CURRENT_SOURCE_DIR}/verifier-tests/bug_1834.pa" VERIFIER_FAIL_TEST SEARCH_STDERR "Bad register type")
add_verifier_test(FILE "${CMAKE_CURRENT_SOURCE_DIR}/verifier-tests/bug_1828.pa" VERIFIER_FAIL_TEST SEARCH_STDERR "Expected instance field")
add_verifier_test(FILE "${CMAKE_CURRENT_SOURCE_DIR}/verifier-tests/bug_2072.pa" VERIFIER_FAIL_TEST SEARCH_STDERR "Bad register type")
add_verifier_test(FILE "${CMAKE_CURRENT_SOURCE_DIR}/verifier-tests/bug_2075.pa" VERIFIER_FAIL_TEST SEARCH_STDERR "Bad register type")
add_verifier_test(FILE "${CMAKE_CURRENT_SOURCE_DIR}/verifier-tests/bug_2085.pa" VERIFIER_FAIL_TEST SEARCH_STDERR "Inconsistent register and field types")
add_verifier_test(FILE "${CMAKE_CURRENT_SOURCE_DIR}/verifier-tests/bug_2086_1.pa" VERIFIER_FAIL_TEST SEARCH_STDERR "Expected instance field")
add_verifier_test(FILE "${CMAKE_CURRENT_SOURCE_DIR}/verifier-tests/bug_2086_2.pa" VERIFIER_FAIL_TEST SEARCH_STDERR "Inconsistent register and field types")
add_verifier_test(FILE "${CMAKE_CURRENT_SOURCE_DIR}/verifier-tests/bug_2088.pa" VERIFIER_FAIL_TEST SEARCH_STDERR "Unexpected field type")
add_verifier_test(FILE "${CMAKE_CURRENT_SOURCE_DIR}/verifier-tests/bug_2090.pa" VERIFIER_FAIL_TEST SEARCH_STDERR "Bad register type")
add_verifier_test(FILE "${CMAKE_CURRENT_SOURCE_DIR}/verifier-tests/bug_2107_1.pa" VERIFIER_FAIL_TEST SEARCH_STDERR "Bad register type")
add_verifier_test(FILE "${CMAKE_CURRENT_SOURCE_DIR}/verifier-tests/bug_2107_2.pa" VERIFIER_FAIL_TEST SEARCH_STDERR "Bad register type")
add_verifier_test(FILE "${CMAKE_CURRENT_SOURCE_DIR}/verifier-tests/bug_2136.pa")
add_verifier_test(FILE "${CMAKE_CURRENT_SOURCE_DIR}/verifier-tests/bug_2256.pa" VERIFIER_FAIL_TEST SEARCH_STDERR "Bad call")
add_verifier_test(FILE "${CMAKE_CURRENT_SOURCE_DIR}/verifier-tests/bug_2260.pa" VERIFIER_FAIL_TEST SEARCH_STDERR "Bad register type")
add_verifier_test(FILE "${CMAKE_CURRENT_SOURCE_DIR}/verifier-tests/bug_2374.pa" VERIFIER_FAIL_TEST SEARCH_STDERR "Bad call static method as virtual")
add_verifier_test(FILE "${CMAKE_CURRENT_SOURCE_DIR}/verifier-tests/issue_964.pa" VERIFIER_FAIL_TEST SEARCH_STDERR "Bad register type")
add_verifier_test(FILE "${CMAKE_CURRENT_SOURCE_DIR}/verifier-tests/bug_1833.pa" VERIFIER_FAIL_TEST SEARCH_STDERR "Inconsistent register and field types")
add_verifier_test(FILE "${CMAKE_CURRENT_SOURCE_DIR}/verifier-tests/jumps_1.pa" VERIFIER_FAIL_TEST SEARCH_STDERR "Register type conflict")
add_verifier_test(FILE "${CMAKE_CURRENT_SOURCE_DIR}/verifier-tests/jumps_2.pa" VERIFIER_FAIL_TEST SEARCH_STDERR "Bad register type")
add_verifier_test(FILE "${CMAKE_CURRENT_SOURCE_DIR}/verifier-tests/jumps_3.pa")
add_verifier_test(FILE "${CMAKE_CURRENT_SOURCE_DIR}/verifier-tests/bug_2089.pa" VERIFIER_FAIL_TEST SEARCH_STDERR "Bad register type")
add_verifier_test(FILE "${CMAKE_CURRENT_SOURCE_DIR}/verifier-tests/bug_2702_1.pa")
add_verifier_test(FILE "${CMAKE_CURRENT_SOURCE_DIR}/verifier-tests/bug_2702_2.pa")
add_verifier_test(FILE "${CMAKE_CURRENT_SOURCE_DIR}/verifier-tests/bug_2702_3.pa")
add_verifier_test(FILE "${CMAKE_CURRENT_SOURCE_DIR}/verifier-tests/bug_2702_4.pa")
add_verifier_test(FILE "${CMAKE_CURRENT_SOURCE_DIR}/verifier-tests/bug_2740.pa" VERIFIER_FAIL_TEST SEARCH_STDERR "Bad register type")
add_verifier_test(FILE "${CMAKE_CURRENT_SOURCE_DIR}/verifier-tests/bug_2787.pa" VERIFIER_FAIL_TEST SEARCH_STDERR "Undefined register")
add_verifier_test(FILE "${CMAKE_CURRENT_SOURCE_DIR}/verifier-tests/bug_2816.pa" VERIFIER_FAIL_TEST SEARCH_STDERR "Bad register type")
add_verifier_test(FILE "${CMAKE_CURRENT_SOURCE_DIR}/verifier-tests/bug_2817.pa" VERIFIER_FAIL_TEST SEARCH_STDERR "Bad register type")
add_verifier_test(FILE "${CMAKE_CURRENT_SOURCE_DIR}/verifier-tests/bug_2818_1.pa" VERIFIER_FAIL_TEST SEARCH_STDERR "Undefined register")
add_verifier_test(FILE "${CMAKE_CURRENT_SOURCE_DIR}/verifier-tests/bug_2818_2.pa" VERIFIER_FAIL_TEST SEARCH_STDERR "Undefined register")
add_verifier_test(FILE "${CMAKE_CURRENT_SOURCE_DIR}/verifier-tests/bug_2921.pa")
add_verifier_test(FILE "${CMAKE_CURRENT_SOURCE_DIR}/verifier-tests/bug_3060.pa")
add_verifier_test(FILE "${CMAKE_CURRENT_SOURCE_DIR}/verifier-tests/bug_3133.pa" VERIFIER_FAIL_TEST SEARCH_STDERR "Bad register type")
add_verifier_test(FILE "${CMAKE_CURRENT_SOURCE_DIR}/verifier-tests/bug_3197.pa" VERIFIER_FAIL_TEST VERIFIER_OPTIONS "--log-level=info" SEARCH_STDERR "lways NPE")
add_verifier_test(FILE "${CMAKE_CURRENT_SOURCE_DIR}/verifier-tests/bug_3219.pa")
add_verifier_test(FILE "${CMAKE_CURRENT_SOURCE_DIR}/verifier-tests/bug_3228.pa" VERIFIER_FAIL_TEST VERIFIER_OPTIONS "--log-level=info" SEARCH_STDERR "lways NPE")
add_verifier_test(FILE "${CMAKE_CURRENT_SOURCE_DIR}/verifier-tests/mr_3176.pa" VERIFIER_OPTIONS "--methods=_GLOBAL::main")
add_verifier_test(FILE "${CMAKE_CURRENT_SOURCE_DIR}/verifier-tests/initobj-bad-03.pa" VERIFIER_FAIL_TEST SEARCH_STDERR "Initobj calls not constructor")
add_verifier_test(FILE "${CMAKE_CURRENT_SOURCE_DIR}/verifier-tests/access_field_nomodifier_core.pa")
add_verifier_test(FILE "${CMAKE_CURRENT_SOURCE_DIR}/verifier-tests/exception_type.pa")
endif()
