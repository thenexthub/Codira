# Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

include(cmake/tooling_tests.cmake)

if (NOT TARGET es2panda OR PANDA_TARGET_ARM32)
    return()
endif()

add_subdirectory(sampler)
add_subdirectory(coverage)

# test that stdlib is generated successfully with debug info
panda_ets_add_gtest(
    NO_CORES
    NAME debug_info_tests_ets
    SOURCES
        test_debug_info.cpp
    LIBRARIES
        arkfile arkruntime
    INCLUDE_DIRS
        ${PANDA_ROOT}
    SANITIZERS
        ${PANDA_SANITIZERS_LIST}
    CUSTOM_PRERUN_ENVIRONMENT
        TEST_FILE=${CMAKE_CURRENT_BINARY_DIR}/BitopsBitwiseAnd.abc
)

add_custom_target(es2panda_ets_stdlib_debug_info
    COMMAND ${es2panda_bin} --opt-level=0 --debug-info --gen-stdlib=true --arktsconfig=${STDLIB}/stdconfig.json
            --extension=ets --output ${CMAKE_CURRENT_BINARY_DIR}/etsstdlib.abc
    DEPENDS ${es2panda_target}
)

add_custom_target(es2panda_convert_ets_debug_info
    COMMAND ${es2panda_bin} --opt-level=0 --debug-info --gen-stdlib=false --extension=ets
            --output ${CMAKE_CURRENT_BINARY_DIR}/BitopsBitwiseAnd.abc ${CMAKE_CURRENT_SOURCE_DIR}/BitopsBitwiseAnd.ets
    DEPENDS es2panda_ets_stdlib_debug_info
)
add_dependencies(debug_info_tests_ets_gtests es2panda_convert_ets_debug_info)
