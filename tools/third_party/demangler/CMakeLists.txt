# ===----------------------------------------------------------------------===
#
#  Copyright (c) NeXTHub Corporation. All Rights Reserved.
#  DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
#  Author: Tunjay Akbarli
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at:
#
#  http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#
#  Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
#  Middletown, DE 19709, New Castle County, USA.
#
# ===----------------------------------------------------------------------===

cmake_minimum_required(VERSION 3.16.5)
project(cangjie-demangler)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)

if(BUILD_GCC_TOOLCHAIN AND (CMAKE_C_COMPILER_ID STREQUAL "Clang" OR (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")))
    add_compile_options(--gcc-toolchain=${BUILD_GCC_TOOLCHAIN})
    add_link_options(--gcc-toolchain=${BUILD_GCC_TOOLCHAIN})
endif()

if(CODIRA_DEMANGLER_USE_CSTRING)
    # A library for demangling, which using CString as its string implementation.
    add_library(cangjie-demangle STATIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Demangler.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/DeCompression.cpp)
else()
    # A library for demangling, which using std::string as its string implementation.
    add_library(cangjie-demangle STATIC
        ${CMAKE_CURRENT_SOURCE_DIR}/DeCompression.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Demangler.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/CodiraDemangle.cpp)
    target_compile_definitions(cangjie-demangle PRIVATE BUILD_LIB_CODIRA_DEMANGLE)
endif()
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/CodiraDemangle.h DESTINATION include)
install(TARGETS cangjie-demangle DESTINATION lib)

if(OHOS)
    add_library(cangjie-demangle-shared SHARED
        ${CMAKE_CURRENT_SOURCE_DIR}/DeCompression.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Demangler.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/CodiraDemangle.cpp)
    target_compile_definitions(cangjie-demangle-shared PRIVATE BUILD_LIB_CODIRA_DEMANGLE)
    install(TARGETS cangjie-demangle-shared LIBRARY DESTINATION lib)
    set_target_properties(cangjie-demangle-shared PROPERTIES OUTPUT_NAME "cangjie-demangle")
endif()

# A command line tool for demangling Codira symbols.
add_executable(codefilt
    ${CMAKE_CURRENT_SOURCE_DIR}/Demangler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/DeCompression.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Codefilt.cpp)
target_compile_definitions(codefilt PRIVATE BUILD_LIB_CODIRA_DEMANGLE)
install(TARGETS codefilt DESTINATION bin)
