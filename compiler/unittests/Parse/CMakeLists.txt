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

add_executable(ParserTest ParserTest.cpp)
target_link_libraries(
    ParserTest
    codira-lsp
    ${CMAKE_DL_LIBS}
    GTest::gtest
    GTest::gtest_main)
add_test(NAME ParserTest COMMAND ParserTest)

add_executable(ParseTypeTest ParseTypeTest.cpp)
target_link_libraries(
    ParseTypeTest
    codira-lsp
    ${CMAKE_DL_LIBS}
    GTest::gtest
    GTest::gtest_main)
add_test(NAME ParseTypeTest COMMAND ParseTypeTest)

add_executable(ParseTypeAliasTest ParseTypeAliasTest.cpp)
target_link_libraries(
    ParseTypeAliasTest
    codira-lsp
    ${CMAKE_DL_LIBS}
    GTest::gtest
    GTest::gtest_main)
add_test(NAME ParseTypeAliasTest COMMAND ParseTypeAliasTest)

add_executable(ParseCommentTest ParseCommentTest.cpp)
target_link_libraries(
    ParseCommentTest
    codira-lsp
    ${LINK_LIBS}
    boundscheck-static
    GTest::gtest
    GTest::gtest_main
    TestCompilerInstanceObject)

add_test(NAME ParseCommentTest COMMAND ParseCommentTest)