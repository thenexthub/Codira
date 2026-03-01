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


panda_ets_interop_js_gtest(ets_interop_js__generic_call_params_sts_to_ts
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/generic_call_params.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/generic_call_params.ets
    JS_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/check_apply_function_generic_array.js
        ${CMAKE_CURRENT_SOURCE_DIR}/check_apply_function_generic.js
        ${CMAKE_CURRENT_SOURCE_DIR}/check_apply_function_generics.js
        ${CMAKE_CURRENT_SOURCE_DIR}/check_apply_function_generic_union.js
        ${CMAKE_CURRENT_SOURCE_DIR}/check_apply_function_with_constraints.js
        ${CMAKE_CURRENT_SOURCE_DIR}/check_generic_extends_class.js
        ${CMAKE_CURRENT_SOURCE_DIR}/check_generic_subset_class.js
        ${CMAKE_CURRENT_SOURCE_DIR}/generic_call_params.test.js
    COMPILATION_JS_WITH_CODES_ON
)
