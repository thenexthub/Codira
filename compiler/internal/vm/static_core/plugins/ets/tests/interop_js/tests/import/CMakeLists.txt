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

panda_ets_interop_js_gtest(ets_interop_js__import_primitive_types_ts_to_sts
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/import_primitive_types/import_primitive_types.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/import_primitive_types/import_primitive_types.ets
    ETS_CONFIG ${ETS_CONFIG}
    JS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/main.js
)

panda_ets_interop_js_gtest(ets_interop_js__import_object_ts_to_sts
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/import_object/import_object.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/import_object/import_object.ets
    ETS_CONFIG ${ETS_CONFIG}
    JS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/main.js
)

panda_ets_interop_js_gtest(ets_interop_js__destructured_import_ts_to_sts
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/destructured_import/destructured_import.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/destructured_import/destructured_import.ets
    ETS_CONFIG ${ETS_CONFIG}
    JS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/destructured_import.js
)

panda_ets_interop_js_gtest(ets_interop_js__import_array_ts_to_sts
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/import_array/import_array.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/import_array/import_array.ets
    ETS_CONFIG ${ETS_CONFIG}
    JS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/main.js
)

panda_ets_interop_js_gtest(ets_interop_js__import_function_ts_to_sts
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/import_function/import_function.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/import_function/import_function.ets
    ETS_CONFIG ${ETS_CONFIG}
    JS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/main.js
)

panda_ets_interop_js_gtest(ets_interop_js__import_date_ts_to_sts
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/import_date/import_date.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/import_date/import_date.ets
    ETS_CONFIG ${ETS_CONFIG}
    JS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/main.js
)

panda_ets_interop_js_gtest(ets_interop_js__import_reg_exp_ts_to_sts
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/import_reg_exp/import_reg_exp.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/import_reg_exp/import_reg_exp.ets
    ETS_CONFIG ${ETS_CONFIG}
    JS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/main.js
)

panda_ets_interop_js_gtest(ets_interop_js__import_with_alias_ts_to_sts
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/import_with_alias/import_with_alias.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/import_with_alias/import_with_alias.ets
    ETS_CONFIG ${ETS_CONFIG}
    JS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/main.js
)

panda_ets_interop_js_gtest(ets_interop_js__import_default_ts_to_sts
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/import_default/import_default.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/import_default/import_default.ets
    ETS_CONFIG ${ETS_CONFIG}
    JS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/main.js
)

panda_ets_interop_js_gtest(ets_interop_js__import_type_ts_to_sts
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/import_type/import_type.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/import_type/import_type.ets
    ETS_CONFIG ${ETS_CONFIG}
    JS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/main.js
)

panda_ets_interop_js_gtest(ets_interop_js__import_class_ts_to_sts
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/import_class/import_class.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/import_class/import_class.ets
    ETS_CONFIG ${ETS_CONFIG}
    JS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/main.js
)
