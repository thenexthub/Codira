//===--- ValueUtils.h -----------------------------------------------------===//
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

#ifndef LANGUAGE_SIL_VALUEUTILS_H
#define LANGUAGE_SIL_VALUEUTILS_H

#include "language/SIL/SILValue.h"

namespace language {

/// Attempt to merge the ValueOwnershipKind of the passed in range's
/// SILValues. Returns OwnershipKind::Any if we found an incompatibility.
/// If \p is a move-only type, we return OwnershipKind::Owned since such
/// values can have deinit side-effects.
///
/// NOTE: This assumes that the passed in SILValues are not values used as type
/// dependent operands.
ValueOwnershipKind getSILValueOwnership(ArrayRef<SILValue> values,
                                        SILType ty = SILType());

} // namespace language

#endif
