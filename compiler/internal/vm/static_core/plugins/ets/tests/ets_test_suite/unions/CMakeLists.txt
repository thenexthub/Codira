# Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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


set(unions_tests
    unions_general
    unions_assignable
)

set(unions_tests_in_dir "${CMAKE_CURRENT_SOURCE_DIR}")
set(unions_tests_out_dir "${CMAKE_CURRENT_BINARY_DIR}")

add_custom_target(ets_test_suite_unions)

foreach(test ${unions_tests})
    set(test_out_dir "${unions_tests_out_dir}/${test}")

    set(test_in "${unions_tests_in_dir}/${test}.ets")
    set(target ets_test_suite_unions_${test})

    if (PANDA_TARGET_ARM32)
        # failure in regalloc, see #13413
        set(extra_options RUNTIME_EXTRA_OPTIONS "--compiler-inlining-blacklist=std.core.ETSGLOBAL::copyTo,std.core.String::codePointAt" "--compiler-regex='(?!std.core.ETSGLOBAL::copyTo)&(?!std.core.String::codePointAt).*'" "--compiler-ignore-failures=true")
    endif()

    run_int_jit_aot_ets_code(${test_in} ${test_out_dir} ${target} ${extra_options})
    add_dependencies(ets_test_suite_unions
        ${target}-ets-jit
        ${target}-ets-int)
    if (NOT CMAKE_CROSSCOMPILING)
        add_dependencies(ets_test_suite_unions ${target}-ets-aot)
    endif()
endforeach()

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
        FILE ${PANDA_ETS_PLUGIN_SOURCE}/tests/ets_test_suite/unions/${ARG_FILE}.pa
        TARGET unions_${ARG_FILE}-verify
        ${VERIFIER_FAIL_TEST}
        SEARCH_STDERR "${ARG_SEARCH_STDERR}"
        ERROR_FILE_VARIABLE error_file
        DEPENDS etsstdlib
        LANGUAGE_CONTEXT ets
        STDLIBS $<TARGET_PROPERTY:etsstdlib,FILE>
    )
    add_dependencies(ets_union_asm_verify unions_${ARG_FILE}-verify)

    if (DEFINED ARG_SEARCH_STDERR AND NOT (CMAKE_BUILD_TYPE MATCHES Release))
        add_custom_target(unions_${ARG_FILE}-check-logmsg
                          COMMENT "Check unions_${ARG_FILE} log message"
                          COMMAND grep -zo \"${ARG_SEARCH_STDERR}\" ${error_file} >/dev/null
                          DEPENDS unions_${ARG_FILE}-verify)

        add_dependencies(ets_union_asm_verify unions_${ARG_FILE}-check-logmsg)
    endif()
endfunction()

add_custom_target(ets_union_asm_verify)
add_dependencies(ets_union_asm_verify verifier)
add_dependencies(ets_test_suite_unions ets_union_asm_verify)

add_ets_verifier_test(FILE "correct_union_arg")
add_ets_verifier_test(FILE "correct_union_arg_2")
add_ets_verifier_test(FILE "correct_union_override")
add_ets_verifier_test(FILE "correct_union_override_2")
add_ets_verifier_test(FILE "neg_union_arg" VERIFIER_FAIL_TEST SEARCH_STDERR "Bad call incompatible parameter")
add_ets_verifier_test(FILE "neg_union_not_in_boot_context" 
    VERIFIER_FAIL_TEST SEARCH_STDERR "Error: method void ETSGLOBAL::color(ULresource.Resource\;Lstd.core.Double\;Lstd.core.String\;) failed to verify")
add_ets_verifier_test(FILE "neg_union_override_multi1" VERIFIER_FAIL_TEST SEARCH_STDERR "Cannot link class: Multiple override LE\;foo LD\;foo")
add_ets_verifier_test(FILE "neg_union_override_multi2" VERIFIER_FAIL_TEST SEARCH_STDERR "Cannot link class: Multiple override LE\;foo LD\;foo")
add_ets_verifier_test(FILE "neg_union_override_multi3" VERIFIER_FAIL_TEST SEARCH_STDERR "Cannot link class: Multiple override LE\;foo LD\;foo")
add_ets_verifier_test(FILE "neg_union_override_multi4" VERIFIER_FAIL_TEST SEARCH_STDERR "Cannot link class: Multiple override LE\;foo LD\;foo")
add_ets_verifier_test(FILE "neg_union_override_multi5" VERIFIER_FAIL_TEST SEARCH_STDERR "Cannot link class: Multiple override LE\;foo LD\;foo")
add_ets_verifier_test(FILE "correct_union_arg_redecl")