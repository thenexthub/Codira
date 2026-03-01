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

# Use `xcrun` to get MacOS SDK path and version. They are used for compiling Codira standard libraries.

execute_process(
    COMMAND xcrun --sdk macosx --show-sdk-path
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    RESULT_VARIABLE CODIRA_MACOSX_SDK_PATH_AVAILABLE
    OUTPUT_VARIABLE CODIRA_MACOSX_SDK_PATH
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE)
if(${CODIRA_MACOSX_SDK_PATH_AVAILABLE} EQUAL 0)
    message(STATUS "CODIRA_MACOSX_SDK_PATH: ${CODIRA_MACOSX_SDK_PATH}")
else()
    message(STATUS "CODIRA_MACOSX_SDK_PATH: Not Available")
endif()

execute_process(
    COMMAND xcrun --sdk macosx --show-sdk-version
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    RESULT_VARIABLE CODIRA_MACOSX_SDK_VERSION_AVAILABLE
    OUTPUT_VARIABLE CODIRA_MACOSX_SDK_VERSION
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE)
if(${CODIRA_MACOSX_SDK_PATH_AVAILABLE} EQUAL 0)
    message(STATUS "CODIRA_MACOSX_SDK_VERSION: ${CODIRA_MACOSX_SDK_VERSION}")
else()
    message(STATUS "CODIRA_MACOSX_SDK_VERSION: Not Available")
endif()
