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

# cmake -P
# ApplyLLVMPatch.cmake (arg 2)
# ${LLVM_SOURCE_DIR} (arg 3)
# ${LLVM_PATCH} (arg 4)
set(LLVM_SOURCE_DIR ${CMAKE_ARGV3})
set(LLVM_PATCH ${CMAKE_ARGV4})

execute_process(COMMAND git diff --quiet
    WORKING_DIRECTORY ${LLVM_SOURCE_DIR}
    RESULT_VARIABLE CODENATIVE_SOURCE_DIR_IS_MODIFIED)

if(CODENATIVE_SOURCE_DIR_IS_MODIFIED EQUAL 0)
    execute_process(
        COMMAND git reset --hard 5c68a1cb123161b54b72ce90e7975d95a8eaf2a4
        WORKING_DIRECTORY ${LLVM_SOURCE_DIR}
    )
    execute_process(
        COMMAND git apply --reject --whitespace=fix ${LLVM_PATCH}
        WORKING_DIRECTORY ${LLVM_SOURCE_DIR}
    )
endif()