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

if(CODIRA_BUILD_CODEC)
    set(SRC_FILES ./*.cpp Toolchains/GCCPathScanner.cpp Toolchains/Gnu.cpp Toolchains/MachO.cpp ./Backend/Backend.cpp)
    if(CODIRA_CODEGEN_CODENATIVE_BACKEND)
        set(SRC_FILES
            ${SRC_FILES}
                Backend/CODENATIVEBackend.cpp
            Toolchains/ToolChain.cpp
            Toolchains/CODENATIVE/Linux_CODENATIVE.cpp
            Toolchains/CODENATIVE/MinGW_CODENATIVE.cpp
            Toolchains/CODENATIVE/Darwin_CODENATIVE.cpp
            Toolchains/CODENATIVE/IOS_CODENATIVE.cpp
            Toolchains/CODENATIVE/Android_CODENATIVE.cpp
            Toolchains/CODENATIVE/Ohos_CODENATIVE.cpp)
    endif()
    file(GLOB DRIVER_SRC ${SRC_FILES})
    include_directories(BEFORE ${CMAKE_CURRENT_SOURCE_DIR})

    add_library(CodiraDriver OBJECT ${DRIVER_SRC})
    # use llvm
    if(CODIRA_CODEGEN_CODENATIVE_BACKEND)
        add_dependencies(CodiraDriver codenative)
    endif()
    target_include_directories(CodiraDriver PRIVATE ${LLVM_INCLUDE_DIRS})
    if(CODIRA_DISABLE_STACK_GROW_FEATURE)
        target_compile_definitions(CodiraDriver PRIVATE CODIRA_DISABLE_STACK_GROW_FEATURE)
    endif()
    target_compile_options(CodiraDriver PRIVATE ${CODEC_WITH_LLVM_EXTRA_WARNINGS})
endif()

# build for lsp
set(LIB_TEMP_FILES_UTIL TempFileManager.cpp)
add_library(CodiraTempFilesUtil OBJECT ${LIB_TEMP_FILES_UTIL})
target_compile_options(CodiraTempFilesUtil PRIVATE ${CODEC_WITH_LLVM_EXTRA_WARNINGS})
