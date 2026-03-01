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

macro(LISTSCRIPTS result scripts_dir)
    file(GLOB children RELATIVE ${scripts_dir} ${scripts_dir}/*)
    set(scripts_list "")
    foreach(child ${children})
        if (IS_DIRECTORY ${scripts_dir}/${child})
            set(scripts_list ${scripts_list} ${child})
        endif()
    endforeach()
    set(${result} ${scripts_list})
endmacro()

LISTSCRIPTS(SCRIPTS ${PANDA_ROOT}/scripts)

foreach(extra ${SCRIPTS})
    if(EXISTS ${PANDA_ROOT}/scripts/${extra}/CMakeLists.txt)
        add_subdirectory(${extra})
    endif()
endforeach()
