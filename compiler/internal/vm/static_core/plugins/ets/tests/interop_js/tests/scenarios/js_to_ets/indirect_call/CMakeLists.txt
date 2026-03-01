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

panda_ets_interop_js_gtest(ets_interop_js_scenarios_js_to_ets_indirect_any
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/indirect_any.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/indirect_any.ets
    JS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/indirect_call.js
    ETS_CONFIG ${ETS_CONFIG}
)

panda_ets_interop_js_gtest(ets_interop_js_scenarios_js_to_ets_indirect_literal
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/indirect_literal.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/indirect_literal.ets
    JS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/indirect_call.js
    ETS_CONFIG ${ETS_CONFIG}
)

panda_ets_interop_js_gtest(ets_interop_js_scenarios_js_to_ets_indirect_extra
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/indirect_extra.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/indirect_extra.ets
    JS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/indirect_call.js
    ETS_CONFIG ${ETS_CONFIG}
)

panda_ets_interop_js_gtest(ets_interop_js_scenarios_js_to_ets_indirect_ref
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/indirect_ref.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/indirect_ref.ets
    JS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/indirect_call.js
    ETS_CONFIG ${ETS_CONFIG}
)

panda_ets_interop_js_gtest(ets_interop_js_scenarios_js_to_ets_indirect_value
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/indirect_value.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/indirect_value.ets
    JS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/indirect_call.js
    ETS_CONFIG ${ETS_CONFIG}
)

panda_ets_interop_js_gtest(ets_interop_js_scenarios_js_to_ets_indirect_user
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/indirect_user.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/indirect_user.ets
    JS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/indirect_call.js
    ETS_CONFIG ${ETS_CONFIG}
)

panda_ets_interop_js_gtest(ets_interop_js_scenarios_js_to_ets_indirect_union
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/indirect_union.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/indirect_union.ets
    JS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/indirect_call.js
    ETS_CONFIG ${ETS_CONFIG}
)

add_custom_target(ets_interop_js_scenarios_js_to_ets_indirect)
add_dependencies(
    ets_interop_js_scenarios_js_to_ets_indirect
    ets_interop_js_scenarios_js_to_ets_indirect_any_gtests
    ets_interop_js_scenarios_js_to_ets_indirect_literal_gtests
    ets_interop_js_scenarios_js_to_ets_indirect_extra_gtests
    ets_interop_js_scenarios_js_to_ets_indirect_ref_gtests
    ets_interop_js_scenarios_js_to_ets_indirect_value_gtests
    ets_interop_js_scenarios_js_to_ets_indirect_user_gtests
    ets_interop_js_scenarios_js_to_ets_indirect_union_gtests
)
