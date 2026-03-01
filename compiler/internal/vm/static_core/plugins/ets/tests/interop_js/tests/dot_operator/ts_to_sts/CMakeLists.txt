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

panda_ets_interop_js_gtest(ets_interop_js__optional_operator_ts_to_sts
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/optional_operator/optional_operator.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/optional_operator/optional_operator.ets
    ETS_CONFIG ${ETS_CONFIG}
    TS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/main.ts
)

panda_ets_interop_js_gtest(ets_interop_js__not_nullish_operator_ts_to_sts
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/not_nullish_operator/not_nullish_operator.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/not_nullish_operator/not_nullish_operator.ets
    ETS_CONFIG ${ETS_CONFIG}
    TS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/main.ts
)

panda_ets_interop_js_gtest(ets_interop_js__call_chaining_ts_to_sts
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/call_chaining/call_chaining.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/call_chaining/call_chaining.ets
    ETS_CONFIG ${ETS_CONFIG}
    TS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/main.ts
)

panda_ets_interop_js_gtest(ets_interop_js__call_method_for_value_ts_to_sts
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/call_method_for_value/call_method_for_value.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/call_method_for_value/call_method_for_value.ets
    ETS_CONFIG ${ETS_CONFIG}
    TS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/main.ts
)
