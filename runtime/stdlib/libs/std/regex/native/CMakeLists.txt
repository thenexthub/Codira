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

include_directories(${CMAKE_BINARY_DIR}/include/pcre2/include)

set(libname cangjie-std-regexFFI)
set(PCRE2_OBJS_DIR ${CMAKE_CURRENT_BINARY_DIR}/temp)
file(MAKE_DIRECTORY ${PCRE2_OBJS_DIR})
extract_archive(
    FROM ${CMAKE_BINARY_DIR}/third_party/pcre2/lib/libpcre2-8.a
    TO ${PCRE2_OBJS_DIR})
file(GLOB PCRE2_OBJS ${PCRE2_OBJS_DIR}/*.o ${PCRE2_OBJS_DIR}/*.obj)
add_library(${libname}-objs OBJECT ./regex_match.c)
target_compile_options(${libname}-objs PRIVATE ${CMAKE_C_COVERAGE_FLAGS})
add_library(${libname} STATIC $<TARGET_OBJECTS:${libname}-objs> ${PCRE2_OBJS})
install_cangjie_library_ffi(${libname})

set(_libname cangjie-std-regexFFIwithoutPCRE2)
add_library(${_libname}-objs OBJECT ./regex_match.c)
target_compile_options(${_libname}-objs PRIVATE ${CMAKE_C_COVERAGE_FLAGS})
add_library(${_libname} STATIC $<TARGET_OBJECTS:${libname}-objs>)
target_link_directories(${_libname} PRIVATE ${CMAKE_BINARY_DIR}/third_party/pcre2/lib)
