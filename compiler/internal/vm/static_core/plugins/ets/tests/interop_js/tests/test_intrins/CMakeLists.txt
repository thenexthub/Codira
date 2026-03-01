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

panda_ets_interop_js_gtest(ets_interop_js_test_intrins
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/test_intrins.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/test_intrins.ets
    TS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/index.ts
    ETS_CONFIG ${ETS_CONFIG}
    PACKAGE_NAME "test_intrins"
)

panda_ets_interop_js_gtest(ets_interop_js_frontend_test_intrins
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/test_intrins.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/frontend_test_intrins.ets
    TS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/index.ts
    ETS_CONFIG ${ETS_CONFIG}
    PACKAGE_NAME "frontend_test_intrins"
)
