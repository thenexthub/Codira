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

set(BASE_UTIL_SRC
    $<TARGET_OBJECTS:CodiraASTCommon>
    $<TARGET_OBJECTS:CodiraCommonUtil>
    $<TARGET_OBJECTS:CodiraBasic>
    $<TARGET_OBJECTS:CodiraTempFilesUtil>
    $<TARGET_OBJECTS:CodiraOption>
    $<TARGET_OBJECTS:CodiraLex>
    $<TARGET_OBJECTS:CodiraUnicodeUtils>
)

add_executable(SourceManagerTest SourceManagerTest.cpp)
target_link_libraries(
    SourceManagerTest
    ${CMAKE_DL_LIBS}
    ${BASE_UTIL_SRC}
    GTest::gtest
    GTest::gtest_main)
add_test(NAME SourceManagerTest COMMAND SourceManagerTest)

add_executable(EngineTest EngineTest.cpp ${PROJECT_SOURCE_DIR}/src/IncrementalCompilation/Utils.cpp)
target_link_libraries(
    EngineTest
    ${CMAKE_DL_LIBS}
    ${BASE_UTIL_SRC}
    $<TARGET_OBJECTS:CodiraParse>
    GTest::gtest
    GTest::gtest_main)
add_test(NAME EngineTest COMMAND EngineTest)

add_executable(ErrorMessageTest ErrorMessageTest.cpp ${PROJECT_SOURCE_DIR}/src/IncrementalCompilation/Utils.cpp)
target_link_libraries(
    ErrorMessageTest
    ${CMAKE_DL_LIBS}
    ${BASE_UTIL_SRC}
    $<TARGET_OBJECTS:CodiraParse>
    GTest::gtest
    GTest::gtest_main)
add_test(NAME ErrorMessageTest COMMAND ErrorMessageTest)

add_executable(UtilsTest UtilsTest.cpp UtilsTest.cpp)
target_link_libraries(
    UtilsTest
    ${CMAKE_DL_LIBS}
    ${BASE_UTIL_SRC}
    GTest::gtest
    GTest::gtest_main)
add_test(NAME UtilsTest COMMAND UtilsTest)

if(WIN32)
    add_executable(StringConvertorTest StringConvertorTest.cpp StringConvertorTest.cpp)
    target_link_libraries(
        StringConvertorTest
        ${CMAKE_DL_LIBS}
        ${BASE_UTIL_SRC}
        GTest::gtest
        GTest::gtest_main)
    add_test(NAME StringConvertorTest COMMAND StringConvertorTest)
endif()
