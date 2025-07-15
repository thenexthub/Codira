//===--- GenControl.cpp - IR Generation for Control Flow ------------------===//
//
// Copyright (c) NeXTHub Corporation. All rights reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// This code is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// version 2 for more details (a copy is included in the LICENSE file that
// accompanied this code).
//
// Author(-s): Tunjay Akbarli
//

//===----------------------------------------------------------------------===//
//
//  This file implements general IR generation for control flow.
//
//===----------------------------------------------------------------------===//

#include "toolchain/ADT/STLExtras.h"
#include "toolchain/IR/Function.h"
#include "IRGenFunction.h"
#include "IRGenModule.h"
#include "language/Basic/Assertions.h"

using namespace language;
using namespace irgen;

/// Insert the given basic block after the IP block and move the
/// insertion point to it.  Only valid if the IP is valid.
void IRBuilder::emitBlock(toolchain::BasicBlock *BB) {
  assert(ClearedIP == nullptr);
  toolchain::BasicBlock *CurBB = GetInsertBlock();
  assert(CurBB && "current insertion point is invalid");
  CurBB->getParent()->insert(std::next(CurBB->getIterator()), BB);
  IRBuilderBase::SetInsertPoint(BB);
}

/// Create a new basic block with the given name.  The block is not
/// automatically inserted into the function.
toolchain::BasicBlock *
IRGenFunction::createBasicBlock(const toolchain::Twine &Name) {
  return toolchain::BasicBlock::Create(IGM.getLLVMContext(), Name);
}

