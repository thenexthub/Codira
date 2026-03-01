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

file(GLOB CodeGenBase
    *.cpp
    Base/*.cpp
    Base/CGTypes/*.cpp
    Base/ExprDispatcher/*.cpp
    Utils/*.cpp
)

include_directories(BEFORE ${CMAKE_CURRENT_SOURCE_DIR})

if(CODIRA_CODEGEN_CODENATIVE_BACKEND)
    file(GLOB CodeGenCODENative CODENative/*.cpp CODENative/CGTypes/*.cpp)
    list(REMOVE_ITEM CodeGenCODENative ${CMAKE_CURRENT_SOURCE_DIR}/CODENative/CODENativeGenArray.cpp)
    set(CODEGEN_SRC ${CodeGenBase} ${CodeGenCODENative})
    add_subdirectory(IncrementalGen)
endif()

add_library(CodiraCodeGen OBJECT ${CODEGEN_SRC})

if(CODIRA_CODEGEN_CODENATIVE_BACKEND)
    add_dependencies(CodiraCodeGen codenative)
endif()
target_include_directories(CodiraCodeGen PRIVATE ${LLVM_INCLUDE_DIRS})
target_compile_options(CodiraCodeGen PRIVATE ${CODEC_WITH_LLVM_EXTRA_WARNINGS})
