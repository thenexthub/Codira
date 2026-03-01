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

function(extract_archive)
    set(oneValueArgs FROM TO)
    cmake_parse_arguments(EXTRACT_ARCHIVE "" "${oneValueArgs}" "" ${ARGV})

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

    file(MAKE_DIRECTORY ${EXTRACT_ARCHIVE_TO})
    execute_process(
        COMMAND ${TARGET_AR} x ${EXTRACT_ARCHIVE_FROM}
        WORKING_DIRECTORY ${EXTRACT_ARCHIVE_TO})
endfunction(extract_archive)
