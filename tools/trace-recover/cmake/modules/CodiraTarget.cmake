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

cmake_minimum_required(VERSION 3.10)

function(_find_or_set_program_validator candidate result_var)
    execute_process(
        COMMAND "${candidate}" --version
        RESULT_VARIABLE _vres
        OUTPUT_QUIET
        ERROR_QUIET
    )
    if(_vres EQUAL 0)
        set(${result_var} TRUE PARENT_SCOPE)
    else()
        set(${result_var} FALSE PARENT_SCOPE)
    endif()
endfunction()

# search for codec, check for env and cmake definitions
find_program(codec_path codec
    PATHS "${CODIRA_HOME}/bin" "$ENV{CODIRA_HOME}/bin"
    VALIDATOR _find_or_set_program_validator REQUIRED NO_DEFAULT_PATH)

get_filename_component(cangjie_bin_path ${codec_path} DIRECTORY)
get_filename_component(target_cangjie_home ${cangjie_bin_path} DIRECTORY)
message(STATUS "found cangjie toolchain path: ${target_cangjie_home}")

# generate cangjie triple
if(CMAKE_CROSSCOMPILING)
    set(CODIRA_LIB_TRIPLE "${CMAKE_SYSTEM_NAME}_${CMAKE_SYSTEM_PROCESSOR}_codenative")
else()
    set(CODIRA_LIB_TRIPLE "${CMAKE_SYSTEM_NAME}_${CMAKE_HOST_SYSTEM_PROCESSOR}_codenative")
endif()
string(TOLOWER "${CODIRA_LIB_TRIPLE}" CODIRA_LIB_TRIPLE)

set(CODIRA_TOOLCHAIN_PATH ${target_cangjie_home} CACHE FILEPATH "cangjie toolchain path")
set(CODIRA_COMPILER_PATH ${codec_path} CACHE FILEPATH "cangjie compiler path")
set(CODIRA_DYNAMIC_LIBRARY_PATH ${target_cangjie_home}/runtime/lib/${CODIRA_LIB_TRIPLE} CACHE FILEPATH "cangjie dynamic library path")
set(CODIRA_STATIC_LIBRARY_PATH ${target_cangjie_home}/lib/${CODIRA_LIB_TRIPLE} CACHE FILEPATH "cangjie static library path")

function(add_cangjie_executable target)
    set(multi_value_args SOURCES COMPILE_OPTIONS DEPENDS)
    cmake_parse_arguments(
        CODIRA_TARGET
        "${options}"
        "${one_value_args}"
        "${multi_value_args}"
        ${ARGN})

    set(full_path_sources)
    foreach(arg ${CODIRA_TARGET_SOURCES})
        find_file(source_code ${arg} PATHS ${CMAKE_CURRENT_SOURCE_DIR} REQUIRED NO_DEFAULT_PATH)
        list(APPEND full_path_sources ${source_code})
    endforeach()

    set(cangjie_compile_flags)
    # build types
    # RelWithDebInfo and MinSizeRelWithDebInfo and Debug
    if(CMAKE_BUILD_TYPE MATCHES "WithDebInfo$" OR (CMAKE_BUILD_TYPE MATCHES "^Debug$"))
        list(APPEND cangjie_compile_flags "-g")
    endif()
    # MinSizeRel and MinSizeRelWithDebInfo
    if(CMAKE_BUILD_TYPE MATCHES "^MinSize")
        list(APPEND cangjie_compile_flags "-Os")
    endif()
    # RelWithDebInfo and Release
    if(CMAKE_BUILD_TYPE MATCHES "^Rel")
        list(APPEND cangjie_compile_flags "-O2")
    endif()

    # cross compile configurations
    if(CMAKE_CROSSCOMPILING)
        if(NOT CODIRA_TRIPLE)
            message(FATAL_ERROR "Please define CODIRA_TRIPLE in your toolchain file")
        endif()
        list(APPEND cangjie_compile_flags "--target=${CODIRA_TRIPLE}")
        list(APPEND cangjie_compile_flags "--sysroot=${CMAKE_SYSROOT}")
    endif()
    
    # depends
    foreach(arg ${CODIRA_TARGET_DEPENDS})
        if (NOT TARGET ${arg})
            message(FATAL_ERROR "${arg} is not a target")
        endif()
        list(APPEND cangjie_compile_flags -L$<TARGET_FILE_DIR:${arg}>)
        list(APPEND cangjie_compile_flags -l${arg})
    endforeach()

    # move compile options to the last, avoiding depends library's deps cannot be found
    list(APPEND cangjie_compile_flags ${CODIRA_TARGET_COMPILE_OPTIONS})

    set(output ${target}$<$<BOOL:${WIN32}>:.exe>)
    add_custom_target(code_${target} ALL
        COMMAND ${CODIRA_COMPILER_PATH} -o ${output} ${cangjie_compile_flags} ${full_path_sources}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/${output}
        SOURCES ${full_path_sources}
    )
    install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/${output} DESTINATION bin)
    
endfunction()
