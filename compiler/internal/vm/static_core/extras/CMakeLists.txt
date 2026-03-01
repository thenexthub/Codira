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

macro(LISTEXTRAS result extras_dir)
    file(GLOB children RELATIVE ${extras_dir} ${extras_dir}/*)
    set(extras_list "")
    foreach(child ${children})
        if (IS_DIRECTORY ${extras_dir}/${child})
            set(extras_list ${extras_list} ${child})
        endif()
    endforeach()
    set(${result} ${extras_list})
endmacro()

LISTEXTRAS(EXTRAS ${PANDA_ROOT}/extras)

foreach(extra ${EXTRAS})
    if(EXISTS ${PANDA_ROOT}/extras/${extra}/CMakeLists.txt)
        add_subdirectory(${extra})
    endif()
endforeach()
