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

set(TARGET_OPTIONS_INCLUDES ${PANDA_ROOT})

if (PANDA_TARGET_OHOS)
    set(TARGET_OPTIONS_SOURCES ${PANDA_ROOT}/platforms/ohos/default_target_options.cpp)
    list(APPEND TARGET_OPTIONS_SOURCES ${PANDA_ROOT}/platforms/ohos/standalone/ohos_device_helpers.cpp)
else()
    set(TARGET_OPTIONS_SOURCES ${PANDA_ROOT}/platforms/common/default_target_options.cpp)
endif()

panda_add_library(arktarget_options SHARED ${TARGET_OPTIONS_SOURCES})

panda_target_include_directories(arktarget_options PUBLIC ${TARGET_OPTIONS_INCLUDES})

if(PANDA_TARGET_OHOS)
    panda_target_link_libraries(arktarget_options arkbase)
endif()
