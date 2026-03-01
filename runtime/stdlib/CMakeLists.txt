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
project(cangjie)
# add path for custom CMake modules and add cmake modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")
include(AddCodiraSource)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CODENATIVE_BACKEND "codenative")

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

string(TOLOWER "${CMAKE_BUILD_TYPE}" runtime_build_mode)
set(target_system_name ${CMAKE_SYSTEM_NAME})
if(OHOS)
    set(target_system_name "linux_ohos")
endif()
if(IOS AND IOS_PLATFORM MATCHES "SIMULATOR")
    set(target_system_name "ios_simulator")
endif()
if(ANDROID)
    if(CMAKE_ANDROID_API EQUAL 26)
        set(target_folder_name "linux_android")
    else()
        set(target_folder_name "linux_android${CMAKE_ANDROID_API}")
    endif()
elseif(OHOS)
    set(target_folder_name "linux_ohos")
else()
    set(target_folder_name "${target_system_name}")
endif()
string(TOLOWER "${target_folder_name}_${runtime_build_mode}_${CMAKE_SYSTEM_PROCESSOR}" dir1)
string(TOLOWER "${target_folder_name}_${CMAKE_SYSTEM_PROCESSOR}_${CODENATIVE_BACKEND}" dir2)
set(CODIRA_RUNTIME_OUTPUT_PATH "")

foreach(lib_path ${CODIRA_TARGET_LIB})
    file(GLOB MATCHING_RUNTIME_FILES "${lib_path}/common/${dir1}/lib/${dir2}/*cangjie-aio*")
    if(MATCHING_RUNTIME_FILES)
        set(CODIRA_RUNTIME_OUTPUT_PATH ${lib_path})
        break()
    endif()
endforeach()

set(RUNTIME_COMMON_LIB_DIR ${CODIRA_RUNTIME_OUTPUT_PATH}/common/${dir1}/lib/${dir2} CACHE STRING "runtime common path lib")

string(TOUPPER "${CMAKE_BUILD_TYPE}" uppercase_CMAKE_BUILD_TYPE)

if(CMAKE_BUILD_TYPE AND NOT ("${uppercase_CMAKE_BUILD_TYPE}" MATCHES "^(DEBUG|RELEASE|RELWITHDEBINFO|MINSIZEREL)$"))
    message(FATAL_ERROR "Invalid value for CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
endif()

if(NOT DEFINED CODIRA_PCRE2_SOURCE_DIR)
    set(CODIRA_PCRE2_SOURCE_DIR ${CMAKE_SOURCE_DIR}/third_party/pcre2)
endif()
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


set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_SKIP_INSTALL_ALL_DEPENDENCY TRUE)

option(CODIRA_DOWNLOAD_DEPS "Download binary of llvm, dopra, runtime, jet" ON)

option(CODIRA_CODEGEN_CODENATIVE_BACKEND "Build a version for CODENATIVE backend" ON)
option(CODIRA_ENABLE_CCACHE "Build cangjie with ccache" OFF)
option(CODIRA_SKIP_FIND_OPENSSL "Do not look for OpenSSL package, OpenSSL library will be searched by linker" OFF)
message(STATUS "Build type for the current project: ${CMAKE_BUILD_TYPE}")

if(CODIRA_ENABLE_CCACHE
   OR CMAKE_BUILD_TYPE MATCHES Debug
   OR CMAKE_BUILD_TYPE MATCHES RelWithDebInfo)
    include(cmake/optional/CodiraCCache.cmake)
endif()

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
    string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}-${target_system_name}-gnu" TRIPLE)
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

# BUILD_GCC_TOOLCHAIN specifies toolchain for codec, stdlib and BE at the same time, which means
# that it is used for both targets for HOST (e.g. codec, llc) and targets for TARGET
# (e.g. stdlib, runtime). In the case that we need to build a codec for a host which is not a target,
# we must build codec and stdlib separately for specifying different toolchains for them.
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

include_directories($ENV{CODIRA_HOME}/include)

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
if(IOS AND IOS_PLATFORM MATCHES "SIMULATOR")
    set(TARGET_OS ios)
else()
    string(TOLOWER "${target_system_name}" TARGET_OS)
endif()
add_compile_definitions(__${TARGET_OS}__)

if(NOT DEFINED CODIRA_ZLIB_SOURCE_DIR)
    set(CODIRA_ZLIB_SOURCE_DIR ${CMAKE_SOURCE_DIR}/third_party/zlib)
endif()

set(CODIRA_ASAN_SUPPORT OFF)
set(CODIRA_TSAN_SUPPORT OFF)
set(CODIRA_HWASAN_SUPPORT OFF)
set(CODIRA_SANITIZER_SUPPORT_ENABLED OFF)
set(CODIRA_SANITIZER_SUPPORT "" CACHE STRING "Enable cangjie sanitizer support for cangjie libraries")
if ("${CODIRA_SANITIZER_SUPPORT}" STREQUAL "asan")
    set(CODIRA_ASAN_SUPPORT ON)
    set(CODIRA_SANITIZER_SUPPORT_ENABLED ON)
elseif ("${CODIRA_SANITIZER_SUPPORT}" STREQUAL "tsan")
    set(CODIRA_TSAN_SUPPORT ON)
    set(CODIRA_SANITIZER_SUPPORT_ENABLED ON)
elseif ("${CODIRA_SANITIZER_SUPPORT}" STREQUAL "hwasan")
    set(CODIRA_HWASAN_SUPPORT ON)
    set(CODIRA_SANITIZER_SUPPORT_ENABLED ON)
endif()

if(CODIRA_SANITIZER_SUPPORT_ENABLED)
    if(NOT CODIRA_CODEGEN_CODENATIVE_BACKEND)
        message(FATAL_ERROR "Codira with sanitizer support is supposed to be built in codenative backend")
    endif()
endif()

if(CODIRA_ASAN_SUPPORT)
    if(WIN32)
        message(FATAL_ERROR "Codira with asan support only supports on Linux/OHOS platform")
    endif()
    # here just use add_compile_options for asan instrumentation
    # do not use add_link_options(-fsanitize=address) which will link library while using gcc specifically
    add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
    set(SANITIZER_SUBPATH /asan)
elseif(CODIRA_TSAN_SUPPORT)
    if(OHOS OR CMAKE_CROSSCOMPILING)
        message(FATAL_ERROR "Codira with tsan support only supports on Linux/Windows platform")
    endif()
    if(CODIRA_SKIP_BUILD_CLANG_RT)
        message(FATAL_ERROR "You have to build compiler-rt locally")
    endif()
    set(SANITIZER_SUBPATH /tsan)
elseif(CODIRA_HWASAN_SUPPORT)
    if(WIN32)
        message(FATAL_ERROR "Codira with hwasan support only supports on Linux/OHOS platform")
    endif()
    add_compile_options(-fsanitize=hwaddress -fno-omit-frame-pointer)
    set(SANITIZER_SUBPATH /hwasan)
endif()

add_subdirectory(third_party)
add_subdirectory(schema)

add_subdirectory(libs)


if(CODIRA_CODEGEN_CODENATIVE_BACKEND)
    add_subdirectory(third_party/boundscheck-v1.1.16)
    install(
        TARGETS boundscheck
        RUNTIME
            DESTINATION
            runtime/lib/${TARGET_TRIPLE_DIRECTORY_PREFIX}_${CODENATIVE_BACKEND}${SANITIZER_SUBPATH} # RUNTIME includes DLLs. This takes effect on Windows.
        LIBRARY
            DESTINATION
            lib/${TARGET_TRIPLE_DIRECTORY_PREFIX}_${CODENATIVE_BACKEND}${SANITIZER_SUBPATH} # LIBRARY includes shared libraries except DLLs.
    )
    install(TARGETS boundscheck-static DESTINATION lib/${TARGET_TRIPLE_DIRECTORY_PREFIX}_${CODENATIVE_BACKEND}${SANITIZER_SUBPATH})
endif()
