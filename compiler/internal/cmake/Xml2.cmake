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

message(STATUS "Configuring xml2 library...")

set(XML2_BUILD_DIR ${CMAKE_BINARY_DIR}/third_party/xml2-build)
set(XML2_INSTALL_DIR ${CMAKE_BINARY_DIR}/third_party/xml2)
file(MAKE_DIRECTORY ${XML2_BUILD_DIR})

set(CMAKE_SHARED_LINKER_FLAGS "${LINK_FLAGS} ${LINK_FLAGS_BUILD_ID} ${STRIP_FLAG}")
set(XML2_SOURCE_DIR ${CMAKE_BINARY_DIR}/third_party/libxml2)
if(NOT EXISTS ${CODIRA_XML2_SOURCE_DIR})
        if(NOT EXISTS ${XML2_SOURCE_DIR})
            message(STATUS "ERROR CODE 100: Absent libxml2 source.")
    endif()
    message(STATUS "Uncompressing libxml2...")
    execute_process(COMMAND tar -C ${CMAKE_SOURCE_DIR}/third_party -xf ${XML2_SOURCE_DIR}/libxml2-2.14.0.tar.xz)
endif()

execute_process(
    COMMAND
        ${CMAKE_COMMAND}
        -G Ninja
        -DLIBXML2_WITH_PYTHON=OFF
        -DLIBXML2_WITH_ICONV=OFF
        -DLIBXML2_WITH_LZMA=OFF
        -DLIBXML2_WITH_ZLIB=OFF
        -DCUSTOM_WARNING_SETTINGS=-Wno-error
        -DCODIRA_TARGET_TOOLCHAIN=${CODIRA_TARGET_TOOLCHAIN}
        -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
        -DCMAKE_SYSROOT=${CODIRA_TARGET_SYSROOT}
        -DCMAKE_BUILD_WITH_INSTALL_RPATH=ON
        -DCMAKE_INSTALL_LIBDIR=lib
        -DCMAKE_BUILD_TYPE=Release
        -DCMAKE_SHARED_LINKER_FLAGS=${CMAKE_SHARED_LINKER_FLAGS}
        -DBUILD_SHARED_LIBS=ON
        -DCMAKE_INSTALL_PREFIX=${XML2_INSTALL_DIR}
        ${CODIRA_XML2_SOURCE_DIR}
    WORKING_DIRECTORY ${XML2_BUILD_DIR}
    RESULT_VARIABLE config_result
    OUTPUT_VARIABLE config_stdout
    ERROR_VARIABLE config_stderr)
if(NOT ${config_result} STREQUAL "0")
    message(STATUS "${config_stdout}")
    message(STATUS "${config_stderr}")
    message(FATAL_ERROR "Configuring xml2 Failed!")
endif()

message(STATUS "Building xml2 libraries...")
execute_process(
    COMMAND ${CMAKE_COMMAND} --build .
    WORKING_DIRECTORY ${XML2_BUILD_DIR}
    RESULT_VARIABLE build_result
    OUTPUT_VARIABLE build_stdout
    ERROR_VARIABLE build_stderr)
if(NOT ${build_result} STREQUAL "0")
    message(STATUS "${config_stdout}")
    message(STATUS "${config_stderr}")
    message(STATUS "${build_stdout}")
    message(STATUS "${build_stderr}")
    message(FATAL_ERROR "Building xml2 Failed!")
endif()

message(STATUS "Installing xml2 libraries to Codira library source...")
execute_process(
    COMMAND ${CMAKE_COMMAND} --install .
    WORKING_DIRECTORY ${XML2_BUILD_DIR}
    RESULT_VARIABLE install_result
    OUTPUT_VARIABLE install_stdout
    ERROR_VARIABLE install_stderr)
if(NOT ${install_result} STREQUAL "0")
    message(STATUS "${config_stdout}")
    message(STATUS "${config_stderr}")
    message(STATUS "${build_stdout}")
    message(STATUS "${build_stderr}")
    message(STATUS "${install_stdout}")
    message(STATUS "${install_stderr}")
    message(FATAL_ERROR "Installing xml2 Failed!")
endif()
