# Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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


panda_ets_interop_js_gtest(ets_interop_js_scenarios_ets_to_js_indirect_ref
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/package_indirect_ref/indirect_ref.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/package_indirect_ref/indirect_ref.ets
    JS_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/type_ref/test_function_type_ref_array_apply.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/type_ref/test_function_type_ref_array_bind_with_arg.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/type_ref/test_function_type_ref_array_bind_without_arg.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/type_ref/test_function_type_ref_array_call.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/type_ref/test_function_type_ref_map_apply.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/type_ref/test_function_type_ref_map_bind_with_arg.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/type_ref/test_function_type_ref_map_bind_without_arg.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/type_ref/test_function_type_ref_map_call.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/type_ref/test_function_type_ref_tuple_apply.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/type_ref/test_function_type_ref_tuple_bind_with_arg.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/type_ref/test_function_type_ref_tuple_bind_without_arg.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/type_ref/test_function_type_ref_tuple_call.js
        ${CMAKE_CURRENT_SOURCE_DIR}/indirect_call.test.js
    COMPILATION_JS_WITH_CODES_ON
)

panda_ets_interop_js_gtest(ets_interop_js_scenarios_ets_to_js_indirect_value
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/package_indirect_value/indirect_value.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/package_indirect_value/indirect_value.ets
    JS_SOURCES  
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/type_value/test_function_type_value_boolean_apply.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/type_value/test_function_type_value_boolean_bind_with_arg.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/type_value/test_function_type_value_boolean_bind_without_arg.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/type_value/test_function_type_value_boolean_call.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/type_value/test_function_type_value_number_apply.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/type_value/test_function_type_value_number_bind_with_arg.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/type_value/test_function_type_value_number_bind_without_arg.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/type_value/test_function_type_value_number_call.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/type_value/test_function_type_value_string_apply.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/type_value/test_function_type_value_string_bind_with_arg.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/type_value/test_function_type_value_string_bind_without_arg.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/type_value/test_function_type_value_string_call.js
        ${CMAKE_CURRENT_SOURCE_DIR}/indirect_call.test.js
    COMPILATION_JS_WITH_CODES_ON
)

panda_ets_interop_js_gtest(ets_interop_js_scenarios_ets_to_js_indirect_union
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/package_indirect_union/indirect_union.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/package_indirect_union/indirect_union.ets
    JS_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/type_union/test_function_arg_type_union_number_apply.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/type_union/test_function_arg_type_union_number_bind_with_arg.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/type_union/test_function_arg_type_union_number_bind_without_arg.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/type_union/test_function_arg_type_union_number_call.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/type_union/test_function_arg_type_union_string_apply.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/type_union/test_function_arg_type_union_string_bind_with_arg.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/type_union/test_function_arg_type_union_string_bind_without_arg.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/type_union/test_function_arg_type_union_string_call.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/type_union/test_function_return_type_union_number_apply.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/type_union/test_function_return_type_union_number_bind_with_arg.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/type_union/test_function_return_type_union_number_bind_without_arg.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/type_union/test_function_return_type_union_number_call.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/type_union/test_function_return_type_union_string_apply.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/type_union/test_function_return_type_union_string_bind_with_arg.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/type_union/test_function_return_type_union_string_bind_without_arg.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/type_union/test_function_return_type_union_string_call.js
        ${CMAKE_CURRENT_SOURCE_DIR}/indirect_call.test.js
    COMPILATION_JS_WITH_CODES_ON
)

add_custom_target(ets_interop_js_scenarios_ets_to_js_indirect)
add_dependencies(
    ets_interop_js_scenarios_ets_to_js_indirect_ref_gtests
    ets_interop_js_scenarios_ets_to_js_indirect_value_gtests
    ets_interop_js_scenarios_ets_to_js_indirect_union_gtests
)