//===--- ApplySite.cpp - Wrapper around apply instructions ----------------===//
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

#include "language/SIL/ApplySite.h"
#include "language/SIL/SILBuilder.h"


using namespace language;

void ApplySite::insertAfterInvocation(function_ref<void(SILBuilder &)> fn) const {
  SILBuilderWithScope::insertAfter(getInstruction(), fn);
}

void ApplySite::insertAfterApplication(
    function_ref<void(SILBuilder &)> fn) const {
  switch (getKind()) {
  case ApplySiteKind::ApplyInst:
  case ApplySiteKind::TryApplyInst:
  case ApplySiteKind::PartialApplyInst:
    return insertAfterInvocation(fn);
  case ApplySiteKind::BeginApplyInst:
    SmallVector<EndApplyInst *, 2> endApplies;
    SmallVector<AbortApplyInst *, 2> abortApplies;
    auto *bai = cast<BeginApplyInst>(getInstruction());
    bai->getCoroutineEndPoints(endApplies, abortApplies);
    for (auto *eai : endApplies) {
      SILBuilderWithScope builder(std::next(eai->getIterator()));
      fn(builder);
    }
    for (auto *aai : abortApplies) {
      SILBuilderWithScope builder(std::next(aai->getIterator()));
      fn(builder);
    }
    return;
  }
  toolchain_unreachable("covered switch isn't covered");
}

bool ApplySite::isAddressable(const Operand &operand) const {
  unsigned calleeArgIndex = getCalleeArgIndex(operand);
  assert(calleeArgIndex >= getSubstCalleeConv().getSILArgIndexOfFirstParam());
  unsigned paramIdx =
    calleeArgIndex - getSubstCalleeConv().getSILArgIndexOfFirstParam();

  CanSILFunctionType calleeType = getSubstCalleeType();
  return calleeType->isAddressable(paramIdx, getFunction());
}
