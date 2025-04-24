//===--- ValueUtils.cpp ---------------------------------------------------===//
//
// This source file is part of the Swift.org open source project
//
// Copyright (c) 2014 - 2018 Apple Inc. and the Swift project authors
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See https://swift.org/LICENSE.txt for license information
// See https://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
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
