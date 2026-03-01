#===----------------------------------------------------------------------===#
#   Copyright (c) NeXTHub Corporation. All Rights Reserved.
#   DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
#   Author: Tunjay Akbarli
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at:
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
#
#   Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
#   Middletown, DE 19709, New Castle County, USA.
#===----------------------------------------------------------------------===#

include_directories(${CMAKE_SOURCE_DIR}/src/Sema)
if(CODIRA_CODEGEN_CODENATIVE_BACKEND)
    add_executable(TokenSerializationTest TokenSerializationTest.cpp)
    add_executable(NodeSerializationTest NodeSerializationTest.cpp)
    add_executable(MacroTest MacroTest.cpp)

    target_link_libraries(
        TokenSerializationTest
        codira-lsp
        ${LINK_LIBS}
        boundscheck-static
        GTest::gtest
        GTest::gtest_main)
    target_link_libraries(
        NodeSerializationTest
        codira-lsp
        ${LINK_LIBS}
        boundscheck-static
        GTest::gtest
        GTest::gtest_main)
    target_link_libraries(
        MacroTest
        codira-lsp
        ${LINK_LIBS}
        boundscheck-static
        GTest::gtest
        GTest::gtest_main
        TestCompilerInstanceObject)

    add_dependencies(NodeSerializationTest CodiraFlatbuffersHeaders)
    target_include_directories(NodeSerializationTest PRIVATE ${FLATBUFFERS_INCLUDE_DIR})

    add_test(NAME TokenSerializationTest COMMAND TokenSerializationTest)
    add_test(NAME NodeSerializationTest COMMAND NodeSerializationTest)
    add_test(NAME MacroTest COMMAND MacroTest)

endif()
