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

set(_add_custom_target_recursion_guard FALSE)

if(USE_GCOV_TOOLS)
    set(coverage_util lcov)
endif()

if(USE_LLVM_COV_TOOLS)
    set(coverage_util llvm-cov)
endif()

function(original_add_custom_target)
    _add_custom_target(${ARGV})
endfunction()

function(add_custom_target)
    if(_add_custom_target_recursion_guard)
        message(WARNING "Recursion detected in add_custom_target! Calling original function directly.")
        original_add_custom_target(${ARGV})
        return()
    endif()

    set(_add_custom_target_recursion_guard TRUE PARENT_SCOPE)

    set(processed_commands "${ARGN}")
    list(GET processed_commands 0 target_name)
    list(LENGTH processed_commands cmd_count)

    if(cmd_count GREATER 1)
        list(SUBLIST processed_commands 1 -1 other_args)
        execute_process(
            COMMAND python3 ${PANDA_ROOT}/cmake/interceptor/parse_args.py "${coverage_util}" "${other_args}"
            OUTPUT_VARIABLE python_output
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        original_add_custom_target(
            ${target_name}
            ${python_output}
        )
    else()
        original_add_custom_target(${target_name})
    endif()

    set(_add_custom_target_recursion_guard FALSE PARENT_SCOPE)
endfunction()
