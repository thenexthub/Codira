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

set(CODIRA_CG_DEP_OBJECTS
    $<TARGET_OBJECTS:CodiraCodeGen>
    $<TARGET_OBJECTS:CodiraCHIRBase>
    )

file(GLOB CG_TEST_SRC *.cpp)
add_executable(CGTests
    ${CG_TEST_SRC}
    ${CODIRA_CG_DEP_OBJECTS}
    ${CMAKE_SOURCE_DIR}/src/Basic/Print.cpp)
target_link_libraries(
    CGTests
    ${LLVM_LIBS}
    GTest::gtest
    GTest::gtest_main)

add_test(NAME CGTests COMMAND CGTests)

if(CODIRA_DOWNLOAD_DEPS)
    add_dependencies(CGTests binary-deps)
elseif(CODIRA_CODEGEN_CODENATIVE_BACKEND)
    add_dependencies(CGTests codenative)
endif()
target_include_directories(CGTests PRIVATE ${LLVM_INCLUDE_DIRS})
