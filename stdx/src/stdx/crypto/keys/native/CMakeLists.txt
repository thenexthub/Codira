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

file(
    GLOB
    keys_ffi_src
    keys.c
    crypto_errors.c
    compat.c
)

set(libname stdx.crypto.keysFFI)

add_library(${libname}-objs OBJECT ${keys_ffi_src})

if(CODIRA_CODEGEN_CODENATIVE_BACKEND)
    add_library(${libname} STATIC $<TARGET_OBJECTS:${libname}-objs>)
endif()

target_include_directories(${libname}-objs PRIVATE ${CMAKE_SOURCE_DIR}/src/stdx/dynamicLoader)
target_include_directories(${libname}-objs PRIVATE ${OPENSSL_INCLUDE_DIR})
install_cangjie_library_ffi(${libname})

# key.ffi compile .so
if(CODIRA_CODEGEN_CODENATIVE_BACKEND)
    set(CMAKE_BUILD_WITH_INSTALL_RPATH ON)
    add_library(${libname}-shared SHARED $<TARGET_OBJECTS:${libname}-objs>)
    target_link_libraries(${libname}-shared cangjie-dynamicLoader-opensslFFI-shared)
    target_link_libraries(${libname}-shared boundscheck)
    if(MINGW)
        target_link_libraries(${libname}-shared crypt32)
        target_link_libraries(${libname}-shared ws2_32)
    endif()
    set_target_properties(${libname}-shared PROPERTIES OUTPUT_NAME ${libname})
    install_cangjie_library_ffi_s(${libname}-shared)
endif()