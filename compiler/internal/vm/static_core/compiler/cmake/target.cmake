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

set(PANDA_COMPILER_MULTI_TARGET OFF)

# Define function just to restrict the scope of variables
function(panda_check_compiler_multi_target)
    set(compiler_any_target OFF)
    foreach(target X86 X86_64 AARCH32 AARCH64)
        set(target_name "PANDA_COMPILER_TARGET_${target}")
        if(compiler_any_target AND ${target_name})
            set(PANDA_COMPILER_MULTI_TARGET ON PARENT_SCOPE)
        endif()
        if(${target_name})
            set(compiler_any_target ON)
        endif()
    endforeach()
endfunction()
panda_check_compiler_multi_target()

panda_promote_to_definitions(
    PANDA_COMPILER_MULTI_TARGET
)

if (PANDA_COMPILER_TARGET_AARCH32)
    if (PANDA_TARGET_ARM64)
        message(FATAL_ERROR "Unimplemented multi-build aarch32 on aarch64-target")
    endif()
    if (PANDA_TARGET_X86)
        message(FATAL_ERROR "Unimplemented multi-build aarch32 on x86-build-target")
    endif()
    # ABI must be defined for build on non-arm archs
    if (NOT(PANDA_TARGET_ARM32_ABI_SOFT OR PANDA_TARGET_ARM32_ABI_SOFTFP OR PANDA_TARGET_ARM32_ABI_HARD))
        panda_set_flag(PANDA_TARGET_ARM32_ABI_SOFTFP)
    endif()
endif()

if (PANDA_COMPILER_TARGET_AARCH64)
    if (PANDA_TARGET_ARM32)
        message(FATAL_ERROR "Unimplemented multi-build aarch64 on aarch32-target")
    endif()
    if (PANDA_TARGET_X86)
        message(FATAL_ERROR "Unimplemented multi-build aarch64 on x86-build-target")
    endif()
endif()

message(STATUS "Compiler backends status:")
message(STATUS "PANDA_COMPILER_MULTI_TARGET = ${PANDA_COMPILER_MULTI_TARGET}")
message(STATUS "PANDA_COMPILER_TARGET: X86=${PANDA_COMPILER_TARGET_X86} X86_64=${PANDA_COMPILER_TARGET_X86_64} AARCH32=${PANDA_COMPILER_TARGET_AARCH32} AARCH64=${PANDA_COMPILER_TARGET_AARCH64}")
