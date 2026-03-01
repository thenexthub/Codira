//===- ValueList.cpp - Internal BitcodeReader implementation --------------===//
//
// Copyright (c) NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
// http://www.apache.org/licenses/LICENSE-2.0
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

#include "ValueList.h"
#include "vm/core/IR/Argument.h"
#include "vm/core/IR/Constant.h"
#include "vm/core/IR/GlobalValue.h"
#include "vm/core/IR/Instruction.h"
#include "vm/core/IR/Type.h"
#include "vm/core/IR/Value.h"
#include "vm/core/Support/Casting.h"
#include "vm/core/Support/Error.h"

using namespace vm::core;

Error BitcodeReaderValueList::assignValue(unsigned Idx, Value *V,
                                          unsigned TypeID) {
  if (Idx == size()) {
    push_back(V, TypeID);
    return Error::success();
  }

  if (Idx >= size())
    resize(Idx + 1);

  auto &Old = ValuePtrs[Idx];
  if (!Old.first) {
    Old.first = V;
    Old.second = TypeID;
    return Error::success();
  }

  assert(!isa<Constant>(&*Old.first) && "Shouldn't update constant");
  // If there was a forward reference to this value, replace it.
  Value *PrevVal = Old.first;
  if (PrevVal->getType() != V->getType())
    return createStringError(
        std::errc::illegal_byte_sequence,
        "Assigned value does not match type of forward declaration");
  Old.first->replaceAllUsesWith(V);
  PrevVal->deleteValue();
  return Error::success();
}

Value *BitcodeReaderValueList::getValueFwdRef(unsigned Idx, Type *Ty,
                                              unsigned TyID,
                                              BasicBlock *ConstExprInsertBB) {
  // Bail out for a clearly invalid value.
  if (Idx >= RefsUpperBound)
    return nullptr;

  if (Idx >= size())
    resize(Idx + 1);

  if (Value *V = ValuePtrs[Idx].first) {
    // If the types don't match, it's invalid.
    if (Ty && Ty != V->getType())
      return nullptr;

    Expected<Value *> MaybeV = MaterializeValueFn(Idx, ConstExprInsertBB);
    if (!MaybeV) {
      // TODO: We might want to propagate the precise error message here.
      consumeError(MaybeV.takeError());
      return nullptr;
    }
    return MaybeV.get();
  }

  // No type specified, must be invalid reference.
  if (!Ty)
    return nullptr;

  // Create and return a placeholder, which will later be RAUW'd.
  Value *V = new Argument(Ty);
  ValuePtrs[Idx] = {V, TyID};
  return V;
}
