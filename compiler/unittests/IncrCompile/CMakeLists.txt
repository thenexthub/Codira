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

set(INCREMENTALCOMPILATIONLOGGER_UT_SRC
    $<TARGET_OBJECTS:CodiraASTCommon>
    $<TARGET_OBJECTS:CodiraCommonUtil>
    $<TARGET_OBJECTS:CodiraBasic>
    $<TARGET_OBJECTS:CodiraTempFilesUtil>
    $<TARGET_OBJECTS:CodiraOption>
    $<TARGET_OBJECTS:CodiraProfileUtils>
    $<TARGET_OBJECTS:CodiraUnicodeUtils>
    $<TARGET_OBJECTS:CodiraLex>
)
add_executable(IncrementalCompilationLoggerTest IncrementalCompilationLoggerTest.cpp)

target_link_libraries(
    IncrementalCompilationLoggerTest
    ${CMAKE_DL_LIBS}
    ${INCREMENTALCOMPILATIONLOGGER_UT_SRC}
    GTest::gtest
    GTest::gtest_main)
add_test(NAME IncrementalCompilationLoggerTest COMMAND IncrementalCompilationLoggerTest)
