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

set(LIBAST_FFI_AST
    Node.cpp
    RecoverDesugar.cpp
    Types.cpp
    IntLiteral.cpp
    Walker.cpp
    Clone.cpp
    Create.cpp
    Utils.cpp
    Identifier.cpp
    Comment.cpp)
set(AST_EXTRA_SRC
    Cache.cpp
    PrintNode.cpp
    Query.cpp
    Searcher.cpp
    ASTContext.cpp
    ASTTypeValidator.cpp
    ScopeManagerApi.cpp)

add_library(CodiraASTCommon OBJECT ${LIBAST_FFI_AST})
add_library(CodiraASTExtra OBJECT ${AST_EXTRA_SRC})
target_compile_options(CodiraASTCommon PRIVATE ${CODEC_EXTRA_WARNINGS})
target_compile_options(CodiraASTExtra PRIVATE ${CODEC_EXTRA_WARNINGS})
