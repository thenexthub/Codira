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

include(AddAndCombineStaticLib)
include(ExtractArchive)

set(BACKEND_TYPE CODENATIVE)

##  make shared/static library
# if we support cross-compiling, we need to change here for target type
string(TOLOWER ${TARGET_TRIPLE_DIRECTORY_PREFIX}_${BACKEND_TYPE} output_code_object_dir)
string(TOLOWER ${TARGET_TRIPLE_DIRECTORY_PREFIX} output_triple_name)
set(CODENATIVE_BACKEND "codenative")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/${output_triple_name}_${CODENATIVE_BACKEND}${SANITIZER_SUBPATH})
set(output_code_object_dir ${CMAKE_BINARY_DIR}/modules/${output_code_object_dir})

make_cangjie_lib(
    cangjieStdx IS_SHARED
    DEPENDS cangjie${BACKEND_TYPE}Stdx
    CODIRA_STD_LIB_LINK std-core
    OBJECTS ${output_code_object_dir}/stdx.o)

add_library(stdx STATIC ${output_code_object_dir}/stdx.o)
set_target_properties(stdx PROPERTIES LINKER_LANGUAGE C)
install(TARGETS stdx DESTINATION ${output_triple_name}_${CODENATIVE_BACKEND}${SANITIZER_SUBPATH}/static/stdx)

make_cangjie_lib(
    encoding IS_SHARED
    DEPENDS cangjie${BACKEND_TYPE}Encoding
    CODIRA_STD_LIB_LINK std-core
    OBJECTS ${output_code_object_dir}/stdx/encoding.o)

add_library(stdx.encoding STATIC ${output_code_object_dir}/stdx/encoding.o)
set_target_properties(stdx.encoding PROPERTIES LINKER_LANGUAGE C)
install(TARGETS stdx.encoding DESTINATION ${output_triple_name}_${CODENATIVE_BACKEND}${SANITIZER_SUBPATH}/static/stdx)

make_cangjie_lib(
    serialization IS_SHARED
    DEPENDS cangjie${BACKEND_TYPE}StdxSerialization
    CODIRA_STD_LIB_LINK std-core
    OBJECTS ${output_code_object_dir}/stdx/serialization.o)

add_library(stdx.serialization STATIC ${output_code_object_dir}/stdx/serialization.o)
set_target_properties(stdx.serialization PROPERTIES LINKER_LANGUAGE C)
install(TARGETS stdx.serialization DESTINATION ${output_triple_name}_${CODENATIVE_BACKEND}${SANITIZER_SUBPATH}/static/stdx)

make_cangjie_lib(
    serialization.serialization IS_SHARED
    DEPENDS cangjie${BACKEND_TYPE}Serialization
    CODIRA_STD_LIB_LINK
        std-core
        std-collection
        std-sort
        std-math
    OBJECTS ${output_code_object_dir}/stdx/serialization.serialization.o)

add_library(stdx.serialization.serialization STATIC ${output_code_object_dir}/stdx/serialization.serialization.o)
set_target_properties(stdx.serialization.serialization PROPERTIES LINKER_LANGUAGE C)
install(TARGETS stdx.serialization.serialization
        DESTINATION ${output_triple_name}_${CODENATIVE_BACKEND}${SANITIZER_SUBPATH}/static/stdx)

make_cangjie_lib(
    encoding.json IS_SHARED
    DEPENDS cangjie${BACKEND_TYPE}Json stdx.encoding.jsonFFI
    CODIRA_STDX_LIB_DEPENDS  serialization.serialization
    CODIRA_STD_LIB_LINK
        std-core
        std-collection
        std-convert
        std-sort
        std-math
    OBJECTS ${output_code_object_dir}/stdx/encoding.json.o
    FLAGS -lstdx.encoding.jsonFFI)
get_target_property(JSONFFI_OBJS stdx.encoding.jsonFFI SOURCES)

add_library(stdx.encoding.json STATIC ${JSONFFI_OBJS} ${output_code_object_dir}/stdx/encoding.json.o)
set_target_properties(stdx.encoding.json PROPERTIES LINKER_LANGUAGE C)
install(TARGETS stdx.encoding.json DESTINATION ${output_triple_name}_${CODENATIVE_BACKEND}${SANITIZER_SUBPATH}/static/stdx)

make_cangjie_lib(
    encoding.json.stream IS_SHARED
    DEPENDS cangjie${BACKEND_TYPE}JsonStream
        stdx.encoding.json.streamFFI
        stdx.encoding.jsonFFI
    CODIRA_STD_LIB_LINK
        std-core
        std-collection
        std-io
        std-math.numeric
        std-math
        std-time
        std-sort
        std-convert
    OBJECTS ${output_code_object_dir}/stdx/encoding.json.stream.o
    FLAGS
        -lstdx.encoding.json.streamFFI
        -lstdx.encoding.jsonFFI)
get_target_property(JSONFFI_OBJS stdx.encoding.jsonFFI SOURCES)

add_library(
    stdx.encoding.json.stream STATIC
    $<TARGET_OBJECTS:stdx.encoding.json.streamFFI-objs>
    ${JSONFFI_OBJS}
    ${output_code_object_dir}/stdx/encoding.json.stream.o)
install(TARGETS stdx.encoding.json.stream DESTINATION ${output_triple_name}_${CODENATIVE_BACKEND}${SANITIZER_SUBPATH}/static/stdx)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Defined all xxx_DEPENDENCIES, which are for all backends.
include(LibraryDependencies)

add_cangjie_library(
    cangjie${BACKEND_TYPE}Stdx
    NO_SUB_PKG
    IS_STDXLIB
    IS_PACKAGE
    IS_CODENATIVE_BACKEND
    OUTPUT_NAME "cangjieStdx"
    PACKAGE_NAME "stdx"
    MODULE_NAME ""
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/stdx
    DEPENDS ${STDX_DEPENDENCIES})

add_cangjie_library(
    cangjie${BACKEND_TYPE}Encoding
    NO_SUB_PKG
    IS_STDXLIB
    IS_PACKAGE
    IS_CODENATIVE_BACKEND
    OUTPUT_NAME "cangjieEncoding"
    PACKAGE_NAME "encoding"
    MODULE_NAME "stdx"
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/stdx/encoding
    DEPENDS ${ENCODING_DEPENDENCIES})

add_cangjie_library(
    cangjie${BACKEND_TYPE}StdxSerialization
    NO_SUB_PKG
    IS_STDXLIB
    IS_PACKAGE
    IS_CODENATIVE_BACKEND
    PACKAGE_NAME "serialization"
    MODULE_NAME "stdx"
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/stdx/serialization
    DEPENDS ${SERIALIZATIONBASE_DEPENDENCIES})

add_cangjie_library(
    cangjie${BACKEND_TYPE}Serialization
    NO_SUB_PKG
    IS_STDXLIB
    IS_PACKAGE
    IS_CODENATIVE_BACKEND
    PACKAGE_NAME "serialization.serialization"
    MODULE_NAME "stdx"
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/stdx/serialization/serialization
    DEPENDS ${SERIALIZATION_DEPENDENCIES})

add_cangjie_library(
    cangjie${BACKEND_TYPE}Json
    IS_STDXLIB
    IS_PACKAGE
    IS_CODENATIVE_BACKEND
    PACKAGE_NAME "encoding.json"
    MODULE_NAME "stdx"
    SOURCES ${JSON_SRCS}
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/stdx/encoding/json
    DEPENDS ${ENCODING_JSON_DEPENDENCIES})

add_cangjie_library(
    cangjie${BACKEND_TYPE}JsonStream
    NO_SUB_PKG
    IS_STDXLIB
    IS_PACKAGE
    IS_CODENATIVE_BACKEND
    OUTPUT_NAME "cangjieJson"
    PACKAGE_NAME "encoding.json.stream"
    MODULE_NAME "stdx"
    SOURCES ${JSON_STREAM_SRCS}
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/stdx/encoding/json/stream
    DEPENDS ${ENCODING_JSON_STREAM_DEPENDENCIES})