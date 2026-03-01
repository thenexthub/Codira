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
# ExtractRtlib.cmake (arg 2)
# ${CMAKE_BINARY_DIR} (arg 3)
# ${SPECIFIC_LIB} (arg 4)
# ${NEW_SPECIFIC_LIB_PATH} (arg 5)
# ${CODIRA_TARGET_ARCH} (arg 6)
# ${CROSS_COMPILING} (arg 7)
# ${THIRD_PARTY_LLVM} (arg 8)
# ${IOS} (arg 9)
set(CMAKE_BINARY_DIR ${CMAKE_ARGV3})
set(SPECIFIC_LIB ${CMAKE_ARGV4})
set(NEW_SPECIFIC_LIB_PATH ${CMAKE_ARGV5})
set(CODIRA_TARGET_ARCH ${CMAKE_ARGV6})
set(CODIRA_BUILD_CODEC ${CMAKE_ARGV7})
set(THIRD_PARTY_LLVM ${CMAKE_ARGV8})
set(IOS ${CMAKE_ARGV9})

set(LIB_SUFFIX ".a")

# Create the specific library
# 1. Find the path of the specific library compiled from llvm compiler-rt
if(CODIRA_BUILD_CODEC) # It is native-compiling
    # asan/tsan has multiple product, select precisely
    if(("asan" STREQUAL "${SPECIFIC_LIB}") OR ("tsan" STREQUAL "${SPECIFIC_LIB}"))
        file(GLOB OLD_SPECIFIC_LIB_PATHS
            ${CMAKE_BINARY_DIR}/${THIRD_PARTY_LLVM}/lib/clang/15.0.4/lib/*/libclang_rt.${SPECIFIC_LIB}-${CODIRA_TARGET_ARCH}${LIB_SUFFIX})
        if(NOT "${OLD_SPECIFIC_LIB_PATHS}")
            file(GLOB OLD_SPECIFIC_LIB_PATHS
                 ${CMAKE_BINARY_DIR}/${THIRD_PARTY_LLVM}/lib/clang/15.0.4/lib/*/libclang_rt.${SPECIFIC_LIB}${LIB_SUFFIX})
        endif()
    else()
        file(GLOB OLD_SPECIFIC_LIB_PATHS
             ${CMAKE_BINARY_DIR}/${THIRD_PARTY_LLVM}/lib/clang/15.0.4/lib/*/libclang_rt.${SPECIFIC_LIB}*${LIB_SUFFIX})
    endif()
    foreach(f ${OLD_SPECIFIC_LIB_PATHS})
        if(f MATCHES ".*${CODIRA_TARGET_ARCH}.*\\.a" OR f MATCHES ".*darwin.*\\.a")
            set(OLD_SPECIFIC_LIB_PATH ${f})
        endif()
    endforeach()
else() # It is cross-compiling
    if(IOS)
        file(GLOB OLD_SPECIFIC_LIB_PATH
            ${CMAKE_BINARY_DIR}/${THIRD_PARTY_LLVM}/lib/*/libclang_rt.${SPECIFIC_LIB}${LIB_SUFFIX})
    else()
        file(GLOB OLD_SPECIFIC_LIB_PATH
            ${CMAKE_BINARY_DIR}/${THIRD_PARTY_LLVM}/lib/*/libclang_rt.${SPECIFIC_LIB}-${CODIRA_TARGET_ARCH}${LIB_SUFFIX})
    endif()
endif()

# 2. Move the specific lib to a temporary location; currently we use the ${CMAKE_BINARY_DIR}/lib
if(NOT "${OLD_SPECIFIC_LIB_PATH}" STREQUAL "" AND EXISTS ${OLD_SPECIFIC_LIB_PATH})
    # The libraries to be extracted have the following relationship with the supported target arch:
    # | lib \ Arch     | x86_64_ | aarch64 |
    # | -------------- | ------- | ------- |
    # | clang-builtins | need    | no need |
    # | clang-profile  | need    | need    |
    # | clang-asan     | need    | need    |
    # | clang-tsan     | need    | need    |
    if(NOT (CODIRA_TARGET_ARCH STREQUAL "aarch64" AND ${SPECIFIC_LIB} STREQUAL "builtins"))
        file(RENAME ${OLD_SPECIFIC_LIB_PATH} ${NEW_SPECIFIC_LIB_PATH})
    endif()
else()
    message(STATUS "The libclang_rt.${SPECIFIC_LIB}-${CODIRA_TARGET_ARCH}.a has been extracted and does not exist.")
endif()

# 3. Avoid installing redundant empty folder when cross-compiling.
# All its contents have been already moved to Codira's lib folder, so it is empty and should be removed.
if(NOT CODIRA_BUILD_CODEC)
    file(GLOB REMAINING_FILES ${CMAKE_BINARY_DIR}/${THIRD_PARTY_LLVM}/lib/linux/*)
    list(LENGTH REMAINING_FILES LEN)
    if(${LEN} STREQUAL "0")
        file(REMOVE_RECURSE ${CMAKE_BINARY_DIR}/${THIRD_PARTY_LLVM}/lib/linux)
    endif()
endif()

# 4. Copy the specific libraries to the runtime directory for build-binary-tar (see TARGET codenative POST_BUILD)
