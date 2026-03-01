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

panda_ets_interop_js_gtest(ets_interop_js_test_class_methods_returning_values_ets_to_js
    CPP_SOURCES ${CMAKE_CURRENT_LIST_DIR}/test_class_methods.cpp
    ETS_SOURCES ${CMAKE_CURRENT_LIST_DIR}/test_class_methods.ets
    JS_SOURCES 
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/test_function_return_bool_subset_by_value.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/test_function_return_class.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/test_function_return_interface.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/test_function_return_int_subset_by_value.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/test_function_return_map_subset_by_ref.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/test_function_return_null_subset_by_value.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/test_function_return_string_subset_by_ref.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/test_function_return_string_subset_by_value.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/test_function_return_undefined_subset_by_value.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/test_function_return_union.js
        ${CMAKE_CURRENT_SOURCE_DIR}/test_class_methods.js
    COMPILATION_JS_WITH_CODES_ON
)
