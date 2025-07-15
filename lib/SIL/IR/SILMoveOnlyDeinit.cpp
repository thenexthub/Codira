//===--- SILMoveOnlyDeinit.cpp ---------------------------------------===//
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

#include "language/Basic/Assertions.h"
#include "language/SIL/SILMoveOnlyDeinit.h"
#include "language/SIL/SILModule.h"
#include "toolchain/Analysis/ValueTracking.h"

using namespace language;

SILMoveOnlyDeinit *SILMoveOnlyDeinit::create(SILModule &mod,
                                             NominalTypeDecl *nominalDecl,
                                             SerializedKind_t serialized,
                                             SILFunction *funcImpl) {
  auto buf =
      mod.allocate(sizeof(SILMoveOnlyDeinit), alignof(SILMoveOnlyDeinit));
  auto *table =
      ::new (buf) SILMoveOnlyDeinit(nominalDecl, funcImpl, unsigned(serialized));
  mod.moveOnlyDeinits.push_back(table);
  mod.MoveOnlyDeinitMap[nominalDecl] = table;
  return table;
}

SILMoveOnlyDeinit::SILMoveOnlyDeinit(NominalTypeDecl *nominalDecl,
                                     SILFunction *implementation,
                                     unsigned serialized)
    : nominalDecl(nominalDecl), funcImpl(implementation),
      serialized(serialized) {
  assert(funcImpl);
  funcImpl->incrementRefCount();
}

SILMoveOnlyDeinit::~SILMoveOnlyDeinit() {
  if (funcImpl)
    funcImpl->decrementRefCount();
}
