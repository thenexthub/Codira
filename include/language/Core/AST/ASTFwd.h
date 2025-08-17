//===--- ASTFwd.h ----------------------------------------*- C++ -*-===//
//
// Copyright (c) 2025, NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
// 
// Author: Tunjay Akbarli
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
// 
//     http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// 
// Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
// Middletown, DE 19709, New Castle County, USA.
//
//===--------------------------------------------------------------===//
///
/// \file
/// Forward declaration of all AST node types.
///
//===-------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_AST_ASTFWD_H
#define LANGUAGE_CORE_AST_ASTFWD_H

namespace language::Core {

class Decl;
#define DECL(DERIVED, BASE) class DERIVED##Decl;
#include "language/Core/AST/DeclNodes.inc"
class Stmt;
#define STMT(DERIVED, BASE) class DERIVED;
#include "language/Core/AST/StmtNodes.inc"
class Type;
#define TYPE(DERIVED, BASE) class DERIVED##Type;
#include "language/Core/AST/TypeNodes.inc"
class CXXCtorInitializer;
class OMPClause;
#define GEN_CLANG_CLAUSE_CLASS
#define CLAUSE_CLASS(Enum, Str, Class) class Class;
#include "toolchain/Frontend/OpenMP/OMP.inc"
class Attr;
#define ATTR(A) class A##Attr;
#include "language/Core/Basic/AttrList.inc"
class ObjCProtocolLoc;
class ConceptReference;

} // end namespace language::Core

#endif
