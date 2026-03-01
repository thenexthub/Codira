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

if(CMAKE_CROSSCOMPILING AND WIN32)
    # When cross-compiling codec.exe to Windows, the symlink created on Linux is unusable on Windows, so just make a copy.
    execute_process(COMMAND ${CMAKE_COMMAND} -E copy ${LINK_TARGET} ${LINK_NAME} WORKING_DIRECTORY ${WORKING_DIR})
else()
    execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${WORKING_DIR})
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E create_symlink ${LINK_TARGET} ${LINK_NAME}
        WORKING_DIRECTORY ${WORKING_DIR}
        ERROR_VARIABLE err_var)
    # In case of windows, symbolic link can only be created in cmd with administrator privilege or developer mode system.
    # We try to create symbolic link and create a copy of `codec` if symbolic link couldn't be created.
    if(WIN32 AND err_var)
        message(WARNING "Symbolic link \"${LINK_NAME}\" could not be made. A copy of \"${LINK_TARGET}\" is created.")
        execute_process(COMMAND ${CMAKE_COMMAND} -E copy ${LINK_TARGET} ${LINK_NAME} WORKING_DIRECTORY ${WORKING_DIR})
    endif()
endif()
