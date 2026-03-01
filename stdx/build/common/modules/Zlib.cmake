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

message(STATUS "Configuring zlib library...")

set(ZLIB_BUILD_DIR ${CMAKE_BINARY_DIR}/third_party/zlib-build)
set(ZLIB_INSTALL_DIR ${CMAKE_BINARY_DIR}/third_party/zlib)
file(MAKE_DIRECTORY ${ZLIB_BUILD_DIR})

set(BUILD_TYPE ${CMAKE_BUILD_TYPE})

if (BUILD_TYPE STREQUAL "Debug")
    unset(BUILD_TYPE)
endif()

if(IOS)
    execute_process(
        COMMAND
            ${CMAKE_COMMAND}
            -G Ninja
            -DCODIRA_TARGET_TOOLCHAIN=${CODIRA_TARGET_TOOLCHAIN}
            -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
            -DCMAKE_SYSROOT=${CODIRA_TARGET_SYSROOT}
            -DCMAKE_ANDROID_NDK=${CMAKE_ANDROID_NDK}
            -DCMAKE_ANDROID_API=${CMAKE_ANDROID_API}
            -DCMAKE_BUILD_WITH_INSTALL_RPATH=ON
            -DCMAKE_INSTALL_LIBDIR=lib
            -DCMAKE_BUILD_TYPE=${BUILD_TYPE}
            -DCMAKE_SHARED_LINKER_FLAGS=${CMAKE_SHARED_LINKER_FLAGS}
            -DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}
            -DBUILD_SHARED_LIBS=ON
            -DCMAKE_INSTALL_PREFIX=${ZLIB_INSTALL_DIR}
            ${CODIRA_ZLIB_SOURCE_DIR}
        WORKING_DIRECTORY ${ZLIB_BUILD_DIR}
        RESULT_VARIABLE config_result
        OUTPUT_VARIABLE config_stdout
        ERROR_VARIABLE config_stderr)
else()
    execute_process(
        COMMAND
            ${CMAKE_COMMAND}
            -G Ninja
            -DCODIRA_TARGET_TOOLCHAIN=${CODIRA_TARGET_TOOLCHAIN}
            -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
            -DCMAKE_SYSROOT=${CODIRA_TARGET_SYSROOT}
            -DCMAKE_ANDROID_NDK=${CMAKE_ANDROID_NDK}
            -DCMAKE_ANDROID_API=${CMAKE_ANDROID_API}
            -DCMAKE_BUILD_WITH_INSTALL_RPATH=ON
            -DCMAKE_INSTALL_LIBDIR=lib
            -DCMAKE_OSX_DEPLOYMENT_TARGET=12.0.0
            -DCMAKE_BUILD_TYPE=${BUILD_TYPE}
            -DCMAKE_SHARED_LINKER_FLAGS=${CMAKE_SHARED_LINKER_FLAGS}
            -DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}
            -DBUILD_SHARED_LIBS=ON
            -DCMAKE_INSTALL_PREFIX=${ZLIB_INSTALL_DIR}
            ${CODIRA_ZLIB_SOURCE_DIR}
        WORKING_DIRECTORY ${ZLIB_BUILD_DIR}
        RESULT_VARIABLE config_result
        OUTPUT_VARIABLE config_stdout
        ERROR_VARIABLE config_stderr)
endif()

if(NOT ${config_result} STREQUAL "0")
    message(STATUS "${config_stdout}")
    message(STATUS "${config_stderr}")
    message(FATAL_ERROR "Configuring zlib Failed!")
endif()

message(STATUS "Building zlib libraries...")
execute_process(
    COMMAND ${CMAKE_COMMAND} --build .
    WORKING_DIRECTORY ${ZLIB_BUILD_DIR}
    RESULT_VARIABLE build_result
    OUTPUT_VARIABLE build_stdout
    ERROR_VARIABLE build_stderr)
if(NOT ${build_result} STREQUAL "0")
    message(STATUS "${config_stdout}")
    message(STATUS "${config_stderr}")
    message(STATUS "${build_stdout}")
    message(STATUS "${build_stderr}")
    message(FATAL_ERROR "Building zlib Failed!")
endif()

message(STATUS "Installing zlib headers and libraries to Codira library source...")
execute_process(
    COMMAND ${CMAKE_COMMAND} --install .
    WORKING_DIRECTORY ${ZLIB_BUILD_DIR}
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
    message(FATAL_ERROR "Installing zlib Failed!")
endif()