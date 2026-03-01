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

set(MINGW_PACKAGE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/binary/windows-x86_64-mingw.tar.gz)

if(NOT EXISTS ${MINGW_PACKAGE_PATH})
    # Explicitly Download the MinGW binary package from the lfs repo if it is not found in the build directory or third_party/binary.
    # There are 2 situations where there is no need to download it from here:
    # 1. When building release, the mingw package will be built from source and copied to third_party/binary.
    # 2. It has been already downloaded along with the whole binary-lfs repo (i.e. "binary-deps" target).
    message(STATUS "Set binary-deps REPOSITORY_PATH: ${REPOSITORY_PATH}")
    # In order to download contents from lfs as few as possible, shallowly clone the repo without any object, then pull the MinGW package only.
    ExternalProject_Add(
        binary-deps-mingw
        DOWNLOAD_COMMAND ${CMAKE_COMMAND} -E env GIT_LFS_SKIP_SMUDGE=1 git clone ${REPOSITORY_PATH} binary-deps-mingw -b
                         dev --depth 1
        COMMAND ${CMAKE_COMMAND} -E chdir <SOURCE_DIR> git lfs pull --include=windows-x86_64-mingw.tar.gz
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
        INSTALL_COMMAND "")
    externalproject_get_property(binary-deps-mingw SOURCE_DIR)
    set(BINARY_DEPS_MINGW_DIR ${SOURCE_DIR})
else()
    # The binary repo has been already downloaded, just create a dummy target.
    add_custom_target(
        binary-deps-mingw ALL
        DEPENDS $<TARGET_NAME_IF_EXISTS:binary-deps>
        COMMENT "Add a target for binary-deps-mingw")
    set(BINARY_DEPS_MINGW_DIR $<IF:$<BOOL:${BINARY_DEPS_DIR}>,${BINARY_DEPS_DIR},${CMAKE_CURRENT_SOURCE_DIR}/binary>)
endif()

set(MINGW_PATH ${BINARY_DEPS_MINGW_DIR}/mingw)
# When building codec, the running platform of binutils should follow the host of that codec.
# When building libs, the running platform of binutils should follow the host platform.
if(CODIRA_BUILD_CODEC)
    set(BINUTILS_DIR ${MINGW_PATH}/bin/${CMAKE_SYSTEM_NAME}/${CMAKE_SYSTEM_PROCESSOR}/)
elseif(CODIRA_BUILD_STD_SUPPORT)
    set(BINUTILS_DIR ${MINGW_PATH}/bin/${CMAKE_HOST_SYSTEM_NAME}/${CMAKE_HOST_SYSTEM_PROCESSOR}/)
endif()
set(NATIVE_CODIRA_BUILD_PATH ${CMAKE_BINARY_DIR})
# If cross-compiling libs, MinGW toolchain should be placed in the native build directory,
# in order to be easily found by the native codec.
if(CODIRA_BUILD_STD_SUPPORT)
    set(NATIVE_CODIRA_BUILD_PATH ${CMAKE_BINARY_DIR}/../build)
endif()

if(CODIRA_BUILD_CODEC)
    set(COPY_DLL_COMMAND
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${MINGW_PATH}/dll/ ${CMAKE_BINARY_DIR}/third_party/llvm/bin/)
endif()

add_custom_command(
    TARGET binary-deps-mingw
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${MINGW_PATH}
    COMMAND tar -C ${MINGW_PATH} -xf windows-x86_64-mingw.tar.gz
    COMMAND ${CMAKE_COMMAND} -E make_directory ${NATIVE_CODIRA_BUILD_PATH}/third_party/mingw/
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${MINGW_PATH}/lib/ ${NATIVE_CODIRA_BUILD_PATH}/third_party/mingw/lib/
    ${COPY_DLL_COMMAND}
    WORKING_DIRECTORY ${BINARY_DEPS_MINGW_DIR}
    COMMENT "Uncompressing MinGW...")

install(DIRECTORY ${MINGW_PATH}/lib/ DESTINATION third_party/mingw/lib/)
# LLVM binaries and dependent dlls are only needed when we are building a codec for Windows platform.
if(CODIRA_BUILD_CODEC)
    install(DIRECTORY ${MINGW_PATH}/dll/ DESTINATION third_party/llvm/bin/)
endif()

if(CODIRA_BUILD_CODEDB)
    install(DIRECTORY ${MINGW_PATH}/dll/ DESTINATION tools/bin/)
endif()
