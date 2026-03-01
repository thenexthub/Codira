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

include_directories(${CMAKE_SOURCE_DIR}/src/Driver/Toolchains)
set(BASE_UTIL_SRC
    $<TARGET_OBJECTS:CodiraASTCommon>
    $<TARGET_OBJECTS:CodiraCommonUtil>
    $<TARGET_OBJECTS:CodiraBasic>
    $<TARGET_OBJECTS:CodiraTempFilesUtil>
    $<TARGET_OBJECTS:CodiraOption>
    $<TARGET_OBJECTS:CodiraUnicodeUtils>
    $<TARGET_OBJECTS:CodiraLex>
)

add_executable(UtilsTests UtilsTests.cpp)
if(CODIRA_CODEGEN_CODENATIVE_BACKEND)
target_link_libraries(
    UtilsTests
    ${CMAKE_DL_LIBS}
    ${BASE_UTIL_SRC}
    $<TARGET_OBJECTS:CodiraProfileUtils>
    $<TARGET_OBJECTS:CodiraUnittestsMangle>
    GTest::gtest
    GTest::gtest_main)
endif()
add_test(NAME UtilsTests COMMAND UtilsTests)

set(CAST_TEST_SRC CastASTTests.cpp CastTyTests.cpp)
set(CAST_TEST_LIB codira-lsp ${CMAKE_DL_LIBS})


add_executable(
    CastTest
    ${CAST_TEST_SRC})
target_link_libraries(
    CastTest
    codira-lsp
    ${CAST_TEST_LIB}
    GTest::gtest
    GTest::gtest_main)
set_target_properties(CastTest PROPERTIES COMPILE_FLAGS "-DUT=1")
add_test(NAME CastTest COMMAND CastTest)

if(CMAKE_BUILD_TYPE MATCHES Release)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fomit-frame-pointer")
    add_definitions(-DRELEASE)

    add_executable(SignalTests SignalTests.cpp)
    target_link_libraries(
        SignalTests
        ${CMAKE_DL_LIBS}
        ${BASE_UTIL_SRC}
        GTest::gtest
        GTest::gtest_main)
    add_test(NAME SignalTests COMMAND SignalTests)

    add_executable(SignalTestCODEC SignalTestCODEC.cpp ${CODIRA_SRC_OBJECTS})
    target_link_libraries(SignalTestCODEC ${LINK_LIBS})
endif()

add_subdirectory(UnicodeTest)
