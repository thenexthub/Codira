//===- Mutations.cpp ------------------------------------------*- C++ -*-=====//
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
#include "language/Core/Tooling/Syntax/Mutations.h"
#include "language/Core/Basic/LLVM.h"
#include "language/Core/Tooling/Syntax/BuildTree.h"
#include "language/Core/Tooling/Syntax/Nodes.h"
#include "language/Core/Tooling/Syntax/Tree.h"
#include <cassert>

using namespace language::Core;

// This class has access to the internals of tree nodes. Its sole purpose is to
// define helpers that allow implementing the high-level mutation operations.
class syntax::MutationsImpl {
public:
  /// Add a new node with a specified role.
  static void addAfter(syntax::Node *Anchor, syntax::Node *New, NodeRole Role) {
    assert(Anchor != nullptr);
    assert(Anchor->Parent != nullptr);
    assert(New->Parent == nullptr);
    assert(New->NextSibling == nullptr);
    assert(New->PreviousSibling == nullptr);
    assert(New->isDetached());
    assert(Role != NodeRole::Detached);

    New->setRole(Role);
    auto *P = Anchor->getParent();
    P->replaceChildRangeLowLevel(Anchor->getNextSibling(),
                                 Anchor->getNextSibling(), New);

    P->assertInvariants();
  }

  /// Replace the node, keeping the role.
  static void replace(syntax::Node *Old, syntax::Node *New) {
    assert(Old != nullptr);
    assert(Old->Parent != nullptr);
    assert(Old->canModify());
    assert(New->Parent == nullptr);
    assert(New->NextSibling == nullptr);
    assert(New->PreviousSibling == nullptr);
    assert(New->isDetached());

    New->Role = Old->Role;
    auto *P = Old->getParent();
    P->replaceChildRangeLowLevel(Old, Old->getNextSibling(), New);

    P->assertInvariants();
  }

  /// Completely remove the node from its parent.
  static void remove(syntax::Node *N) {
    assert(N != nullptr);
    assert(N->Parent != nullptr);
    assert(N->canModify());

    auto *P = N->getParent();
    P->replaceChildRangeLowLevel(N, N->getNextSibling(),
                                 /*New=*/nullptr);

    P->assertInvariants();
    N->assertInvariants();
  }
};

void syntax::removeStatement(syntax::Arena &A, TokenBufferTokenManager &TBTM,
                             syntax::Statement *S) {
  assert(S);
  assert(S->canModify());

  if (isa<CompoundStatement>(S->getParent())) {
    // A child of CompoundStatement can just be safely removed.
    MutationsImpl::remove(S);
    return;
  }
  // For the rest, we have to replace with an empty statement.
  if (isa<EmptyStatement>(S))
    return; // already an empty statement, nothing to do.

  MutationsImpl::replace(S, createEmptyStatement(A, TBTM));
}
