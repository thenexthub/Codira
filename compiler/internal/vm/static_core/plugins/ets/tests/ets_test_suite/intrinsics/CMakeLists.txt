# Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

set(intrinsics_tests
    bigint64array_sort
    float32array_sort
    float64array_sort
    int8array_sort
    int16array_sort
    int32array_sort
    isNaN
    isFinite
    isInteger
    isSafeInteger
    string_array
    string_charat
    string_compare_to
    string_concat
    string_endswith
    string_equals
    string_from_char_code_escape
    string_from_char_code
    string_from_chars
    string_from_string
    string_fromcodepoint
    string_gc
    string_get
    string_getbytes
    string_getchars
    string_getlength
    string_hashcode
    string_indexof
    string_indexofstring
    string_iscompressed
    string_isempty
    string_iswellformed
    string_lastindexof
    string_length
    string_normalize
    string_repeat
    string_startswith
    string_substring
    string_tolocalelowercase
    string_tolocaleuppercase
    string_tolowercase
    string_touppercase
    string_trim
    stringbuilder_oom
    stringbuilder
    typed_arrays_reverse
    char_isuppercase
    abs
    min
    max
    round
    floor
    ceil
    trunc
    uint8array_sort
    uint8clampedarray_sort
    uint16array_sort
    uint32array_sort
    biguint64array_sort
)

if (NOT PANDA_TARGET_ARM32)
    # need coroutine support
    # #21382: fixed array -> T[] casts
    # list(APPEND intrinsics_tests to_string_cache)
endif()

set(intrinsics_tests_in_dir "${CMAKE_CURRENT_SOURCE_DIR}")
set(intrinsics_tests_out_dir "${CMAKE_CURRENT_BINARY_DIR}")

add_custom_target(ets_test_suite_intrinsics)

foreach(test ${intrinsics_tests})
    set(test_out_dir "${intrinsics_tests_out_dir}/${test}")

    set(test_in "${intrinsics_tests_in_dir}/${test}.ets")
    set(target ets_test_suite_intrinsics_${test})
    set(extra_options "")

    # disable OSR for the tests throwing exceptions
    if ("${test}" STREQUAL "stringbuilder_oom")
        set(extra_options RUNTIME_EXTRA_OPTIONS "--compiler-enable-osr=false")
    endif()

    # enable heap-verifier to check the TLB cleanup before going to the slow path
    if ("${test}" STREQUAL "string_from_char_code")
        set(extra_options RUNTIME_EXTRA_OPTIONS "--heap-verifier=fail_on_verification:pre:into:before_g1_concurrent:post")
    endif()

    if ("${test}" STREQUAL "stringbuilder" OR
        "${test}" STREQUAL "string_compare_to" OR
        "${test}" STREQUAL "string_from_char_code")
        run_int_jit_aot_ets_code_foreach_gc(ets_test_suite_intrinsics ${test_in} ${test_out_dir} ${target})
    else()
        run_int_jit_aot_ets_code(${test_in} ${test_out_dir} ${target} ${extra_options})
        add_dependencies(ets_test_suite_intrinsics ${target})
    endif()
endforeach()
