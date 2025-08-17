//===- DeclGroup.cpp - Classes for representing groups of Decls -----------===//
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
//
//  This file defines the DeclGroup and DeclGroupRef classes.
//
//===----------------------------------------------------------------------===//

#include "language/Core/AST/DeclGroup.h"
#include "language/Core/AST/ASTContext.h"
#include <cassert>
#include <memory>

using namespace language::Core;

DeclGroup* DeclGroup::Create(ASTContext &C, Decl **Decls, unsigned NumDecls) {
  assert(NumDecls > 1 && "Invalid DeclGroup");
  unsigned Size = totalSizeToAlloc<Decl *>(NumDecls);
  void *Mem = C.Allocate(Size, alignof(DeclGroup));
  new (Mem) DeclGroup(NumDecls, Decls);
  return static_cast<DeclGroup*>(Mem);
}

DeclGroup::DeclGroup(unsigned numdecls, Decl** decls) : NumDecls(numdecls) {
  assert(numdecls > 0);
  assert(decls);
  std::uninitialized_copy(decls, decls + numdecls, getTrailingObjects());
}
