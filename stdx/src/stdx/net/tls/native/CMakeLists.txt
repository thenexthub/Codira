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
    tls_ffi_src
    alpn.c
    ciphers.c
    compat.c
    errors.c
    hostname.c
    sessions.c
    tls_bio.c
    tls-impl.c
    tls_keys_certs.c
    versions.c
    dhparam.c
    provider_signature.c
    provider_asymcipher.c
    provider_keymgt.c
    provider_helper.c
    provider.c
)

set(libname stdx.net.tlsFFI)
add_library(${libname}-objs OBJECT ${tls_ffi_src})
target_compile_options(${libname}-objs PRIVATE ${CMAKE_C_COVERAGE_FLAGS})
target_include_directories(${libname}-objs PRIVATE ${OPENSSL_INCLUDE_DIR})
target_include_directories(${libname}-objs PRIVATE ${CMAKE_SOURCE_DIR}/src/stdx/dynamicLoader)
if(CODIRA_CODEGEN_CODENATIVE_BACKEND)
    include(AddAndCombineStaticLib)

    add_and_combine_static_lib(
        TARGET ${libname}
        OUTPUT_NAME lib${libname}.a
        LIBRARIES
            $<TARGET_OBJECTS:${libname}-objs>
            $<TARGET_OBJECTS:cangjie-dynamicLoader-opensslFFI-objs>
        DEPENDS ${libname}-objs cangjie-dynamicLoader-opensslFFI)
    string(TOLOWER ${TARGET_TRIPLE_DIRECTORY_PREFIX} output_lib_dir)
    set(CODENATIVE_BACKEND "codenative")
    install(FILES ${CMAKE_BINARY_DIR}/lib/lib${libname}.a DESTINATION ${output_lib_dir}_${CODENATIVE_BACKEND}${SANITIZER_SUBPATH}/static/stdx)

    add_library(${libname}-shared SHARED $<TARGET_OBJECTS:${libname}-objs>)

    # tls.ffi compile .so
    set(CMAKE_BUILD_WITH_INSTALL_RPATH ON)
    target_link_libraries(${libname}-shared cangjie-dynamicLoader-opensslFFI-shared)
    target_link_libraries(${libname}-shared boundscheck)
    if(MINGW)
        target_link_options(${libname}-shared PRIVATE -pthread -Wl,--export-all-symbols)
        target_link_libraries(${libname}-shared crypt32)
        target_link_libraries(${libname}-shared ws2_32)
    endif()
    set_target_properties(${libname}-shared PROPERTIES OUTPUT_NAME ${libname})
    install_cangjie_library_ffi_s(${libname}-shared)
endif()
