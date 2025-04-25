//===--- ValueUtils.cpp ---------------------------------------------------===//
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

#include "language/SIL/ValueUtils.h"
#include "language/Basic/Assertions.h"
#include "language/Basic/STLExtras.h"
#include "language/SIL/SILFunction.h"

using namespace language;

ValueOwnershipKind swift::getSILValueOwnership(ArrayRef<SILValue> values,
                                               SILType ty) {
  auto range = makeTransformRange(values, [](SILValue v) {
    assert(v->getType().isObject());
    return v->getOwnershipKind();
  });

  auto mergedOwnership = ValueOwnershipKind::merge(range);

  // If we have a move only type, return owned ownership.
  if (ty && ty.isMoveOnly() && mergedOwnership == OwnershipKind::None) {
    return OwnershipKind::Owned;
  }
  return mergedOwnership;
}
