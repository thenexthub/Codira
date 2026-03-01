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

ani_add_gtest(ani_test_bind_ops_class_bind_native_methods
    ETS_CONFIG ${CMAKE_CURRENT_SOURCE_DIR}/arktsconfig.json
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/class_bind_native_methods_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/class_bind_native_methods_test.ets
)

ani_add_gtest(ani_test_bind_ops_class_bind_static_native_methods
    ETS_CONFIG ${CMAKE_CURRENT_SOURCE_DIR}/arktsconfig.json
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/class_bind_static_native_methods_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/class_bind_static_native_methods_test.ets
)

ani_add_gtest(ani_test_bind_ops_module_bind_native_functions
    ETS_CONFIG ${CMAKE_CURRENT_SOURCE_DIR}/arktsconfig.json
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/module_bind_native_functions_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/module_bind_native_functions_test.ets
)

ani_add_gtest(ani_test_bind_ops_namespace_bind_native_functions
    ETS_CONFIG ${CMAKE_CURRENT_SOURCE_DIR}/arktsconfig.json
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/namespace_bind_native_functions_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/namespace_bind_native_functions_test.ets
)
