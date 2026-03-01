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
    openssl_ffi_src
    opensslSymbols.c
)
set(libname cangjie-dynamicLoader-opensslFFI)
add_library(${libname}-objs OBJECT ${openssl_ffi_src})


if(CODIRA_CODEGEN_CODENATIVE_BACKEND)
    add_library(${libname} STATIC $<TARGET_OBJECTS:${libname}-objs>)
endif()

target_include_directories(${libname}-objs PRIVATE ${OPENSSL_INCLUDE_DIR})

install_cangjie_library_ffi(${libname})


if(CODIRA_CODEGEN_CODENATIVE_BACKEND)
    add_library(${libname}-shared SHARED $<TARGET_OBJECTS:${libname}-objs>)
    set_target_properties(${libname}-shared PROPERTIES OUTPUT_NAME ${libname})
    if(MINGW)
        target_link_options(${libname}-shared PRIVATE -pthread -Wl,--export-all-symbols)
    endif()
    install_cangjie_library_ffi_s(${libname}-shared)
endif()
