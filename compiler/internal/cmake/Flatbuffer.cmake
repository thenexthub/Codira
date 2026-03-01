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

set(FLATBUFFERS_SRC ${CMAKE_CURRENT_SOURCE_DIR}/flatbuffers)
set(FLATBUFFERS_COMPILE_OPTIONS -DCMAKE_C_FLAGS=${GCC_TOOLCHAIN_FLAG} -DCMAKE_CXX_FLAGS=${GCC_TOOLCHAIN_FLAG})
if(CMAKE_HOST_WIN32)
    list(APPEND FLATBUFFERS_COMPILE_OPTIONS -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++)
else()
    list(APPEND FLATBUFFERS_COMPILE_OPTIONS -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++)
endif()

# for CloudDragon, download in Prebuild
if(EXISTS ${FLATBUFFERS_SRC}/CMakeLists.txt)
    set(FLATBUFFERS_DOWNLOAD_ARGS
        SOURCE_DIR ${FLATBUFFERS_SRC})
else()
    set(REPOSITORY_PATH https://gitcode.com/openharmony/third_party_flatbuffers.git)
    message(STATUS "Set flatbuffers REPOSITORY_PATH: ${REPOSITORY_PATH}")
    set(FLATBUFFERS_DOWNLOAD_ARGS
        GIT_REPOSITORY ${REPOSITORY_PATH}
        GIT_TAG 8355307828c7a6bc6bae9d2dba48ad3ab0b5ff2d
        GIT_PROGRESS ON
        GIT_CONFIG ${GIT_ARGS}
        GIT_SHALLOW OFF)
endif()

ExternalProject_Add(
    flatbuffers
    ${FLATBUFFERS_DOWNLOAD_ARGS}
    CMAKE_ARGS
        # no need to Build tests and install.
        -DFLATBUFFERS_BUILD_TESTS=OFF
        -DFLATBUFFERS_INSTALL=ON
        # Build only necessary targets.
        -DFLATBUFFERS_BUILD_FLATHASH=OFF
        -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}
        ${FLATBUFFERS_COMPILE_OPTIONS})
externalproject_get_property(flatbuffers SOURCE_DIR)
set(FLATBUFFERS_SRC ${SOURCE_DIR})
