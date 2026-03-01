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

include(ExternalProject)

find_program(SSH_EXECUTABLE ssh)
# Don't check known_host to get rid of interactive operations in cmake
if(CMAKE_HOST_WIN32)
    set(GIT_ARGS "ssh.variant=ssh")
else()
    set(GIT_ARGS "core.sshCommand=${SSH_EXECUTABLE}\\ -o\\ StrictHostKeyChecking=no" "ssh.variant=ssh")
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

# Handle third-party dependencies
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")

include(Flatbuffer)

if(NOT EXISTS ${CODIRA_PCRE2_SOURCE_DIR})
    set(REPOSITORY_PATH https://gitcode.com/openharmony/third_party_pcre2.git)
    message(STATUS "Set pcre2 REPOSITORY_PATH: ${REPOSITORY_PATH}")
    execute_process(
        COMMAND git clone --branch master ${REPOSITORY_PATH} ${CODIRA_PCRE2_SOURCE_DIR}
    )
endif()
include(Pcre2)
