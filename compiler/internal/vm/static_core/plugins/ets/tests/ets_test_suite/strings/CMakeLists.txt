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

set(strings_tests
    string_api
    string_compare_to
)

set(strings_tests_in_dir "${CMAKE_CURRENT_SOURCE_DIR}")
set(strings_tests_out_dir "${CMAKE_CURRENT_BINARY_DIR}")

add_custom_target(ets_test_suite_strings)

foreach(test ${strings_tests})
    set(test_out_dir "${strings_tests_out_dir}/${test}")

    set(test_in "${strings_tests_in_dir}/${test}.ets")
    set(target ets_test_suite_strings_${test})

    if (PANDA_TARGET_ARM32)
        # failure in regalloc, see #13413
        set(extra_options RUNTIME_EXTRA_OPTIONS "--compiler-inlining-blacklist=std.core.ETSGLOBAL::copyTo,std.core.String::codePointAt" "--compiler-regex='(?!std.core.ETSGLOBAL::copyTo)&(?!std.core.String::codePointAt).*'" "--compiler-ignore-failures=true")
    endif()

    run_int_jit_aot_ets_code(${test_in} ${test_out_dir} ${target} ${extra_options})
    add_dependencies(ets_test_suite_strings
        ${target}-ets-jit
        ${target}-ets-int)
    if (NOT CMAKE_CROSSCOMPILING)
        add_dependencies(ets_test_suite_strings ${target}-ets-aot)
    endif()
endforeach()

#26986
add_custom_target(ets_test_suite_strings_concat)

set(strings_concat_tests
    string_concat
)

foreach(test ${strings_concat_tests})
    add_ets_test(ets_test_suite_strings_concat
        FILE "${test}.ets"
        TEST_NAME "ets_test_suite_strings_concat_true_${test}"
        MODE "INT" "JIT" "AOT" "LLVMAOT" "JITOSR"
        COMPILER_EXTRA_OPTIONS "--ets-strings-concat=true"
    )
    add_ets_test(ets_test_suite_strings_concat
        FILE "${test}.ets"
        TEST_NAME "ets_test_suite_strings_concat_false_${test}"
        MODE "INT" "JIT" "AOT" "LLVMAOT" "JITOSR"
        COMPILER_EXTRA_OPTIONS "--ets-strings-concat=false"
    )
endforeach()

# test build stdlib
add_custom_target(ets_test_suite_strings_concat_stdlib
    COMMAND ${es2panda_bin}
            --opt-level=2
            --gen-stdlib=true
            --ets-strings-concat=true
            --extension=ets
            --output ${strings_tests_out_dir}/etsstdlib.abc
    DEPENDS ${es2panda_target}
)
add_dependencies(ets_test_suite_strings_concat ets_test_suite_strings_concat_stdlib)
