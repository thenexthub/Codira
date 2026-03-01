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

file(GLOB CHIR_BASE_SRC *.cpp)
add_subdirectory(Analysis)
add_subdirectory(AST2CHIR)
add_subdirectory(Checker)
add_subdirectory(Expression)
add_subdirectory(GenerateVTable)
add_subdirectory(Type)
add_subdirectory(Visitor)
add_subdirectory(NativeFFI)

add_library(CodiraCHIRBase OBJECT ${CHIR_BASE_SRC})

add_subdirectory(Interpreter)
add_subdirectory(Serializer)
add_subdirectory(Transformation)
add_library(CodiraCHIRExtra OBJECT ${CHIR_OPT_SRC} ${CHIRInterpreter} ${CHIRSerializer} ${ComputeAnnotations})
target_include_directories(CodiraCHIRExtra PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/AST2CHIR/TranslateASTNode/)

# the interpreter needs third-party and also flatbuffers
target_include_directories(CodiraCHIRExtra PRIVATE ${BOUNDSCHECK}/include)
add_dependencies(CodiraCHIRExtra CodiraFlatbuffersHeaders)
target_include_directories(CodiraCHIRExtra PRIVATE ${FLATBUFFERS_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/Annotations)

# CHIR.cpp indirectly includes PackageFormat_generated.h, this will be refactored later.
target_include_directories(CodiraCHIRBase PRIVATE ${FLATBUFFERS_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/Annotations)
add_dependencies(CodiraCHIRBase CodiraFlatbuffersHeaders)

target_link_libraries(CodiraCHIRExtra)
target_compile_options(CodiraCHIRBase PRIVATE ${CODEC_EXTRA_WARNINGS})
target_compile_options(CodiraCHIRExtra PRIVATE ${CODEC_EXTRA_WARNINGS})
