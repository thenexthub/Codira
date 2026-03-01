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

ani_add_gtest(ani_test_array_get_length
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/array_get_length_test.cpp
)

ani_add_gtest(ani_test_array_get_set
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/array_get_set_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/array_get_set_test.ets
)

ani_add_gtest(ani_test_array_push_pop
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/array_push_pop_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/array_push_pop_test.ets
)

ani_add_gtest(ani_test_array_managed
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/array_managed_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/array_managed_test.ets
)
