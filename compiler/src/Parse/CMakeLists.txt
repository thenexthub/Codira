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

file(GLOB PARSE_SRC *.cpp)

set(PARSE_FFI_SRC
    AttachComment.cpp
    ParseAnnotations.cpp
    ParseAtom.cpp
    ParseCODEMPDecl.cpp
    ParseDecl.cpp
    ParseExpr.cpp
    ParseImports.cpp
    ParseFeatures.cpp
    ParseMacro.cpp
    ParseModifiers.cpp
    ParsePattern.cpp
    ParseQuote.cpp
    ParseType.cpp
    Parser.cpp
    ParserDiag.cpp
    ParserImpl.cpp
    ParserModifierRules.cpp
    ParserUtils.cpp
)

add_subdirectory(NativeFFI)

if(NOT CMAKE_ENABLE_ASSERT
   AND NOT CMAKE_BUILD_TYPE MATCHES Debug)
    get_filename_component(ASTChecker ${CMAKE_CURRENT_SOURCE_DIR}/ASTChecker.cpp ABSOLUTE)
    list(REMOVE_ITEM PARSE_SRC ${ASTChecker})
endif()

add_library(CodiraParse OBJECT ${PARSE_SRC})
add_library(CodiraParseFFI OBJECT ${PARSE_FFI_SRC})
target_compile_options(CodiraParse PRIVATE ${CODEC_EXTRA_WARNINGS})
