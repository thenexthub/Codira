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

panda_ets_interop_js_gtest(ets_interop_js__generic_static_sts_to_ts
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/generic_static.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/generic_static.ets
    JS_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/check_class_extends.js
        ${CMAKE_CURRENT_SOURCE_DIR}/check_generic_static_extra_set.js
        ${CMAKE_CURRENT_SOURCE_DIR}/check_generic_static_literal.js
        ${CMAKE_CURRENT_SOURCE_DIR}/check_generic_subset_by_ref.js
        ${CMAKE_CURRENT_SOURCE_DIR}/check_generic_subset_by_value.js
        ${CMAKE_CURRENT_SOURCE_DIR}/check_generic_union_static.js
        ${CMAKE_CURRENT_SOURCE_DIR}/check_interface_static.js
        ${CMAKE_CURRENT_SOURCE_DIR}/check_static_method.js
        ${CMAKE_CURRENT_SOURCE_DIR}/check_user_class.js
        ${CMAKE_CURRENT_SOURCE_DIR}/generic_static.test.js
    COMPILATION_JS_WITH_CODES_ON
)
