# Copyright (c) 2026 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

set(verifier_tests_in_dir "${CMAKE_CURRENT_SOURCE_DIR}")
set(verifier_tests_out_dir "${CMAKE_CURRENT_BINARY_DIR}")

add_custom_target(ets_verify_call_name)

# NOTE(mgroshev): Unit test for checking of ets_call_name handlers
panda_ets_add_gtest(
    NO_CORES
    NAME ets_verify_call_name_tests
    SOURCES 
        ets_call_name_check_test.cpp
    LIBRARIES arkruntime arkassembler
    INCLUDE_DIRS ${PANDA_ETS_PLUGIN_SOURCE}/runtime
    SANITIZERS ${PANDA_SANITIZERS_LIST}
    PANDA_STD_LIB "${PANDA_BINARY_ROOT}/plugins/ets/etsstdlib.abc"
    DEPS_TARGETS etsstdlib
)
add_dependencies(ets_verify_call_name
    ets_verify_call_name_tests_gtests)

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
        FILE ${PANDA_ETS_PLUGIN_SOURCE}/tests/verifier-tests-ets/ets_call_name/${ARG_FILE}.pa
        TARGET verify-test-ets_${ARG_FILE}
        ${VERIFIER_FAIL_TEST}
        SEARCH_STDERR "${ARG_SEARCH_STDERR}"
        ERROR_FILE_VARIABLE error_file
        DEPENDS etsstdlib
        LANGUAGE_CONTEXT ets
        STDLIBS $<TARGET_PROPERTY:etsstdlib,FILE>
    )
    add_dependencies(ets_asm_verify_ets_call_name verify-test-ets_${ARG_FILE})

    if (DEFINED ARG_SEARCH_STDERR AND NOT (CMAKE_BUILD_TYPE MATCHES Release))
        add_custom_target(verify-test-ets_${ARG_FILE}-check-logmsg
                          COMMENT "Check verify-test-ets_${ARG_FILE} log message"
                          COMMAND grep -zo \"${ARG_SEARCH_STDERR}\" ${error_file} >/dev/null
                          DEPENDS verify-test-ets_${ARG_FILE})

        add_dependencies(ets_asm_verify_ets_call_name verify-test-ets_${ARG_FILE}-check-logmsg)
    endif()
endfunction()

add_custom_target(ets_asm_verify_ets_call_name)
add_dependencies(ets_verify_call_name ets_asm_verify_ets_call_name)

add_ets_verifier_test(FILE "neg_byname_args" VERIFIER_FAIL_TEST SEARCH_STDERR "Verifier error 31: Bad call incompatible parameter")
add_ets_verifier_test(FILE "neg_byname_return" VERIFIER_FAIL_TEST SEARCH_STDERR "Verifier error 31: Bad call incompatible parameter")