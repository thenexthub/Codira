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

cmake_minimum_required(VERSION 3.16.5)
project(stdx)
# Add path for custom CMake modules and add cmake modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/build/common/modules")
include(AddCodiraSource)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_INCLUDE_SYSTEM_FLAG_CXX)
    set(CXX_SYSTEM_INCLUDE_CONFIGURATION_FLAG /experimental:external /external:W0)
    set(CMAKE_INCLUDE_SYSTEM_FLAG_CXX /external:I)
endif()

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "No build type selected, default to Debug")
    set(CMAKE_BUILD_TYPE
        "Debug"
        CACHE STRING "Build type (default Debug)" FORCE)
endif()

string(TOUPPER "${CMAKE_BUILD_TYPE}" uppercase_CMAKE_BUILD_TYPE)

if(CMAKE_BUILD_TYPE AND NOT ("${uppercase_CMAKE_BUILD_TYPE}" MATCHES "^(DEBUG|RELEASE|RELWITHDEBINFO)$"))
    message(FATAL_ERROR "Invalid value for CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
endif()


set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_SKIP_INSTALL_ALL_DEPENDENCY TRUE)

option(CODIRA_CODEGEN_CODENATIVE_BACKEND "Build a version for CODENATIVE backend" ON)
option(CODIRA_SKIP_FIND_OPENSSL "Do not look for OpenSSL package, OpenSSL library will be searched by linker" OFF)

message(STATUS "Build type for the current project: ${CMAKE_BUILD_TYPE}")


if(CODIRA_CODEGEN_CODENATIVE_BACKEND)
    add_compile_definitions(CODIRA_CODEGEN_CODENATIVE_BACKEND)
endif()

if(OHOS)
    # This is a CMake bug until 3.22, where MINGW is wrongly set
    # when targeting other platforms on a Windows machine with a MinGW toolchain.
    # See more at CMake issue tracker #22647.
    set(MINGW 0)
endif()

if(NOT TRIPLE)
    string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}-${CMAKE_SYSTEM_NAME}-gnu" TRIPLE)
endif()

if(MINGW)
    set(TRIPLE x86_64-w64-mingw32)
endif()

if(NOT TARGET_TRIPLE_DIRECTORY_PREFIX)
    if(IOS AND IOS_PLATFORM MATCHES "SIMULATOR")
        set(EXTRA_OS_SUFFIX _simulator)
    endif()
    string(REPLACE "-" "_" TARGET_TRIPLE_DIRECTORY_PREFIX "${CMAKE_SYSTEM_NAME}${EXTRA_OS_SUFFIX}_${CMAKE_SYSTEM_PROCESSOR}")
    string(TOLOWER "${TARGET_TRIPLE_DIRECTORY_PREFIX}" TARGET_TRIPLE_DIRECTORY_PREFIX)
endif()

if(CMAKE_CROSSCOMPILING)
    message(STATUS "CROSS COMPILING libs from ${CMAKE_HOST_SYSTEM_PROCESSOR}-${CMAKE_HOST_SYSTEM_NAME} to ${TRIPLE}")
endif()

include(SetupAr)

message(STATUS "Building with target=${TRIPLE}")

if(MACOS)
    include(ReadDarwinSDKInfo)
    set(CMAKE_OSX_DEPLOYMENT_TARGET 12.0.0)
endif()

if(CODIRA_INCLUDE)
    foreach(include_path ${CODIRA_INCLUDE})
        include_directories(${include_path})
    endforeach()
endif()

if(NOT (CMAKE_BUILD_TYPE MATCHES Debug))
    add_definitions(-DNDEBUG)
endif()

if(CMAKE_BUILD_TYPE MATCHES Release)
    add_definitions(-DRELEASE)
endif()

if(BUILD_GCC_TOOLCHAIN AND (CMAKE_C_COMPILER_ID STREQUAL "Clang" OR (CMAKE_CXX_COMPILER_ID STREQUAL "Clang"))
)# It is clang that takes --gcc-toolchain options.
    message(STATUS "Add compile option for clang, --gcc-toolchain=${BUILD_GCC_TOOLCHAIN}")
    add_compile_options(--gcc-toolchain=${BUILD_GCC_TOOLCHAIN})
    add_link_options(--gcc-toolchain=${BUILD_GCC_TOOLCHAIN})
endif()

foreach(libpath ${CODIRA_TARGET_LIB})
    add_link_options("-L${libpath}")
endforeach()

add_compile_options(${CXX_SYSTEM_INCLUDE_CONFIGURATION_FLAG})


if(CODIRA_LINK_JOB_POOL AND CMAKE_GENERATOR MATCHES "Ninja")
    set_property(GLOBAL APPEND PROPERTY JOB_POOLS link_job_pool=${CODIRA_LINK_JOB_POOL})
    set(CMAKE_JOB_POOL_LINK link_job_pool)
endif()


if(WIN32)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/lib)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/lib)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/lib)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/lib)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin)
endif()
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if(NOT CODIRA_SKIP_FIND_OPENSSL AND NOT ANDROID)
    include(PrepareOpenSSL)
else()
    # OpenSSL library wil be searched by the linker at build time
    set(OPENSSL_CRYPTO_LINK_OPTIONS -lcrypto)
    set(OPENSSL_SSL_LINK_OPTIONS -lssl -lcrypto)
endif()

string(TOLOWER "${CMAKE_SYSTEM_NAME}" TARGET_OS)
add_compile_definitions(__${TARGET_OS}__)

if(NOT DEFINED CODIRA_ZLIB_SOURCE_DIR)
    set(CODIRA_ZLIB_SOURCE_DIR ${CMAKE_SOURCE_DIR}/third_party/zlib)
endif()


set(GCC_TOOLCHAIN_FLAG "")
if(MINGW)
    # There will be lots of warnings when compiling backend with MinGW toolchain, temporarily suppress them.
    set(GCC_TOOLCHAIN_FLAG "${GCC_TOOLCHAIN_FLAG} -w")
endif()
if(BUILD_GCC_TOOLCHAIN)
    message(STATUS "Set GCC toolchain: ${BUILD_GCC_TOOLCHAIN}")
    set(GCC_TOOLCHAIN_FLAG "${GCC_TOOLCHAIN_FLAG} --gcc-toolchain=\"${BUILD_GCC_TOOLCHAIN}\"")
endif()

if(NOT EXISTS ${CODIRA_ZLIB_SOURCE_DIR})
    set(REPOSITORY_PATH https://github.com/madler/zlib.git)
    message(STATUS "Set Zlib REPOSITORY_PATH: ${REPOSITORY_PATH}")
    execute_process(
        COMMAND git clone --branch develop ${REPOSITORY_PATH}
    )
endif()
include(Zlib)

set(BOUNDSCHECK ${CMAKE_SOURCE_DIR}/third_party/boundscheck-v1.1.16)

# Provide secure functions.
if(NOT EXISTS ${BOUNDSCHECK})
    set(REPOSITORY_PATH https://gitcode.com/openharmony/third_party_bounds_checking_function.git)
    message(STATUS "Set boundscheck REPOSITORY_PATH: ${REPOSITORY_PATH}")
    execute_process(
        COMMAND git clone --branch OpenHarmony-v6.0-Release ${REPOSITORY_PATH} ${BOUNDSCHECK}
    )
endif()
file(COPY ${CMAKE_MODULE_PATH}/BoundsCheck.cmake DESTINATION ${BOUNDSCHECK}/)
file(RENAME ${BOUNDSCHECK}/BoundsCheck.cmake ${BOUNDSCHECK}/CMakeLists.txt)

if(CODIRA_CODEGEN_CODENATIVE_BACKEND)
    add_subdirectory(third_party/boundscheck-v1.1.16)
    install(
        TARGETS boundscheck
        RUNTIME
            DESTINATION
            ${TARGET_TRIPLE_DIRECTORY_PREFIX}_${CODENATIVE_BACKEND}${SANITIZER_SUBPATH}/dynamic/stdx # RUNTIME includes DLLs. This takes effect on Windows.
        LIBRARY
            DESTINATION
            ${TARGET_TRIPLE_DIRECTORY_PREFIX}_${CODENATIVE_BACKEND}${SANITIZER_SUBPATH}/dynamic/stdx # LIBRARY includes shared libraries except DLLs.
    )
    install(TARGETS boundscheck-static DESTINATION ${TARGET_TRIPLE_DIRECTORY_PREFIX}_${CODENATIVE_BACKEND}${SANITIZER_SUBPATH}/static/stdx)
endif()

add_subdirectory(src)