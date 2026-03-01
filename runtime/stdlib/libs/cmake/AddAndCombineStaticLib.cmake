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

function(add_and_combine_static_lib)
    set(oneValueArgs TARGET OUTPUT_NAME)
    set(multiValueArgs LIBRARIES DEPENDS)
    cmake_parse_arguments(
        COMBINE
        ""
        "${oneValueArgs}"
        "${multiValueArgs}"
        ${ARGV})

    set(TARGET_AR ar)
    if(CMAKE_CROSSCOMPILING)
        if(IOS)
            set(TARGET_AR ${CODIRA_TARGET_TOOLCHAIN}/ar)
        elseif(${CMAKE_C_COMPILER_ID} STREQUAL "Clang")
            set(TARGET_AR ${CODIRA_TARGET_TOOLCHAIN}/llvm-ar)
        else()
            set(TARGET_AR ${CODIRA_TARGET_TOOLCHAIN}/${TRIPLE}-ar)
        endif()
    endif()

    add_custom_command(
        OUTPUT ${COMBINE_OUTPUT_NAME}
        COMMAND
            ${CMAKE_COMMAND} -P ${PROJECT_SOURCE_DIR}/cmake/modules/MergeArchives.cmake ARCHIVER ${TARGET_AR}
            OUTPUT_FILENAME ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/${COMBINE_OUTPUT_NAME} INPUTS "\"${COMBINE_LIBRARIES}\""
        DEPENDS ${COMBINE_DEPENDS})
    add_custom_target(
        ${COMBINE_TARGET} ALL
        DEPENDS ${COMBINE_OUTPUT_NAME}
        COMMENT "Generating ${COMBINE_OUTPUT_NAME}")
    # Caller may get output file path through COMBINED_STATIC_LIB_LOC property.
    set_target_properties(${COMBINE_TARGET} PROPERTIES
        COMBINED_STATIC_LIB_LOC "${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/${COMBINE_OUTPUT_NAME}")
endfunction()
