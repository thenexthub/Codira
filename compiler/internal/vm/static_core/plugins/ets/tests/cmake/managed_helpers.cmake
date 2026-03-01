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

# Compile a set of ETS sources into a single .abc file using arktsconfig.json.
#
# Example usage:
#   compile_arktsconfig_unit(my_target ABC_OUT ${CMAKE_CURRENT_BINARY_DIR}
#                            ${CMAKE_CURRENT_SOURCE_DIR}/arktsconfig.json
#                            "${SRC_ETS_FILES}"
#                            OPT_LEVEL release)

function(compile_arktsconfig_unit TARGET ABC_FILES WORK_DIR ARKTS_CONFIG ETS_SOURCES)
    set(oneValueArgs OPT_LEVEL)
    set(multiValueArgs "")
    cmake_parse_arguments(ARG "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    compile_ets_sources(${WORK_DIR} ${TARGET} RESULT "${ETS_SOURCES}"
                        ARKTS_CONFIG ${ARKTS_CONFIG}
                        OPT_LEVEL ${ARG_OPT_LEVEL}
    )
    set(${ABC_FILES} ${RESULT} PARENT_SCOPE)
endfunction()
