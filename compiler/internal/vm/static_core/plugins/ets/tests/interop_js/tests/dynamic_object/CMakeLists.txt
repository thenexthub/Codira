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

panda_ets_interop_js_gtest(ets_interop_js_test__dynamic_object
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/dynamic_object.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/dynamic_object.ets
    JS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/index.js
    ETS_CONFIG ${ETS_CONFIG}
)

panda_ets_interop_js_gtest(ets_interop_js_test_dynamic_call
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/dynamic_call.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/dynamic_call.ets
    ETS_CONFIG ${ETS_CONFIG}
    JS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/index.js
)
