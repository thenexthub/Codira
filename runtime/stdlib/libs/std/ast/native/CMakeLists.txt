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

# Generate cpp code for ast using flatc with cpp backend.
set(SCHEMA_SRC ${CMAKE_CURRENT_SOURCE_DIR}/../../../../schema/NodeFormat.fbs)
get_filename_component(FILENAME ${SCHEMA_SRC} NAME_WE)
set(FLATC_EXECUTABLE "${CMAKE_BINARY_DIR}/bin/flatc")
set(FLATBUFFER_SRC ${CMAKE_CURRENT_SOURCE_DIR}/../../../../third_party/flatbuffers)

if(IOS)
    get_directory_property(SRC_COMPILE_OPTIONS COMPILE_OPTIONS)
    list(REMOVE_ITEM SRC_COMPILE_OPTIONS "--target=${TRIPLE}")
    set_directory_properties(PROPERTIES COMPILE_OPTIONS "${SRC_COMPILE_OPTIONS}")
    add_compile_options(--target=${CXX_COMPATIABLE_TRIPLE})
endif()

# Generate cangjie code for ast using flatc with cangjie backend.
set(FLATC_CPP_OUTPUT "${CMAKE_BINARY_DIR}/include/flatbuffers/${FILENAME}_generated.h")
add_custom_command(
    OUTPUT ${FLATC_CPP_OUTPUT}
    COMMAND ${FLATC_EXECUTABLE} ARGS --no-warnings -c -o "${CMAKE_BINARY_DIR}/include/flatbuffers" ${SCHEMA_SRC}
    DEPENDS flatbuffers ${SCHEMA_SRC} # flatc defined in flatbuffers project: add_executable(flatc..)
    COMMENT "generate cpp file using flatc"
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

set(FLATC_CODE_OUTPUT "${CMAKE_BINARY_DIR}/${FILENAME}_generated.code")
set(FLATBUFFER_DEPEND_CODE_DIR ${FLATBUFFER_SRC}/cangjie)

add_custom_command(
    OUTPUT ${FLATC_CODE_OUTPUT}
    COMMAND ${FLATC_EXECUTABLE} ARGS --no-warnings --cangjie -o "${CMAKE_BINARY_DIR}" ${SCHEMA_SRC}
    DEPENDS flatbuffers ${SCHEMA_SRC} # flatc defined in flatbuffers project: add_executable(flatc..)

    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${FLATBUFFER_DEPEND_CODE_DIR}/table.code
            ${CMAKE_SOURCE_DIR}/libs/std/ast/
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${FLATBUFFER_DEPEND_CODE_DIR}/flatbuffer_object.code
            ${CMAKE_SOURCE_DIR}/libs/std/ast/
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${FLATBUFFER_DEPEND_CODE_DIR}/decode.code
            ${CMAKE_SOURCE_DIR}/libs/std/ast/
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${FLATBUFFER_DEPEND_CODE_DIR}/builder.code
            ${CMAKE_SOURCE_DIR}/libs/std/ast/
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${FLATBUFFER_DEPEND_CODE_DIR}/constants.code
            ${CMAKE_SOURCE_DIR}/libs/std/ast/
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${FLATBUFFER_DEPEND_CODE_DIR}/exception.code
            ${CMAKE_SOURCE_DIR}/libs/std/ast/flatc_exception.code
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_BINARY_DIR}/${FILENAME}_generated.code
        ${CMAKE_SOURCE_DIR}/libs/std/ast/${FILENAME}_generated.code
    COMMENT "Copy cangjie library files to libs")

add_custom_target(FLATC_OUTPUTS ALL DEPENDS ${FLATC_CODE_OUTPUT} ${FLATC_CPP_OUTPUT})

file(GLOB ast_ffi_src
    ./ast_api.cpp)
set(libname cangjie-std-astFFI) 
add_library(${libname}-objs OBJECT ${ast_ffi_src})
set_target_properties(${libname}-objs PROPERTIES COMPILE_FLAGS "-fPIC")
add_dependencies(${libname}-objs FLATC_OUTPUTS)

set(CODIRA_INC_DIR)
set(AST_SUPPORT_LIB_DIR)
# using CODIRA_HOME or cangjie source directory
if(DEFINED ENV{CODIRA_HOME})
    string(TOLOWER ${TARGET_TRIPLE_DIRECTORY_PREFIX}_${CODENATIVE_BACKEND} output_code_lib_dir)
    set(AST_SUPPORT_LIB_DIR $ENV{CODIRA_HOME}/lib/${output_code_lib_dir})
    set(CODIRA_INC_DIR $ENV{CODIRA_HOME}/include)
    if(NOT EXISTS "${CODIRA_INC_DIR}/cangjie")
        # using local cangjie source
        set(CODIRA_INC_DIR ~/dev/cangjie/cangjie/include)
        if(NOT EXISTS "${CODIRA_INC_DIR}/cangjie")
            message(FATAL_ERROR "NOT FOUND CODIRA HEADERS")
        endif()
    endif()
else()
    message(FATAL_ERROR "NOT FOUND CODEC")
endif()

target_compile_options(${libname}-objs PRIVATE ${CMAKE_C_COVERAGE_FLAGS})
if(CODIRA_BUILD_STDLIB_WITH_COVERAGE)
    target_compile_options(${libname}-objs PUBLIC "-fno-exceptions")
    target_compile_definitions(${libname}-objs PRIVATE CODIRA_ENABLE_GCOV)
endif()
target_include_directories(${libname}-objs PRIVATE ${CODIRA_INC_DIR} ${CMAKE_BINARY_DIR}/include)

set(TARGET_AR ar)
if(MINGW OR DARWIN)
    set(TARGET_AR ${CMAKE_AR})
elseif(CMAKE_CROSSCOMPILING)
    if(IOS)
        set(TARGET_AR ${CODIRA_TARGET_TOOLCHAIN}/ar)
    elseif(${CMAKE_C_COMPILER_ID} STREQUAL "Clang")
        set(TARGET_AR ${CODIRA_TARGET_TOOLCHAIN}/llvm-ar)
    else()
        set(TARGET_AR ${CODIRA_TARGET_TOOLCHAIN}/${TRIPLE}-ar)
    endif()
endif()

set(AST_FFI_LIB ${CMAKE_BINARY_DIR}/lib${libname}.a)

# add ast_api.obj to ast-support.a, because the same-named files will be overwritten when decompressing ast-suppor.a.
add_custom_command(
    OUTPUT ${AST_FFI_LIB}
    COMMAND ${CMAKE_COMMAND} -E copy 
        $ENV{CODIRA_HOME}/lib/${output_code_lib_dir}/libcangjie-ast-support.a
        ${AST_FFI_LIB}
    COMMAND ${TARGET_AR} qD ${AST_FFI_LIB} $<TARGET_OBJECTS:${libname}-objs>
    DEPENDS ${libname}-objs
)

if(CODIRA_BUILD_STDLIB_WITH_COVERAGE)
    target_compile_options(${libname} PUBLIC "-fno-exceptions")
    target_compile_definitions(${libname} PRIVATE CODIRA_ENABLE_GCOV)
endif()

add_custom_target(AST_SUPPORT ALL DEPENDS ${AST_FFI_LIB})

# install for static lib
install_ast_library_ffi(${AST_FFI_LIB})
