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

enable_testing()
include(CTest)
include_directories(${GTEST_INCLUDE_DIRS})
include_directories(TestCompilerInstance)
add_definitions(-DPROJECT_SOURCE_DIR="${PROJECT_SOURCE_DIR}")

# use llvm
include_directories(${LLVM_INCLUDE_DIRS})
get_target_property(LINK_LIBS codec LINK_LIBRARIES)

list(APPEND LINK_LIBS ffi)

if(CODIRA_BUILD_TESTS)
    add_compile_definitions(CODIRA_BUILD_TESTS)
    add_subdirectory(TestCompilerInstance)
    add_subdirectory(Driver)
    add_subdirectory(Frontend)
    add_subdirectory(Lex)
    add_subdirectory(Parse)
    add_subdirectory(AST)
    add_subdirectory(Utils)
    add_subdirectory(Sema)
    add_subdirectory(Option)
    add_subdirectory(Modules)
    add_subdirectory(Basic)
    add_subdirectory(Macro)
    add_subdirectory(ConditionalCompilation)
    add_subdirectory(IncrCompile)
    add_subdirectory(CHIR)
endif()
