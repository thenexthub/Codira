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

# GNU ar and ranlib operates in non-deterministic mode by default on some systems.
# To keep build consistency, we set -D option to force deterministic mode if it's
# possible. Note that some tools do not support -D option, for example BSD `ar` and
# `llvm-ranlib`. We test whether the tool supports -D option and only set the option
# if it supports.

execute_process(
    COMMAND ${CMAKE_AR} -crD dummy_archive.a
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    RESULT_VARIABLE AR_SUPPORT_DETERMINISTIC_MODE
    OUTPUT_QUIET ERROR_QUIET)
if(${AR_SUPPORT_DETERMINISTIC_MODE} EQUAL 0)
    set(CMAKE_C_ARCHIVE_CREATE "<CMAKE_AR> -crD <TARGET> <LINK_FLAGS> <OBJECTS>")
    set(CMAKE_C_ARCHIVE_APPEND "<CMAKE_AR> -rD <TARGET> <LINK_FLAGS> <OBJECTS>")
    set(CMAKE_CXX_ARCHIVE_CREATE "<CMAKE_AR> -crD <TARGET> <LINK_FLAGS> <OBJECTS>")
    set(CMAKE_CXX_ARCHIVE_APPEND "<CMAKE_AR> -rD <TARGET> <LINK_FLAGS> <OBJECTS>")
    message(STATUS "<CMAKE_AR> deterministic mode (-D) is set.")
endif()

if(DARWIN)
    file(TOUCH ${CMAKE_BINARY_DIR}/dummy_object.o)
    execute_process(
        COMMAND ${CMAKE_AR} -cr dummy_archive.a dummy_object.o
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        OUTPUT_QUIET ERROR_QUIET)
else()
    execute_process(
        COMMAND ${CMAKE_AR} -cr dummy_archive.a
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        OUTPUT_QUIET ERROR_QUIET)
endif()
execute_process(
    COMMAND ${CMAKE_RANLIB} -D dummy_archive.a
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    RESULT_VARIABLE RANLIB_SUPPORT_DETERMINISTIC_MODE
    OUTPUT_QUIET ERROR_QUIET)
if(${RANLIB_SUPPORT_DETERMINISTIC_MODE} EQUAL 0)
    set(CMAKE_C_ARCHIVE_FINISH "<CMAKE_RANLIB> -D <TARGET>")
    set(CMAKE_CXX_ARCHIVE_FINISH "<CMAKE_RANLIB> -D <TARGET>")
    message(STATUS "<CMAKE_RANLIB> deterministic mode (-D) is set.")
endif()
