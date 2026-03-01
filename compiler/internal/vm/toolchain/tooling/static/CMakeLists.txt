# Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

project(arkinspector)

set(ARKINSPECTOR_ROOTS
    connection/endpoint_base.cpp
    connection/server_endpoint_base.cpp
    debugger/breakpoint.cpp
    debugger/breakpoint_storage.cpp
    debugger/conditional_breakpoint.cpp
    debugger/debug_info_cache.cpp
    debugger/debuggable_thread.cpp
    debugger/object_repository.cpp
    debugger/thread_state.cpp
    error.cpp
    evaluation/evaluation_engine.cpp
    init.cpp
    inspector.cpp
    inspector_server.cpp
    json_serialization/jrpc_error.cpp
    session_manager.cpp
    source_manager.cpp
    types/custom_url_breakpoint_response.cpp
    types/debugger_evaluation_request.cpp
    types/debugger_call_function_on_request.cpp
    types/exception_details.cpp
    types/location.cpp
    types/profile_result.cpp
    types/remote_object.cpp
    types/object_preview.cpp
    types/scope.cpp
    types/url_breakpoint_request.cpp
    types/url_breakpoint_response.cpp
)
set(ARKINSPECTOR_LINKED_LIBRARIES
    arkbase
    arkdisassembler
    arkfile
    arkruntime
)
set(ARKINSPECTOR_DEFINITIONS
    ""
)

# required for arkcompiler_toolchain compilation
if(PANDA_TARGET_OHOS)
    list(APPEND ARKINSPECTOR_DEFINITIONS -DOHOS_PLATFORM)
endif()

if(PANDA_TOOLING_ASIO)
    list(APPEND ARKINSPECTOR_ROOTS
        connection/asio/asio_server.cpp
        connection/asio/ws_logger.cpp
    )
    set(ARKINSPECTOR_INCLUDES
        ${PANDA_THIRD_PARTY_SOURCES_DIR}/asio/asio/include/
        ${PANDA_THIRD_PARTY_SOURCES_DIR}/websocketpp/
    )
    list(APPEND ARKINSPECTOR_DEFINITIONS
        -DPANDA_TOOLING_ASIO
        -DASIO_NO_TYPEID
        -DASIO_STANDALONE
    )
else()
    list(APPEND ARKINSPECTOR_ROOTS
        connection/ohos_ws/ohos_ws_server_endpoint.cpp
        connection/ohos_ws/ohos_ws_server.cpp
    )
    list(APPEND ARKINSPECTOR_LINKED_LIBRARIES toolchain_websocket)
    set(ARKINSPECTOR_INCLUDES
        ${PANDA_THIRD_PARTY_SOURCES_DIR}/arkcompiler/toolchain/
        ${PANDA_ROOT}/runtime/tooling/
    )
endif()


panda_add_library(${PROJECT_NAME} SHARED ${ARKINSPECTOR_ROOTS})

panda_target_compile_definitions(${PROJECT_NAME} PUBLIC ${ARKINSPECTOR_DEFINITIONS})

panda_target_compile_options(${PROJECT_NAME} PUBLIC -fexceptions -Wno-invalid-offsetof)

add_dependencies(${PROJECT_NAME} arkruntime)

panda_target_include_directories(${PROJECT_NAME}
    PUBLIC ${PANDA_ROOT}/runtime/include/
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)

panda_target_include_directories(${PROJECT_NAME} SYSTEM
    PUBLIC ${ARKINSPECTOR_INCLUDES}
)

panda_target_link_libraries(${PROJECT_NAME}
    ${ARKINSPECTOR_LINKED_LIBRARIES}
)

panda_add_sanitizers(TARGET ${PROJECT_NAME} SANITIZERS ${PANDA_SANITIZERS_LIST})

panda_add_gtest(
    NAME arkinspector_tests
    NO_CORES
    SOURCES
        tests/base64.cpp
        tests/debug_info_cache.cpp
        tests/inspector_server.cpp
        tests/json_object_matcher.cpp
        tests/object_repository.cpp
        tests/session_manager.cpp
        tests/source_manager.cpp
        tests/thread_state.cpp
    INCLUDE_DIRS
        ${PANDA_ROOT}/runtime/tooling/inspector 
    LIBRARIES
        arkassembler
        ${PROJECT_NAME}
        arkruntime
        gmock
    SANITIZERS
        ${PANDA_SANITIZERS_LIST}
)
