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

project(toolchain_websocket)

set(SOURCES
    ${TOOLCHAIN_ROOT}/common/log_wrapper.cpp
    ${TOOLCHAIN_ROOT}/websocket/frame_builder.cpp
    ${TOOLCHAIN_ROOT}/websocket/handshake_helper.cpp
    ${TOOLCHAIN_ROOT}/websocket/http.cpp
    ${TOOLCHAIN_ROOT}/websocket/network.cpp
    ${TOOLCHAIN_ROOT}/websocket/websocket_base.cpp
    ${TOOLCHAIN_ROOT}/websocket/server/websocket_server.cpp
)

if (PANDA_TARGET_WINDOWS)
    list(APPEND SOURCES ${TOOLCHAIN_ROOT}/platform/windows/file.cpp)
else()
    list(APPEND SOURCES ${TOOLCHAIN_ROOT}/platform/unix/file.cpp)
endif()

set(DEFINITIONS
    ""
)
if (PANDA_TARGET_WINDOWS)
    list(APPEND DEFINITIONS -DWINDOWS_PLATFORM)
elseif (PANDA_TARGET_OHOS)
    list(APPEND DEFINITIONS -DOHOS_PLATFORM)
endif()

panda_add_library(toolchain_websocket ${PANDA_DEFAULT_LIB_TYPE} ${SOURCES})
panda_target_include_directories(toolchain_websocket SYSTEM PUBLIC
    ${TOOLCHAIN_ROOT}/websocket
    ${TOOLCHAIN_ROOT}
    ${PANDA_THIRD_PARTY_SOURCES_DIR}/openssl/include/
    ${PANDA_THIRD_PARTY_SOURCES_DIR}/utils_native/base/include/
)
panda_target_link_libraries(toolchain_websocket
    c_secshared
    crypto_shared
)
panda_target_compile_definitions(toolchain_websocket PRIVATE
    NDEBUG
    ${DEFINITIONS}
)
