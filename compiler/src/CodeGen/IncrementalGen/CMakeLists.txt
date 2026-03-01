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

file(GLOB CodeGenIncrementalGen *.cpp)

set(INCREMENTAL_COMPILATION_SRC ${CodeGenIncrementalGen})

# The libraries of llvm are not compiled with RTTI, since
# some llvm classes without typeinfo are used in codegen
# incremental compilation part, it is required to set
# `-fno-rtti` flag to compile this part.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")

add_library(CodiraCodeGenIncrementalGen OBJECT ${INCREMENTAL_COMPILATION_SRC})

# use llvm
if(CODIRA_CODEGEN_CODENATIVE_BACKEND)
    add_dependencies(CodiraCodeGenIncrementalGen codenative)
endif()
target_include_directories(CodiraCodeGenIncrementalGen PRIVATE ${LLVM_INCLUDE_DIRS})
