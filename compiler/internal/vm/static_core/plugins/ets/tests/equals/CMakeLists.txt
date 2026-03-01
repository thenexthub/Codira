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

set(EQUALS_TEST_LIST
    "null_null"
    "null_str"
    "null_undef"
    "undef_str"
    "undef_undef"
    "str_str_correct"
    "str_str_invalid"
    "num_num_correct"
    "num_num_invalid"
    "num_long_correct"
    "num_long_invalid"
    "num_int_invalid"
    "num_int_correct"
)

add_custom_target(ets_tests_equals)
add_dependencies(ets_tests ets_tests_equals)

foreach(NAME ${EQUALS_TEST_LIST})
    panda_add_asm_file(
        FILE ${PANDA_ETS_PLUGIN_SOURCE}/tests/equals/${NAME}.pa
        TARGET ets_equals_${NAME}_cpp
        DEPENDS etsstdlib
        LANGUAGE_CONTEXT ets
        ADDITIONAL_STDLIBS $<TARGET_PROPERTY:etsstdlib,FILE>
        RUNTIME_OPTIONS "--interpreter-type=cpp"
    )
    add_dependencies(ets_tests_equals ets_equals_${NAME}_cpp)

    panda_add_asm_file(
        FILE ${PANDA_ETS_PLUGIN_SOURCE}/tests/equals/${NAME}.pa
        TARGET ets_equals_${NAME}_irtoc
        DEPENDS etsstdlib
        LANGUAGE_CONTEXT ets
        ADDITIONAL_STDLIBS $<TARGET_PROPERTY:etsstdlib,FILE>
        RUNTIME_OPTIONS "--interpreter-type=irtoc"
    )
    add_dependencies(ets_tests_equals ets_equals_${NAME}_irtoc)

    panda_add_asm_file(
        FILE ${PANDA_ETS_PLUGIN_SOURCE}/tests/equals/${NAME}.pa
        TARGET ets_equals_${NAME}_cpp_opt
        DEPENDS etsstdlib
        LANGUAGE_CONTEXT ets
        ADDITIONAL_STDLIBS $<TARGET_PROPERTY:etsstdlib,FILE>
        ASSEMBLER_OPTIONS "--optimize"
        RUNTIME_OPTIONS "--interpreter-type=cpp"
    )
    add_dependencies(ets_tests_equals ets_equals_${NAME}_cpp_opt)

    panda_add_asm_file(
        FILE ${PANDA_ETS_PLUGIN_SOURCE}/tests/equals/${NAME}.pa
        TARGET ets_equals_${NAME}_irtoc_opt
        DEPENDS etsstdlib
        LANGUAGE_CONTEXT ets
        ADDITIONAL_STDLIBS $<TARGET_PROPERTY:etsstdlib,FILE>
        ASSEMBLER_OPTIONS "--optimize"
        RUNTIME_OPTIONS "--interpreter-type=irtoc"
    )
    add_dependencies(ets_tests_equals ets_equals_${NAME}_irtoc_opt)

    if(PANDA_COMPILER_ENABLE AND NOT PANDA_TARGET_ARM32)

        # 26982 64-bit pointer support
        if (PANDA_32_BIT_MANAGED_POINTER OR NOT PANDA_TARGET_ARM64)
            panda_add_asm_file(
                FILE ${PANDA_ETS_PLUGIN_SOURCE}/tests/equals/${NAME}.pa
                TARGET ets_equals_${NAME}_enforce_jit
                DEPENDS etsstdlib
                LANGUAGE_CONTEXT ets
                ADDITIONAL_STDLIBS $<TARGET_PROPERTY:etsstdlib,FILE>
                ASSEMBLER_OPTIONS "--optimize"
                RUNTIME_OPTIONS "--compiler-enable-jit=true" "--compiler-hotness-threshold=0" "--no-async-jit"
            )
            add_dependencies(ets_tests_equals ets_equals_${NAME}_enforce_jit)
        endif()

        if (PANDA_TARGET_ARM64 OR PANDA_TARGET_AMD64)
            panda_add_asm_file(
                FILE ${PANDA_ETS_PLUGIN_SOURCE}/tests/equals/${NAME}.pa
                TARGET ets_equals_${NAME}_aot
                DEPENDS etsstdlib
                LANGUAGE_CONTEXT ets
                AOT_MODE TRUE
                ADDITIONAL_STDLIBS $<TARGET_PROPERTY:etsstdlib,FILE>
                ASSEMBLER_OPTIONS "--optimize"
                RUNTIME_OPTIONS "--compiler-enable-jit=true"
            )
            add_dependencies(ets_tests_equals ets_equals_${NAME}_aot)
        endif()
    endif()
endforeach()

function(add_ets_verifier_test)
    set(prefix ARG)
    set(noValues VERIFIER_FAIL_TEST)
    set(singleValues FILE)
    set(multiValues SEARCH_STDERR)
    cmake_parse_arguments(${prefix}
                        "${noValues}"
                        "${singleValues}"
                        "${multiValues}"
                        ${ARGN})

    if (ARG_VERIFIER_FAIL_TEST)
        set(VERIFIER_FAIL_TEST VERIFIER_FAIL_TEST)
    else()
        set(VERIFIER_FAIL_TEST)
    endif()

    set(error_file)

    verifier_add_asm_file(
        FILE ${PANDA_ETS_PLUGIN_SOURCE}/tests/equals/${ARG_FILE}.pa
        TARGET ets_equals_${ARG_FILE}-verify
        ${VERIFIER_FAIL_TEST}
        SEARCH_STDERR "${ARG_SEARCH_STDERR}"
        ERROR_FILE_VARIABLE error_file
        DEPENDS etsstdlib
        LANGUAGE_CONTEXT ets
        STDLIBS $<TARGET_PROPERTY:etsstdlib,FILE>
    )
    add_dependencies(ets_tests_equals_verify ets_equals_${ARG_FILE}-verify)

    if (DEFINED ARG_SEARCH_STDERR AND NOT (CMAKE_BUILD_TYPE MATCHES Release))
        add_custom_target(ets_equals_${ARG_FILE}-check-logmsg
                          COMMENT "Check ets_equals_${ARG_FILE} log message"
                          COMMAND grep -zo \"${ARG_SEARCH_STDERR}\" ${error_file} >/dev/null
                          DEPENDS ets_equals_${ARG_FILE}-verify)

    add_dependencies(ets_tests_equals_verify ets_equals_${ARG_FILE}-check-logmsg)
    endif()
endfunction()

add_custom_target(ets_tests_equals_verify)
add_dependencies(ets_tests_equals_verify verifier)
add_dependencies(ets_tests_equals ets_tests_equals_verify)

foreach(NAME ${EQUALS_TEST_LIST})
    add_ets_verifier_test(FILE ${NAME})
endforeach()
