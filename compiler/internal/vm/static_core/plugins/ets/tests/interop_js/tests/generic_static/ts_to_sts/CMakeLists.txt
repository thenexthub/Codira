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

panda_ets_interop_js_gtest(ets_interop_js__static_method_ts_to_sts
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/static_method/static_method.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/static_method/static_method.ets
    ETS_CONFIG ${ETS_CONFIG}
    JS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/main.js
)

panda_ets_interop_js_gtest(ets_interop_js__class_extends_ts_to_sts
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/class_extends/class_extends.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/class_extends/class_extends.ets
    ETS_CONFIG ${ETS_CONFIG}
    JS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/main.js
)

panda_ets_interop_js_gtest(ets_interop_js__generic_static_literal_ts_to_sts
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/generic_static_literal/generic_static_literal.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/generic_static_literal/generic_static_literal.ets
    ETS_CONFIG ${ETS_CONFIG}
    JS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/main.js
)

panda_ets_interop_js_gtest(ets_interop_js__generic_static_extra_set_ts_to_sts
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/generic_static_extra_set/generic_static_extra_set.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/generic_static_extra_set/generic_static_extra_set.ets
    ETS_CONFIG ${ETS_CONFIG}
    JS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/main.js
)

panda_ets_interop_js_gtest(ets_interop_js__generic_subset_by_ref_static_ts_to_sts
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/generic_subset_by_ref/generic_subset_by_ref.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/generic_subset_by_ref/generic_subset_by_ref.ets
    ETS_CONFIG ${ETS_CONFIG}
    JS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/main.js
)

panda_ets_interop_js_gtest(ets_interop_js__generic_subset_by_value_static_ts_to_sts
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/generic_subset_by_value/generic_subset_by_value.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/generic_subset_by_value/generic_subset_by_value.ets
    ETS_CONFIG ${ETS_CONFIG}
    JS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/main.js
)

panda_ets_interop_js_gtest(ets_interop_js__generic_union_static_ts_to_sts
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/generic_union_static/generic_union_static.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/generic_union_static/generic_union_static.ets
    ETS_CONFIG ${ETS_CONFIG}
    JS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/main.js
)

panda_ets_interop_js_gtest(ets_interop_js__generic_user_class_static_ts_to_ets
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/user_class/user_class.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/user_class/user_class.ets
    ETS_CONFIG ${ETS_CONFIG}
    JS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/main.js
)

panda_ets_interop_js_gtest(ets_interop_js__generic_interface_static_ts_to_sts
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/interface_static/interface_static.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/interface_static/interface_static.ets
    ETS_CONFIG ${ETS_CONFIG}
    JS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/main.js
)
