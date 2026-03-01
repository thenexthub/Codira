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

set(LOOKUP_BY_NAME_TEST_LIST
    "correct_case_8bit"
    "correct_case_16bit"
    "correct_case_32bit"
    "correct_case_64bit"
    "correct_case_ref"
    # enable after fix #23965
    # "correct_case_ref_2"
    # "no_field_case"
    # "no_field_case_2"
    # "no_getter_case"
    # "no_setter_case"
    # "wrong_setter_args_case"
    # "wrong_setter_args_case_2"
    # "wrong_setter_args_case_3"
    # "wrong_setter_args_case_4"
    # "wrong_getter_args_case"
    # "wrong_getter_args_case_2"
    "field_case_8bit"
    "field_case_32bit"
    "field_case_64bit"
    "field_case_ref"
)
add_custom_target(ets_tests_lookup_by_name)
add_dependencies(ets_tests ets_tests_lookup_by_name)

foreach(NAME ${LOOKUP_BY_NAME_TEST_LIST})
    panda_add_asm_file(
        FILE ${PANDA_ETS_PLUGIN_SOURCE}/tests/lookup_by_name/${NAME}.pa
        TARGET ets_lookup_by_name_${NAME}
        DEPENDS etsstdlib
        LANGUAGE_CONTEXT ets
        ADDITIONAL_STDLIBS $<TARGET_PROPERTY:etsstdlib,FILE>
        ASSEMBLER_OPTIONS "--optimize"
    )
    add_dependencies(ets_tests_lookup_by_name ets_lookup_by_name_${NAME})
    if(PANDA_COMPILER_ENABLE AND NOT PANDA_TARGET_ARM32)

        # 26982 64-bit pointer support
        if (PANDA_32_BIT_MANAGED_POINTER OR NOT PANDA_TARGET_ARM64)
            panda_add_asm_file(
                FILE ${PANDA_ETS_PLUGIN_SOURCE}/tests/lookup_by_name/${NAME}.pa
                TARGET ets_lookup_by_name_${NAME}_enforce_jit
                DEPENDS etsstdlib
                LANGUAGE_CONTEXT ets
                ADDITIONAL_STDLIBS $<TARGET_PROPERTY:etsstdlib,FILE>
                ASSEMBLER_OPTIONS "--optimize"
                RUNTIME_OPTIONS "--compiler-enable-jit=true" "--compiler-hotness-threshold=0" "--no-async-jit"
            )
            add_dependencies(ets_tests_lookup_by_name ets_lookup_by_name_${NAME}_enforce_jit)
        endif()

        if (PANDA_TARGET_ARM64 OR PANDA_TARGET_AMD64)
            panda_add_asm_file(
                FILE ${PANDA_ETS_PLUGIN_SOURCE}/tests/lookup_by_name/${NAME}.pa
                TARGET ets_lookup_by_name_${NAME}_aot
                DEPENDS etsstdlib
                LANGUAGE_CONTEXT ets
                AOT_MODE TRUE
                ADDITIONAL_STDLIBS $<TARGET_PROPERTY:etsstdlib,FILE>
                ASSEMBLER_OPTIONS "--optimize"
                RUNTIME_OPTIONS "--compiler-enable-jit=true"
            )
            add_dependencies(ets_tests_lookup_by_name ets_lookup_by_name_${NAME}_aot)

            if (PANDA_LLVM_AOT)
                panda_add_asm_file(
                    FILE ${PANDA_ETS_PLUGIN_SOURCE}/tests/lookup_by_name/${NAME}.pa
                    TARGET ets_lookup_by_name_${NAME}_llvmaot
                    DEPENDS etsstdlib
                    LANGUAGE_CONTEXT ets
                    AOT_MODE TRUE
                    AOT_COMPILER "llvm"
                    ADDITIONAL_STDLIBS $<TARGET_PROPERTY:etsstdlib,FILE>
                    ASSEMBLER_OPTIONS "--optimize"
                    RUNTIME_OPTIONS "--compiler-enable-jit=true"
                )
                add_dependencies(ets_tests_lookup_by_name ets_lookup_by_name_${NAME}_llvmaot)
            endif()
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
        FILE ${PANDA_ETS_PLUGIN_SOURCE}/tests/lookup_by_name/${ARG_FILE}.pa
        TARGET ets_lookup_by_name_${ARG_FILE}-verify
        ${VERIFIER_FAIL_TEST}
        SEARCH_STDERR "${ARG_SEARCH_STDERR}"
        ERROR_FILE_VARIABLE error_file
        DEPENDS etsstdlib
        LANGUAGE_CONTEXT ets
        STDLIBS $<TARGET_PROPERTY:etsstdlib,FILE>
    )
    add_dependencies(ets_tests_lookup_by_name_verify ets_lookup_by_name_${ARG_FILE}-verify)

    if (DEFINED ARG_SEARCH_STDERR AND NOT (CMAKE_BUILD_TYPE MATCHES Release))
        add_custom_target(ets_lookup_by_name_${ARG_FILE}-check-logmsg
                          COMMENT "Check ets_lookup_by_name_${ARG_FILE} log message"
                          COMMAND grep -zo \"${ARG_SEARCH_STDERR}\" ${error_file} >/dev/null
                          DEPENDS ets_lookup_by_name_${ARG_FILE}-verify)

    add_dependencies(ets_tests_lookup_by_name_verify ets_lookup_by_name_${ARG_FILE}-check-logmsg)
    endif()

endfunction()

add_custom_target(ets_tests_lookup_by_name_verify)
add_dependencies(ets_tests_lookup_by_name_verify verifier)
add_dependencies(ets_tests_lookup_by_name ets_tests_lookup_by_name_verify)

add_ets_verifier_test(FILE "correct_case_32bit")
add_ets_verifier_test(FILE "correct_case_64bit")
add_ets_verifier_test(FILE "correct_case_ref")
add_ets_verifier_test(FILE "field_case_8bit")
add_ets_verifier_test(FILE "field_case_32bit")
add_ets_verifier_test(FILE "field_case_64bit")
add_ets_verifier_test(FILE "field_case_ref")

# currently all union types are encoded as single one class “$UNION_FIELD_DUMMY_CLASS”
# at bytecode level, thus we do not have accurate union type info to verify each variables
# so the current temporary solution would be to skip verification for union types.This
# actually introduce insecure possibilities here. Accurate verification for union types
# will need redesign for union types support in the future
#
# based on the above,here we skip:
# 1. checking whether a field existed in the union or not
# 2. skip checking member access violiations
# enable after fix #23965
# add_ets_verifier_test(FILE "no_field_case")
# add_ets_verifier_test(FILE "no_field_case_2")
# add_ets_verifier_test(FILE "no_getter_case")
# add_ets_verifier_test(FILE "no_setter_case")
# add_ets_verifier_test(FILE "wrong_setter_args_case")
# add_ets_verifier_test(FILE "wrong_setter_args_case_2")
# add_ets_verifier_test(FILE "wrong_setter_args_case_3" VERIFIER_FAIL_TEST SEARCH_STDERR "Inconsistent register and field types")
# add_ets_verifier_test(FILE "wrong_setter_args_case_4")
# add_ets_verifier_test(FILE "wrong_getter_args_case")
# add_ets_verifier_test(FILE "wrong_getter_args_case_2")
