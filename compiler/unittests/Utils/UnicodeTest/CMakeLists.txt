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

add_executable(UnicodeTest UnicodeTest.cpp)
target_link_libraries(
    UnicodeTest
    ${CMAKE_DL_LIBS}
    $<TARGET_OBJECTS:CodiraUnicodeUtils>
    GTest::gtest
    GTest::gtest_main)
add_test(NAME UnicodeTest COMMAND UnicodeTest)

add_executable(GeneratedNFCTests NFC.cpp)
target_link_libraries(
    GeneratedNFCTests
    ${CMAKE_DL_LIBS}
    $<TARGET_OBJECTS:CodiraUnicodeUtils>
    GTest::gtest
    GTest::gtest_main)
target_include_directories(GeneratedNFCTests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../../src/Utils/UnicodeTables)
add_test(NAME GeneratedNFCTests COMMAND GeneratedNFCTests)
# regenerate unicode test data tables
if(CODIRA_GENERATE_UNICODE_TABLE)
    add_dependencies(GeneratedNFCTests GenerateNFCTestData)
endif()

add_executable(UnicodeWidthTest UnicodeWidthTest.cpp)
target_link_libraries(
    UnicodeWidthTest
    ${CMAKE_DL_LIBS}
    $<TARGET_OBJECTS:CodiraUnicodeUtils>
    GTest::gtest
    GTest::gtest_main)
add_test(NAME UnicodeWidthTest COMMAND UnicodeWidthTest)
if(CODIRA_GENERATE_UNICODE_TABLE)
    add_dependencies(UnicodeWidthTest GenerateWidthData)
endif()

set(BASE_UTIL_SRC)
