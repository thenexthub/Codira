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

cmake_minimum_required(VERSION 3.14.1 FATAL_ERROR)
project(plugin_ets_sdk)

message(STATUS "Building ArkTS SDK")

add_custom_target(ets_sdk_prereqs)

file(GLOB_RECURSE SDK_SOURCES CONFIGURE_DEPENDS "**/*.ets")
panda_target_sources(ets_sdk_prereqs PRIVATE ${SDK_SOURCES})

add_dependencies(ets_sdk_prereqs etsstdlib)
# Note(ivan-tyulyandin): use of ark_link is a workaround for internal issue #21462
add_dependencies(ets_sdk_prereqs ark_link)

# Note(ivan-tyulyandin): provide script to collect all ets files in build time on each rerun, issue #21800
set(SDK_SRCS
    "${CMAKE_CURRENT_SOURCE_DIR}/api/@ohos.buffer.ets"
    "${CMAKE_CURRENT_SOURCE_DIR}/api/@ohos.uri.ets"
    "${CMAKE_CURRENT_SOURCE_DIR}/api/@ohos.url.ets"
    "${CMAKE_CURRENT_SOURCE_DIR}/api/@ohos.util.ArrayList.ets"
    "${CMAKE_CURRENT_SOURCE_DIR}/api/@ohos.util.Deque.ets"
    "${CMAKE_CURRENT_SOURCE_DIR}/api/@ohos.util.HashMap.ets"
    "${CMAKE_CURRENT_SOURCE_DIR}/api/@ohos.util.HashSet.ets"
    "${CMAKE_CURRENT_SOURCE_DIR}/api/@ohos.util.LightWeightMap.ets"
    "${CMAKE_CURRENT_SOURCE_DIR}/api/@ohos.util.LightWeightSet.ets"
    "${CMAKE_CURRENT_SOURCE_DIR}/api/@ohos.util.LinkedList.ets"
    "${CMAKE_CURRENT_SOURCE_DIR}/api/@ohos.util.List.ets"
    "${CMAKE_CURRENT_SOURCE_DIR}/api/@ohos.util.PlainArray.ets"
    "${CMAKE_CURRENT_SOURCE_DIR}/api/@ohos.util.Queue.ets"
    "${CMAKE_CURRENT_SOURCE_DIR}/api/@ohos.util.Stack.ets"
    "${CMAKE_CURRENT_SOURCE_DIR}/api/@ohos.util.stream.ets"
    "${CMAKE_CURRENT_SOURCE_DIR}/api/@ohos.util.ets"
    "${CMAKE_CURRENT_SOURCE_DIR}/api/@ohos.util.TreeMap.ets"
    "${CMAKE_CURRENT_SOURCE_DIR}/api/@ohos.util.TreeSet.ets"
    "${CMAKE_CURRENT_SOURCE_DIR}/api/@ohos.xml.ets"
    "${CMAKE_CURRENT_SOURCE_DIR}/api/@ohos.base.ets"
    "${CMAKE_CURRENT_SOURCE_DIR}/arkts/@arkts.math.Decimal.ets"
    "${CMAKE_CURRENT_SOURCE_DIR}/arkts/@arkts.collections.ets"
)

set(ABCS_TO_LINK "")

foreach(FILE ${SDK_SRCS})
    get_filename_component(FILE_NAME ${FILE} NAME)
    # Note(ivan-tyulyandin): @ is not a valid symbol for cmake target
    string(REPLACE "@" "" FILE_NAME ${FILE_NAME})
    set(ABC_TARGET "sdk_${FILE_NAME}")
    set(ABC_LOC "${CMAKE_CURRENT_BINARY_DIR}/${ABC_TARGET}.abc")

    compile_ets_code(${FILE} ${ABC_LOC} ${ABC_TARGET})
    panda_target_sources(${ABC_TARGET} PRIVATE ${FILE})

    add_dependencies(ets_sdk_prereqs ${ABC_TARGET})
    set(ABCS_TO_LINK "${ABCS_TO_LINK}" "${ABC_LOC}")
endforeach()

set(SDK_ABC "${CMAKE_CURRENT_BINARY_DIR}/etssdk.abc")

add_subdirectory(native)

add_custom_target(
    link_ets_sdk
    COMMAND $<TARGET_FILE:ark_link> --output ${SDK_ABC} -- ${ABCS_TO_LINK}
)
add_dependencies(link_ets_sdk ets_sdk_prereqs)

add_custom_target(etssdk DEPENDS link_ets_sdk)

if(NOT CMAKE_CROSSCOMPILING)
    set(verifier_target verifier)
    set(verifier_bin    $<TARGET_FILE:${verifier_target}>)

    add_dependencies(link_ets_sdk ${verifier_target})

    set (ETS_STD_LIB "${PANDA_BINARY_ROOT}/plugins/ets/etsstdlib.abc")
    set (ETS_SDK_ABC "${PANDA_BINARY_ROOT}/plugins/ets/sdk/etssdk.abc")
    
    set (RUN_VERIFIER_DST_DIR "${PANDA_BINARY_ROOT}/verification/verifier/")
    set (RUN_VERIFIER_SRC_DIR "${CMAKE_SOURCE_DIR}/verification/verifier/")

    configure_file(
        ${RUN_VERIFIER_SRC_DIR}/run_verifier.sh
        ${RUN_VERIFIER_DST_DIR}/run_verifier.sh
        COPYONLY
    )
    add_custom_command(TARGET link_ets_sdk 
        POST_BUILD
        COMMAND bash -c "${RUN_VERIFIER_DST_DIR}/run_verifier.sh '${verifier_bin}' '${ETS_STD_LIB}' '${ETS_SDK_ABC}'" -P
    )
        
endif()
