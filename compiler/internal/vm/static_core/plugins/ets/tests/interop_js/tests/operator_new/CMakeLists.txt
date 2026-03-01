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

set(ETS_CONFIG ${CMAKE_CURRENT_BINARY_DIR}/arktsconfig.json)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/arktsconfig.in.json ${ETS_CONFIG})

panda_ets_interop_js_gtest(ets_interop_js_operator_new
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/operator_new.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/operator_new_test/operator_new.ets
    JS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/operator_new.js
    ETS_CONFIG ${ETS_CONFIG}
)

panda_ets_interop_js_gtest(ets_interop_js_operator_new_ets_to_js
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/operator_new_ets_to_js.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/operator_new_ets_to_js_test/operator_new_ets_to_js.ets
    JS_SOURCES 
        ${CMAKE_CURRENT_SOURCE_DIR}/operator_new_many_arg.js
        ${CMAKE_CURRENT_SOURCE_DIR}/operator_new_no_arg.js
        ${CMAKE_CURRENT_SOURCE_DIR}/operator_new_one_arg.js
        ${CMAKE_CURRENT_SOURCE_DIR}/operator_new_sub_class.js
        ${CMAKE_CURRENT_SOURCE_DIR}/operator_new.test.js
        ${CMAKE_CURRENT_SOURCE_DIR}/operator_new_two_arg.js
    ETS_CONFIG ${ETS_CONFIG}
    COMPILATION_JS_WITH_CODES_ON
)
