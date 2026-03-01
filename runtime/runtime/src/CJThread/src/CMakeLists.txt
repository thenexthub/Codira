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

cmake_minimum_required(VERSION 3.14.1)

set(Python_ADDITIONAL_VERSIONS 3.7)
find_package (Python3 COMPONENTS Interpreter REQUIRED)
if(NOT Python3_FOUND)
    message(FATAL_ERROR
        "Unable to find Python interpreter, required for builds and testing.
        Please install Python or specify the Python3_EXECUTABLE CMake variable.")
endif()
if(${Python3_VERSION} VERSION_LESS 3.7 )
    message(FATAL_ERROR "Python 3.7 or newer is required. Current python version is " ${Python3_VERSION})
endif()

enable_language(ASM)

if (CMAKE_SYSTEM_NAME MATCHES "Linux")
    set(OS_PREFIX "linux")
elseif (CMAKE_SYSTEM_NAME MATCHES "Windows")
    set(OS_PREFIX "windows")
elseif (CMAKE_SYSTEM_NAME MATCHES "Darwin")
    set(OS_PREFIX "macos")
endif ()

IF(CMAKE_HOST_UNIX)
    set(CMAKE_C_COMPILE_OBJECT "<CMAKE_C_COMPILER> <DEFINES> -DVOSFILENAME='\"$(notdir \$(patsubst \"%\",%, $(abspath <SOURCE>)))\"' <INCLUDES> <FLAGS> -o <OBJECT> -c <SOURCE>")
    set(CMAKE_CXX_COMPILE_OBJECT "<CMAKE_CXX_COMPILER> <DEFINES> -DVOSFILENAME='\"$(notdir \$(patsubst \"%\",%, $(abspath <SOURCE>)))\"' <INCLUDES> <FLAGS> -o <OBJECT> -c <SOURCE>")
ELSE()
    set(CMAKE_C_COMPILE_OBJECT "<CMAKE_C_COMPILER> <DEFINES> <INCLUDES> <FLAGS> -o <OBJECT> -c <SOURCE>")
    set(CMAKE_CXX_COMPILE_OBJECT "<CMAKE_CXX_COMPILER> <DEFINES> <INCLUDES> <FLAGS> -o <OBJECT> -c <SOURCE>")
ENDIF()

add_subdirectory(base)
add_subdirectory(runtime)
add_subdirectory(sync)
add_subdirectory(syscall)
add_subdirectory(aio)
add_subdirectory(trace)

set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/../../output/temp/lib)


add_library(cangjie-thread ${LIBTYPE}
$<TARGET_OBJECTS:schedule>
$<TARGET_OBJECTS:log>
$<TARGET_OBJECTS:netpoll>
$<TARGET_OBJECTS:basetime>
$<TARGET_OBJECTS:timer>
$<TARGET_OBJECTS:syscall>
$<TARGET_OBJECTS:waitqueue>
$<TARGET_OBJECTS:sema>
$<TARGET_OBJECTS:external>
)

if ((${PLATFORM_NAME} STREQUAL "ios_cangjie" OR ${PLATFORM_NAME} STREQUAL "ios_simulator_aarch64_cangjie" OR 
     ${PLATFORM_NAME} STREQUAL "mac_x86_64_cangjie" OR ${PLATFORM_NAME} STREQUAL "mac_aarch64_cangjie" OR
     ${PLATFORM_NAME} STREQUAL "ios_cangjie_x86_64" OR ${PLATFORM_NAME} STREQUAL "ios_simulator_x86_64_cangjie") AND 
    ("${APPLE_STATIC_BUILD}" STREQUAL "true"))
    add_custom_command(TARGET cangjie-thread PRE_BUILD
        COMMAND sh ${CMAKE_SOURCE_DIR}/../../build/apple_rename_section.sh 
            "${PLATFORM_NAME}"
            "${CMAKE_C_COMPILER}"
            "$<TARGET_OBJECTS:schedule>"
            "$<TARGET_OBJECTS:log>"
            "$<TARGET_OBJECTS:netpoll>"
            "$<TARGET_OBJECTS:basetime>"
            "$<TARGET_OBJECTS:timer>"
            "$<TARGET_OBJECTS:syscall>"
            "$<TARGET_OBJECTS:waitqueue>"
            "$<TARGET_OBJECTS:sema>"
            "$<TARGET_OBJECTS:external>"
        VERBATIM
    )
endif()

if(${PLATFORM_NAME} STREQUAL "linux_aarch64_cangjie" OR ${PLATFORM_NAME} STREQUAL "linux_aarch64" OR
    ${PLATFORM_NAME} STREQUAL "linux12_x86_64_cangjie" OR ${PLATFORM_NAME} STREQUAL "linux12_x86_64_fuzz" OR
    ${PLATFORM_NAME} STREQUAL "linux12_x86_64_gcov" OR ${PLATFORM_NAME} STREQUAL "linux12_x86_64" OR
    ${PLATFORM_NAME} STREQUAL "ios_cangjie" OR ${PLATFORM_NAME} STREQUAL "ios_simulator_aarch64_cangjie" OR
    ${PLATFORM_NAME} STREQUAL "ios_cangjie_x86_64" OR ${PLATFORM_NAME} STREQUAL "ios_simulator_x86_64_cangjie" OR
    ${PLATFORM_NAME} STREQUAL "mac_x86_64_cangjie" OR ${PLATFORM_NAME} STREQUAL "mac_aarch64_cangjie" OR
    ${PLATFORM_NAME} STREQUAL "windows_x86_64" OR ${PLATFORM_NAME} STREQUAL "cross_windows_x86_64")
install(TARGETS cangjie-thread ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/${TARGET_ARCH}${SANITIZER_SUBPATH})
set(OBJCOPY_PROGRAM objcopy)
 
install(CODE "execute_process(COMMAND ${OBJCOPY_PROGRAM} -D --rename-section .text=.text_thread \
            ${CMAKE_INSTALL_PREFIX}/lib/${TARGET_ARCH}${SANITIZER_SUBPATH}/libcangjie-thread.a)"
        COMPONENT cangjie-thread)
if(CMAKE_BUILD_TYPE MATCHES "Release" OR CMAKE_BUILD_TYPE MATCHES "MinSizeRel")
    install(CODE "execute_process(COMMAND ${STRIP_PROGRAM} --strip-unneeded ${CMAKE_INSTALL_PREFIX}/lib/libcangjie-thread.a)"
        COMPONENT cangjie-thread)
endif()
endif()

if(${OS} STREQUAL "linux")
add_library(cangjie-aio ${LIBTYPE}
$<TARGET_OBJECTS:schdfd>
$<TARGET_OBJECTS:sock>
$<TARGET_OBJECTS:tcpsock>
$<TARGET_OBJECTS:domainsock>
$<TARGET_OBJECTS:udpsock>
$<TARGET_OBJECTS:rawsock>
)
elseif(${OS} STREQUAL "windows")
add_library(cangjie-aio ${LIBTYPE}
$<TARGET_OBJECTS:schdfd>
$<TARGET_OBJECTS:sock>
$<TARGET_OBJECTS:tcpsock>
$<TARGET_OBJECTS:udpsock>
$<TARGET_OBJECTS:rawsock>
)
elseif(${OS} STREQUAL "macos")
add_library(cangjie-aio ${LIBTYPE}
$<TARGET_OBJECTS:schdfd>
$<TARGET_OBJECTS:sock>
$<TARGET_OBJECTS:tcpsock>
$<TARGET_OBJECTS:domainsock>
$<TARGET_OBJECTS:udpsock>
$<TARGET_OBJECTS:rawsock>
)
elseif(${OS} STREQUAL "ios")
add_library(cangjie-aio ${LIBTYPE}
$<TARGET_OBJECTS:schdfd>
$<TARGET_OBJECTS:sock>
$<TARGET_OBJECTS:tcpsock>
$<TARGET_OBJECTS:domainsock>
$<TARGET_OBJECTS:udpsock>
$<TARGET_OBJECTS:rawsock>
)
endif()

add_library(codethread-trace ${LIBTYPE}
$<TARGET_OBJECTS:trace>
)
install(TARGETS cangjie-aio ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/${TARGET_ARCH})
