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

file(GLOB SEMA_SRC *.cpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(Desugar)
add_subdirectory(FFI)
add_subdirectory(NativeFFI)
add_subdirectory(GenericInstantiation)
add_subdirectory(InheritanceChecker)
add_subdirectory(LegalityOfUsage)
add_subdirectory(Test)
add_subdirectory(TypeCheckExpr)
add_subdirectory(CODEMP)
add_subdirectory(Plugin)

add_library(CodiraSema OBJECT ${SEMA_SRC})
target_compile_options(CodiraSema PRIVATE ${CODEC_EXTRA_WARNINGS})
