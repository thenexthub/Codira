# Copyright (c) 2024-2025 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

set(abc2progTestFiles
    "Field"
    "Functions"
    "HelloWorld"
    "Ins"
    "LiteralArray"
    "Metadata"
    "Record"
    "Type"
)

add_custom_target(abc2program_gen)
add_dependencies(abc2program_gen etsstdlib)

# --ets-strings-concat options #26986
foreach(NAME ${abc2progTestFiles})
    add_custom_target(abc2program_gen_${NAME} COMMENT "Generating ${NAME} for abc2program"
        COMMAND ${PANDA_BINARY_ROOT}/bin/es2panda ${CMAKE_CURRENT_SOURCE_DIR}/ets/${NAME}.ets
        --opt-level=0 --ets-strings-concat=false --output=${CMAKE_CURRENT_BINARY_DIR}/${NAME}.abc
        DEPENDS es2panda)
    add_dependencies(abc2program_gen abc2program_gen_${NAME})
endforeach()

add_custom_target(abc2program_gen_Functions_concat COMMENT "Generating Functions_concat for abc2program"
    COMMAND ${PANDA_BINARY_ROOT}/bin/es2panda ${CMAKE_CURRENT_SOURCE_DIR}/ets/Functions.ets
    --opt-level=0 --ets-strings-concat=true --output=${CMAKE_CURRENT_BINARY_DIR}/Functions_concat.abc
    DEPENDS es2panda)
add_dependencies(abc2program_gen abc2program_gen_Functions_concat)


panda_ets_add_gtest(
    NAME abc2program_tests
    SOURCES
        abc2program_test.cpp
    LIBRARIES
        abc2program
        gmock
    SANITIZERS
        ${PANDA_SANITIZERS_LIST}
    DEPS_TARGETS
        abc2program_gen
)
