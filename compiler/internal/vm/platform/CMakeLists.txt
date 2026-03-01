# Copyright (c) 2024 Huawei Device Co., Ltd.
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

cmake_minimum_required(VERSION 3.10)

project(platform)

set(PLATFORM_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

set(ARTPLATFORM_SOURCES
    hybrid/xgc/xgc.cpp
)

add_library(platform SHARED ${ARTPLATFORM_SOURCES})
target_include_directories(platform PUBLIC ${PLATFORM_ROOT})

if(PANDA_WITH_RUNTIME)
    panda_target_link_libraries(arkruntime_obj platform)
endif(PANDA_WITH_RUNTIME)
