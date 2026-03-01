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

include(ExtractArchive)
set(libname stdx.compress.zlibFFI)

include_directories(${CMAKE_BINARY_DIR}/third_party/zlib/include)

message(STATUS "---CMAKE_BINARY_DIR: ${CMAKE_BINARY_DIR} ")

set(ZLIB_OBJS_DIR ${CMAKE_CURRENT_BINARY_DIR}/temp)
file(MAKE_DIRECTORY ${ZLIB_OBJS_DIR})

if(WIN32)
    extract_archive(
    FROM ${CMAKE_BINARY_DIR}/third_party/zlib/lib/libzlibstatic.a
    TO ${ZLIB_OBJS_DIR})
else()
    extract_archive(
    FROM ${CMAKE_BINARY_DIR}/third_party/zlib/lib/libz.a
    TO ${ZLIB_OBJS_DIR})
endif()

file(GLOB ZLIB_OBJS ${ZLIB_OBJS_DIR}/*.o ${ZLIB_OBJS_DIR}/*.obj)
set(ZLIB_OBJS ${ZLIB_OBJS} PARENT_SCOPE)
add_library(${libname}-objs OBJECT ./zlib.c)
target_compile_options(${libname}-objs PRIVATE ${CMAKE_C_COVERAGE_FLAGS})

if(CODIRA_CODEGEN_CODENATIVE_BACKEND)
    add_library(${libname} STATIC $<TARGET_OBJECTS:${libname}-objs> ${ZLIB_OBJS})
endif()
install_cangjie_library_ffi(${libname})
