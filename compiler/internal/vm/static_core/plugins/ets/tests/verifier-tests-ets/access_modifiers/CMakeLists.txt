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

set(verifier_tests_in_dir "${CMAKE_CURRENT_SOURCE_DIR}")
set(verifier_tests_out_dir "${CMAKE_CURRENT_BINARY_DIR}")

add_custom_target(ets_verify_access_modifiers)

# NOTE(mgroshev): Unit test for checking of access modifiers
panda_ets_add_gtest(
    NO_CORES
    NAME ets_verify_access_modifiers_tests
    SOURCES 
        access_modifiers_check_test.cpp
    LIBRARIES arkruntime arkassembler
    INCLUDE_DIRS ${PANDA_ETS_PLUGIN_SOURCE}/runtime
    SANITIZERS ${PANDA_SANITIZERS_LIST}
    PANDA_STD_LIB "${PANDA_BINARY_ROOT}/plugins/ets/etsstdlib.abc"
    DEPS_TARGETS etsstdlib
)
add_dependencies(ets_verify_access_modifiers
    ets_verify_access_modifiers_tests_gtests)

# NOTE(mgroshev): E2E tests for checking of access modifiers
function(add_ets_verifier_test)
    set(prefix ARG)
    set(noValues VERIFIER_FAIL_TEST)
    set(singleValues FILE)
    cmake_parse_arguments(${prefix}
                          "${noValues}"
                          "${singleValues}"
                          ""
                          ${ARGN})
    cmake_parse_arguments(PARSE_ARGV 3
                          ${prefix}
                          ""
                          "SEARCH_STDERR"
                          "")
    if (ARG_VERIFIER_FAIL_TEST)
        set(VERIFIER_FAIL_TEST VERIFIER_FAIL_TEST)
    else()
        set(VERIFIER_FAIL_TEST)
    endif()

    set(error_file)

    verifier_add_asm_file(
        FILE ${PANDA_ETS_PLUGIN_SOURCE}/tests/verifier-tests-ets/access_modifiers/${ARG_FILE}.pa
        TARGET verify-test-ets_${ARG_FILE}
        ${VERIFIER_FAIL_TEST}
        SEARCH_STDERR "${ARG_SEARCH_STDERR}"
        ERROR_FILE_VARIABLE error_file
        DEPENDS etsstdlib
        LANGUAGE_CONTEXT ets
        STDLIBS $<TARGET_PROPERTY:etsstdlib,FILE>
    )
    add_dependencies(ets_asm_verify verify-test-ets_${ARG_FILE})

    if (DEFINED ARG_SEARCH_STDERR AND NOT (CMAKE_BUILD_TYPE MATCHES Release))
        add_custom_target(verify-test-ets_${ARG_FILE}-check-logmsg
                          COMMENT "Check verify-test-ets_${ARG_FILE} log message"
                          COMMAND grep -zo \"${ARG_SEARCH_STDERR}\" ${error_file} >/dev/null
                          DEPENDS verify-test-ets_${ARG_FILE})

        add_dependencies(ets_asm_verify verify-test-ets_${ARG_FILE}-check-logmsg)
    endif()
endfunction()

add_custom_target(ets_asm_verify)
add_dependencies(ets_verify_access_modifiers ets_asm_verify)

add_ets_verifier_test(FILE "correct_call_private_method")
add_ets_verifier_test(FILE "neg_call_private_method" VERIFIER_FAIL_TEST SEARCH_STDERR "Call to a private method is prohibited.")
add_ets_verifier_test(FILE "correct_access_private_field")
add_ets_verifier_test(FILE "neg_access_private_field" VERIFIER_FAIL_TEST SEARCH_STDERR "Access to private object field is prohibited.")
add_ets_verifier_test(FILE "correct_access_protected_field")
add_ets_verifier_test(FILE "neg_access_protected_field" VERIFIER_FAIL_TEST SEARCH_STDERR "Access to protected object field is prohibited.")
# NOTE(@mgroshev): enable this test, when 30901
# add_ets_verifier_test(FILE "neg_call_private_ctor" VERIFIER_FAIL_TEST SEARCH_STDERR "Call to a private method is prohibited.")

