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

set(FRONTENDTOOL_SRC FrontendTool.cpp DefaultCompilerInstance.cpp IncrementalCompilerInstance.cpp)

add_library(CodiraFrontendTool OBJECT ${FRONTENDTOOL_SRC})
add_dependencies(CodiraFrontendTool CodiraFlatbuffersHeaders)
target_include_directories(CodiraFrontendTool PRIVATE ${FLATBUFFERS_INCLUDE_DIR})
# use llvm
if(CODIRA_CODEGEN_CODENATIVE_BACKEND)
    add_dependencies(CodiraFrontendTool codenative)
endif()
target_include_directories(CodiraFrontendTool PRIVATE ${BOUNDSCHECK}/include)
target_include_directories(CodiraFrontendTool PRIVATE ${LLVM_INCLUDE_DIRS})
target_compile_options(CodiraFrontendTool PRIVATE ${CODEC_WITH_LLVM_EXTRA_WARNINGS})
