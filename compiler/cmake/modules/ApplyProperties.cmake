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

function(apply_properties)
    set(oneValueArgs FROM_TARGET TO_TARGET)
    set(multiValueArgs PROPERTY_NAMES)

    cmake_parse_arguments(
        ARG
        ""
        "${oneValueArgs}"
        "${multiValueArgs}"
        ${ARGN})

    foreach(arg ${ARG_PROPERTY_NAMES})
        get_target_property(PROPERTY_VALUE ${ARG_FROM_TARGET} ${arg})
        if(NOT ("${PROPERTY_VALUE}" MATCHES "PROPERTY_VALUE-NOTFOUND"))
            set_target_properties(${ARG_TO_TARGET} PROPERTIES ${arg} "${PROPERTY_VALUE}")
        endif()
    endforeach(arg)
endfunction()
