# Copyright (c) 2024-2025 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

function(regenerate_and_check_stdlib)
    # NOTE(ivan-tyulyandin): add array.sh and related generated files
    # Preparations to check generated against used code equality
    set(ESCOMPAT "stdlib/escompat")
    set(ESCOMPAT_GEN_FILES "TypedUArrays.ets" "TypedArrays.ets" )
    set(STD_CORE "stdlib/std/core")
    set(STD_CORE_GEN_FILES "DataView.ets" "Function.ets" "Tuple.ets" "BuiltinArray.ets" "BuiltinArraySort.ets" "BuiltinArrayAlgorithms.ets" "Array.ets")
    set(STD_INTEROP "stdlib/std/interop/js")
    set(STD_INTEROP_GEN_FILES "InteropTransferHelper.ets")
    set(GEN_FILES )
    foreach(file ${ESCOMPAT_GEN_FILES})
        list(APPEND GEN_FILES "${ESCOMPAT}/${file}")
    endforeach()
    foreach(file ${STD_CORE_GEN_FILES})
        list(APPEND GEN_FILES "${STD_CORE}/${file}")
    endforeach()
    foreach(file ${STD_INTEROP_GEN_FILES})
        list(APPEND GEN_FILES "${STD_INTEROP}/${file}")
    endforeach()

    set(GEN_SCRIPTS "genlib.sh" )

    message(STATUS "Generating part of StdLib")
    set(GENPATH "${CMAKE_CURRENT_BINARY_DIR}/stdlib")
    foreach(script ${GEN_SCRIPTS})
        set(SCRIPT_FULL_PATH "${ETS_PLUGIN_SOURCE_DIR}/templates/stdlib/${script}")
        message(STATUS ${SCRIPT_FULL_PATH})
        execute_process(COMMAND ${SCRIPT_FULL_PATH} ${GENPATH} OUTPUT_QUIET RESULT_VARIABLE ret)
        if(ret EQUAL "1")
            message( FATAL_ERROR "Bad exit status of script \"${SCRIPT_FULL_PATH}\"")
        endif()
    endforeach()

    # Check generated against used code equality
    foreach(file ${GEN_FILES})
        execute_process( COMMAND ${CMAKE_COMMAND} -E compare_files
            "${ETS_PLUGIN_SOURCE_DIR}/${file}" "${CMAKE_CURRENT_BINARY_DIR}/${file}"
            RESULT_VARIABLE compare_result
        )
        if( NOT compare_result EQUAL 0)
            message(FATAL_ERROR "Generate ${file} is not equal to currently used ${file}, fix and rerun generating script. "
            "Checkout the guide: "
            "https://gitee.com/OpenHarmony/arkcompiler_runtime_core/blob/master/static_core/plugins/ets/stdlib/README.md")
        endif()
    endforeach()
endfunction()

regenerate_and_check_stdlib()
