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


set(AST_SERIALIZATION_SRC NodeSerialization.cpp ExprSerialization.cpp TokenSerialization.cpp MacroCommon.cpp MacroCall.cpp)
add_library(CodiraAstSerialization OBJECT ${AST_SERIALIZATION_SRC})
add_dependencies(CodiraAstSerialization CodiraFlatbuffersHeaders)
target_include_directories(CodiraAstSerialization PRIVATE ${FLATBUFFERS_INCLUDE_DIR})

if(CODIRA_BUILD_CODEC)
    set(MACRO_SRC
        MacroExpansion.cpp
        MacroProcess.cpp
        MacroEvaluation.cpp
        InvokeUtil.cpp
        MacroCallResolve.cpp
        TestEntryConstructor.cpp)
    list(APPEND MACRO_SRC MacroEvaluationSrv.cpp MacroEvaluationClient.cpp MacroEvalMsgSerializer.cpp)
    if(CODIRA_CODEGEN_CODENATIVE_BACKEND)
        list(APPEND MACRO_SRC MacroEvaluationCODENative.cpp)
        list(APPEND MACRO_SRC InvokeUtilCODENative.cpp)
    endif()
    add_library(CodiraMacro OBJECT ${MACRO_SRC})
    add_dependencies(CodiraMacro CodiraFlatbuffersHeaders)
    target_include_directories(CodiraMacro PRIVATE ${FLATBUFFERS_INCLUDE_DIR})

    if(CODIRA_CODEGEN_CODENATIVE_BACKEND)
        add_dependencies(CodiraMacro codenative)
    endif()
    target_include_directories(CodiraMacro PRIVATE ${LLVM_INCLUDE_DIRS})
    target_include_directories(CodiraMacro PRIVATE ${BOUNDSCHECK}/include)
    target_compile_options(CodiraMacro PRIVATE ${CODEC_EXTRA_WARNINGS})
endif()
