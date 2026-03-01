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

panda_ets_interop_js_gtest(ets_interop_js__intersection_arg_ts_to_sts
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/arg_interface/arg_interface.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/arg_interface/arg_interface.ets
    JS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/main.js
    ETS_CONFIG ${ETS_CONFIG}
)

panda_ets_interop_js_gtest(ets_interop_js__intersection_arg_negative_result_ts_to_sts
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/arg_interface_negative_result/arg_interface_negative_result.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/arg_interface_negative_result/arg_interface_negative_result.ets
    JS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/main.js
    ETS_CONFIG ${ETS_CONFIG}
)

panda_ets_interop_js_gtest(ets_interop_js__arg_generic_ts_to_sts
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/arg_generic/arg_generic.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/arg_generic/arg_generic.ets
    JS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/main.js
    ETS_CONFIG ${ETS_CONFIG}
)

panda_ets_interop_js_gtest(ets_interop_js__intersection_arg_generic_negative_result_ts_to_sts
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/arg_generic_negative_result/arg_generic_negative_result.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/arg_generic_negative_result/arg_generic_negative_result.ets
    JS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/main.js
    ETS_CONFIG ${ETS_CONFIG}
)

panda_ets_interop_js_gtest(ets_interop_js__arg_union_ts_to_sts
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/arg_union/arg_union.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/arg_union/arg_union.ets
    JS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/main.js
    ETS_CONFIG ${ETS_CONFIG}
)

panda_ets_interop_js_gtest(ets_interop_js__intersection_arg_union_negative_result_ts_to_sts
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/arg_union_negative_result/arg_union_negative_result.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/arg_union_negative_result/arg_union_negative_result.ets
    JS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/main.js
    ETS_CONFIG ${ETS_CONFIG}
)

panda_ets_interop_js_gtest(ets_interop_js__arg_literal_ts_to_sts
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/arg_literal/arg_literal.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/arg_literal/arg_literal.ets
    JS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/main.js
    ETS_CONFIG ${ETS_CONFIG}
)

panda_ets_interop_js_gtest(ets_interop_js__intersection_arg_literal_negative_result_ts_to_sts
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/arg_literal_negative_result/arg_literal_negative_result.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/arg_literal_negative_result/arg_literal_negative_result.ets
    JS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/main.js
    ETS_CONFIG ${ETS_CONFIG}
)
