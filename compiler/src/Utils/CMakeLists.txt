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

if(EXISTS ${CODIRA_CODENATIVE_SOURCE_DIR})
    set(UNICODE_DEPENDENCIES ${CODIRA_CODENATIVE_SOURCE_DIR})
else()
    execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/utils_dep/
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
    execute_process(COMMAND git clone https://gitcode.com/openharmony/third_party_llvm-project.git --depth 1
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/utils_dep/
        RESULT_VARIABLE GIT_CLONE_RESULT)
    execute_process(COMMAND git fetch --depth 1 origin 5c68a1cb123161b54b72ce90e7975d95a8eaf2a4
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/utils_dep/third_party_llvm-project
        RESULT_VARIABLE GIT_FETCH_RESULT)
    execute_process(COMMAND git checkout 5c68a1cb123161b54b72ce90e7975d95a8eaf2a4
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/utils_dep/third_party_llvm-project
        RESULT_VARIABLE GIT_FETCH_RESULT)
    set(UNICODE_DEPENDENCIES ${CMAKE_BINARY_DIR}/utils_dep/third_party_llvm-project)
endif()

set(UNICODE_UTIL_SRC
    Unicode.cpp
    UnicodeNFC.cpp
    UnicodeWidth.cpp
    ${UNICODE_DEPENDENCIES}/llvm/lib/Support/ConvertUTF.cpp)

set(COMMON_UTIL_SRC
    FileUtil.cpp
    SafePointer.cpp
    SignalUtil.cpp
    FloatFormat.cpp
    Utils.cpp
    ICEUtil.cpp
    Semaphore.cpp
    StdUtils/StdUtils.cpp)

set(PROFILE_SRC
    ProfileRecorder.cpp
    UserBase.cpp
    UserTimer.cpp
    UserMemoryUsage.cpp
    UserCodeInfo.cpp)

if(CODIRA_BUILD_CODEC OR CODIRA_BUILD_TESTS)
    if(WIN32)
        list(APPEND COMMON_UTIL_SRC SignalWin.cpp)
    else()
        list(APPEND COMMON_UTIL_SRC SignalUnix.cpp)
    endif()
endif()

add_library(CodiraCommonUtil OBJECT ${COMMON_UTIL_SRC})
add_library(CodiraCommonUtilFFI OBJECT
            FileUtil.cpp
            SafePointer.cpp
            FloatFormat.cpp
            Utils.cpp
            ICEUtil.cpp
            Semaphore.cpp
            StdUtils/StdUtils.cpp)
add_library(CodiraUnicodeUtils OBJECT ${UNICODE_UTIL_SRC})

target_include_directories(CodiraUnicodeUtils PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(CodiraUnicodeUtils PRIVATE ${UNICODE_DEPENDENCIES}/llvm/include)
if(CODIRA_GENERATE_UNICODE_TABLE)
    add_dependencies(CodiraUnicodeUtils GenerateNFCData)
    add_dependencies(CodiraUnicodeUtils GenerateWidthData)
endif()

if(CODIRA_BUILD_CODEC)
    add_library(CodiraProfileUtils OBJECT ${PROFILE_SRC})
    target_compile_options(CodiraProfileUtils PRIVATE ${CODEC_EXTRA_WARNINGS})
endif()
target_include_directories(CodiraCommonUtil PRIVATE ${BOUNDSCHECK}/include)

target_compile_options(CodiraCommonUtil PRIVATE ${CODEC_WITH_LLVM_EXTRA_WARNINGS})
target_compile_options(CodiraCommonUtilFFI PRIVATE ${CODEC_WITH_LLVM_EXTRA_WARNINGS})
target_compile_options(CodiraUnicodeUtils PRIVATE ${CODEC_WITH_LLVM_EXTRA_WARNINGS})
