//===- DeclOccurrence.h - An occurrence of a decl within a file -*- C++ -*-===//
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
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_INDEX_DECLOCCURRENCE_H
#define LANGUAGE_CORE_INDEX_DECLOCCURRENCE_H

#include "language/Core/AST/DeclBase.h"
#include "language/Core/Basic/LLVM.h"
#include "language/Core/Index/IndexSymbol.h"
#include "language/Core/Lex/MacroInfo.h"
#include "toolchain/ADT/ArrayRef.h"
#include "toolchain/ADT/PointerUnion.h"
#include "toolchain/ADT/SmallVector.h"

namespace language::Core {
namespace index {

struct DeclOccurrence {
  SymbolRoleSet Roles;
  unsigned Offset;
  toolchain::PointerUnion<const Decl *, const MacroInfo *> DeclOrMacro;
  const IdentifierInfo *MacroName = nullptr;
  SmallVector<SymbolRelation, 3> Relations;

  DeclOccurrence(SymbolRoleSet R, unsigned Offset, const Decl *D,
                 ArrayRef<SymbolRelation> Relations)
      : Roles(R), Offset(Offset), DeclOrMacro(D), Relations(Relations) {}
  DeclOccurrence(SymbolRoleSet R, unsigned Offset, const IdentifierInfo *Name,
                 const MacroInfo *MI)
      : Roles(R), Offset(Offset), DeclOrMacro(MI), MacroName(Name) {}

  friend bool operator<(const DeclOccurrence &LHS, const DeclOccurrence &RHS) {
    return LHS.Offset < RHS.Offset;
  }
};

} // namespace index
} // namespace language::Core

#endif // LANGUAGE_CORE_INDEX_DECLOCCURRENCE_H
