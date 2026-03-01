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

ani_add_gtest(ani_test_arraybuffer_accessors
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/accessors_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/accessors_test.ets
)

ani_add_gtest(ani_test_arraybuffer_create
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/arraybuffer_create_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/arraybuffer_create_test.ets
)

ani_add_gtest(ani_test_arraybuffer_get_info
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/arraybuffer_get_info_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/arraybuffer_get_info_test.ets
)
