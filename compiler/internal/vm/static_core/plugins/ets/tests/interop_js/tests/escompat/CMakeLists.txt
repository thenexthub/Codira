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

panda_ets_interop_js_gtest(ets_interop_js_test__escompat
    CPP_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/escompat.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/escompat_error.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/escompat_array_buffer.cpp
    ETS_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/escompat_test.ets
    JS_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/array_buffer_js_suites/test_is_view.js
        ${CMAKE_CURRENT_SOURCE_DIR}/array_buffer_js_suites/test_length.js
        ${CMAKE_CURRENT_SOURCE_DIR}/array_buffer_js_suites/test_resize.js
        ${CMAKE_CURRENT_SOURCE_DIR}/array_buffer_js_suites/test_slice.js
        ${CMAKE_CURRENT_SOURCE_DIR}/array_js_suites/test_copy_within.js
        ${CMAKE_CURRENT_SOURCE_DIR}/array_js_suites/test_every.js
        ${CMAKE_CURRENT_SOURCE_DIR}/array_js_suites/test_fill.js
        ${CMAKE_CURRENT_SOURCE_DIR}/array_js_suites/test_filter.js
        ${CMAKE_CURRENT_SOURCE_DIR}/array_js_suites/test_filter_array.js
        ${CMAKE_CURRENT_SOURCE_DIR}/array_js_suites/test_find.js
        ${CMAKE_CURRENT_SOURCE_DIR}/array_js_suites/test_find_index.js
        ${CMAKE_CURRENT_SOURCE_DIR}/array_js_suites/test_find_last.js
        ${CMAKE_CURRENT_SOURCE_DIR}/array_js_suites/test_flat_map.js
        ${CMAKE_CURRENT_SOURCE_DIR}/array_js_suites/test_from_async.js
        ${CMAKE_CURRENT_SOURCE_DIR}/array_js_suites/test_is_array.js
        ${CMAKE_CURRENT_SOURCE_DIR}/array_js_suites/test_join.js
        ${CMAKE_CURRENT_SOURCE_DIR}/array_js_suites/test_last_index_of.js
        ${CMAKE_CURRENT_SOURCE_DIR}/array_js_suites/test_length.js
        ${CMAKE_CURRENT_SOURCE_DIR}/array_js_suites/test_map.js
        ${CMAKE_CURRENT_SOURCE_DIR}/array_js_suites/test_pop.js
        ${CMAKE_CURRENT_SOURCE_DIR}/array_js_suites/test_reduce.js
        ${CMAKE_CURRENT_SOURCE_DIR}/array_js_suites/test_reduce_right.js
        ${CMAKE_CURRENT_SOURCE_DIR}/array_js_suites/test_shift.js
        ${CMAKE_CURRENT_SOURCE_DIR}/array_js_suites/test_slice.js
        ${CMAKE_CURRENT_SOURCE_DIR}/array_js_suites/test_some.js
        ${CMAKE_CURRENT_SOURCE_DIR}/array_js_suites/test_sort.js
        ${CMAKE_CURRENT_SOURCE_DIR}/array_js_suites/test_splice.js
        ${CMAKE_CURRENT_SOURCE_DIR}/array_js_suites/test_to_reversed.js
        ${CMAKE_CURRENT_SOURCE_DIR}/array_js_suites/test_to_sorted.js
        ${CMAKE_CURRENT_SOURCE_DIR}/array_js_suites/test_to_spliced.js
        ${CMAKE_CURRENT_SOURCE_DIR}/array_js_suites/test_with.js
        ${CMAKE_CURRENT_SOURCE_DIR}/compat_array.js
        ${CMAKE_CURRENT_SOURCE_DIR}/compat_boolean.js
        ${CMAKE_CURRENT_SOURCE_DIR}/compat_error.js
        ${CMAKE_CURRENT_SOURCE_DIR}/compat_stringify.js
        ${CMAKE_CURRENT_SOURCE_DIR}/error_js_suites/test_cause.js
        ${CMAKE_CURRENT_SOURCE_DIR}/error_js_suites/test_to_string.js
        ${CMAKE_CURRENT_SOURCE_DIR}/escompat.test.js
    COMPILATION_JS_WITH_CODES_ON
)
