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

cmake_minimum_required(VERSION 3.16.3)

project(codetrace-recover)

set(CMAKE_CXX_STANDARD 17)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")
include(CodiraTarget)
include(ExternalProject)

option(DARWIN_SET_RPATH "set rpath on darwin, not used on other platform." OFF)

set(ENV{ZERO_AR_DATE} "1")
set(CMAKE_C_COMPILER_FORCED ON)
set(CMAKE_CXX_COMPILER_FORCED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fstack-protector-all -fPIE -D_FORTIFY_SOURCE=2 -ftrapv")
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -fstack-protector-all -fPIE -D_FORTIFY_SOURCE=2 -ftrapv")
set(CMAKE_SKIP_BUILD_RPATH TRUE)

if(NOT "${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
    find_program(CMAKE_AR llvm-ar REQUIRED)
    find_program(CMAKE_RANLIB llvm-ranlib REQUIRED)
endif()

if("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
    set(CMAKE_CXX_ARCHIVE_CREATE "<CMAKE_AR> -crD <TARGET> <LINK_FLAGS> <OBJECTS>")
    set(CMAKE_CXX_ARCHIVE_APPEND "<CMAKE_AR> -rD <TARGET> <LINK_FLAGS> <OBJECTS>")
    set(CMAKE_CXX_ARCHIVE_FINISH "<CMAKE_RANLIB> -D <TARGET>")
endif()

include(CheckSymbolExists)
check_symbol_exists(memcpy_s "string.h" HAVE_MEMCPY_S)
if(NOT HAVE_MEMCPY_S)
    # boundscheck can be obtained from cangjie toolchain
    add_library(boundscheck SHARED IMPORTED)
    set_target_properties(boundscheck PROPERTIES
        IMPORTED_LOCATION "${CODIRA_DYNAMIC_LIBRARY_PATH}/libboundscheck.so"
    )
endif()

# build demangler
set(DEMANGLER_INSTALL_PATH ${CMAKE_BINARY_DIR}/cangjie-demangler)
set(CODIRA_DEMANGLER_CMAKE_ARGS
    -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
    -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
    -DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}
    -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}
    -DCMAKE_INSTALL_PREFIX=${DEMANGLER_INSTALL_PATH}
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE})

if(CMAKE_CROSSCOMPILING)
    set(CODIRA_DEMANGLER_CMAKE_ARGS
        ${CODIRA_DEMANGLER_CMAKE_ARGS}
        -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
        -DCMAKE_SYSROOT=${CMAKE_SYSROOT}
    )
endif()

ExternalProject_Add(
    cangjie-demangler
    SOURCE_DIR ${CMAKE_SOURCE_DIR}/../third_party/demangler
    CMAKE_ARGS ${CODIRA_DEMANGLER_CMAKE_ARGS})

add_library(cangjie-demangle STATIC IMPORTED)
add_dependencies(cangjie-demangle cangjie-demangler)
# create a dummy path to make cmake happy
file(MAKE_DIRECTORY "${DEMANGLER_INSTALL_PATH}/include")
set_target_properties(cangjie-demangle PROPERTIES
    IMPORTED_LOCATION "${DEMANGLER_INSTALL_PATH}/lib/libcangjie-demangle.a"
    INTERFACE_INCLUDE_DIRECTORIES "${DEMANGLER_INSTALL_PATH}/include"
)

add_library(demangle_wrapper STATIC src/native/DemangleWrapper.cpp)
target_link_libraries(demangle_wrapper PRIVATE cangjie-demangle)
if(NOT HAVE_MEMCPY_S)
    target_compile_definitions(demangle_wrapper PRIVATE USE_BOUNDSCHECK_LIBRARY)
    target_link_libraries(demangle_wrapper PRIVATE boundscheck)
endif()
target_include_directories(demangle_wrapper PRIVATE ${RUNTIME_REPO_PATH}/src/Demangler)

# build codetrace-recover
set(code_compile_options)
if("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
    if(RUNTIME_RPATH)
        list(APPEND code_compile_options --link-options=\"--disable-new-dtags -rpath ${RUNTIME_RPATH}\")
    endif()
    list(APPEND code_compile_options -lstdc++ -lgcc_s --link-options=\"-z noexecstack -z relro -z now -s\")
elseif("${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
    if(RUNTIME_RPATH)
        list(APPEND code_compile_options --link-options=\"-rpath ${RUNTIME_RPATH}\")
    endif()
    list(APPEND code_compile_options -lc++ -lc++abi)
elseif("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
    if(CMAKE_CROSSCOMPILING)
        list(APPEND code_compile_options
            --trimpath ${CMAKE_SOURCE_DIR}
            --link-options="--no-insert-timestamp")
    endif()
    list(APPEND code_compile_options -lc++ -lunwind)
else()
    message(FATAL_ERROR "Unknown system")
endif()

add_cangjie_executable(codetrace-recover
    SOURCES src/codetrace-recover.code
    COMPILE_OPTIONS ${code_compile_options}
    DEPENDS demangle_wrapper cangjie-demangle
)
