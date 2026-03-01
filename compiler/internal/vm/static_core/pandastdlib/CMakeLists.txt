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

cmake_minimum_required(VERSION 3.10)

project(arkstdlib)

add_panda_assembly(TARGET arkstdlib SOURCE pandastdlib.pa)

if(CMAKE_CROSSCOMPILING)
    ExternalProject_Get_Property(panda_host_tools binary_dir)
    if(PANDA_WITH_QUICKENER)
        set(arkquick_target panda_host_tools-build)
        set(arkquick_bin    "${binary_dir}/quickener/arkquick")
    endif()
else()
    set(arkquick_target arkquick)
    set(arkquick_bin    $<TARGET_FILE:${arkquick_target}>)
endif()

# TODO: remove after all components will use arkstdlib.abc instead of pandastdlib.bin
add_custom_target(pandastdlib)
add_dependencies(pandastdlib arkstdlib)

if(PANDA_WITH_QUICKENER)
    add_dependencies(pandastdlib arkquick)

    add_custom_command(TARGET arkstdlib
                       POST_BUILD
                       COMMAND ${CMAKE_COMMAND} -E create_symlink "arkstdlib.abc" "pandastdlib.bin"
                       COMMAND ${arkquick_bin} "${CMAKE_CURRENT_BINARY_DIR}/arkstdlib.abc" "${CMAKE_CURRENT_BINARY_DIR}/arkstdlib.abcq"
                       COMMAND ${CMAKE_COMMAND} -E create_symlink "arkstdlib.abcq" "pandastdlib.binq"
                       WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
else()
    add_custom_command(TARGET arkstdlib
                       POST_BUILD
                       COMMAND ${CMAKE_COMMAND} -E create_symlink "arkstdlib.abc" "pandastdlib.bin"
                       WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
endif()
