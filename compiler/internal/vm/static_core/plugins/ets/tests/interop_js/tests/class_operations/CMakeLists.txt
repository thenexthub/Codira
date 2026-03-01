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

set(ETS_CONFIG ${CMAKE_CURRENT_BINARY_DIR}/arktsconfig_no_decl.json)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/arktsconfig.in.json ${ETS_CONFIG})

set(ETS_CONFIG_DECL ${CMAKE_CURRENT_BINARY_DIR}/arktsconfig_decl.json)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/arktsconfig.in.decl.json ${ETS_CONFIG_DECL})

panda_ets_interop_js_gtest(ets_interop_js_class_operations
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/class_operations.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/class_operations.ets
    TS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/module.ts
    ETS_CONFIG ${ETS_CONFIG}
    PACKAGE_NAME "class_operations"
)
panda_ets_interop_js_gtest(ets_interop_js_frontend_class_operations
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/class_operations.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/class_operations_frontend.ets
    TS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/module.ts
    ETS_CONFIG ${ETS_CONFIG}
    PACKAGE_NAME "class_operations_frontend"
)
panda_ets_interop_js_gtest(ets_interop_js_class_operations_decl
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/class_operations.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/class_operations_decl.ets
    TS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/module.ts
    ETS_CONFIG ${ETS_CONFIG_DECL}
    PACKAGE_NAME "class_operations_decl"
)
