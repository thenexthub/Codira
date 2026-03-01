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

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../third_party/boundscheck-v1.1.16/include)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../build/libs/")

add_subdirectory(stdx)
if(CODIRA_CODEGEN_CODENATIVE_BACKEND)
    include(MakeCODENATIVEStdSharedLib)
    if(TRIPLE STREQUAL "arm-linux-ohos")
        include(AddCODENATIVELibraryArm32)
    else()
        include(AddCODENATIVELibrary)
    endif()
endif()

execute_process(
    COMMAND codec --version 
    RESULT_VARIABLE codec_result
    OUTPUT_VARIABLE codec_output
    ERROR_VARIABLE codec_error
)

set(BACKEND)
set(version_number)
if(codec_result EQUAL 0)
    string(REGEX MATCH "Codira Compiler: (([0-9]+\\.[0-9]+\\.[0-9]+)(-[a-z]+\\.[0-9]+)?) \\(codenative\\)" first_match_codenative "${codec_output}")
    if(first_match_codenative)
        set(version_number "${CMAKE_MATCH_1}")
        set(BACKEND "codenative")
    endif()
else()
    message(WARNING "codec --version error:")
    message(WARNING "${codec_error}")
endif() 
 
message("codec version is " ${version_number})
 
 
set(json_file_path "${CMAKE_SOURCE_DIR}/package.json")
 
file(READ "${json_file_path}" file_content)
string(REGEX MATCH "\"version\": \"([^\"]+)\"" match_stdx_version_result "${file_content}")
if(match_stdx_version_result)
    set(stdx_version_number "${CMAKE_MATCH_1}")
else()
    message(STATUS "not find stdx version")
endif()
 
message("stdx version is "  ${stdx_version_number})
 
set(json_content "{
    \"codec_version\": \"${version_number}\",
    \"version\": \"${version_number}.${stdx_version_number}\",
    \"description\": \"\"
}")
set(json_file "${CMAKE_BINARY_DIR}/package.json")
file(WRITE "${json_file}" "${json_content}")
install(FILES "${json_file}" DESTINATION "${TARGET_TRIPLE_DIRECTORY_PREFIX}_${BACKEND}")