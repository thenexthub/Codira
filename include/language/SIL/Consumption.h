//===--- Consumption.h - Value consumption for SIL --------------*- C++ -*-===//
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
// This file defines the CastConsumptionKind enum, which describes
// under what circumstances an operation consumes a value.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_SIL_CONSUMPTION_H
#define LANGUAGE_SIL_CONSUMPTION_H

#include "toolchain/Support/ErrorHandling.h"
#include <cstdint>

namespace language {

/// Is an operation a "take"?  A take consumes the original value,
/// leaving it uninitialized.
enum IsTake_t : bool { IsNotTake, IsTake };

/// Is an operation an "initialization"?  An initialization simply
/// fills in an uninitialized address with a value; a
/// non-initialization also consumes the value already there.
enum IsInitialization_t : bool { IsNotInitialization, IsInitialization };

/// The behavior of a dynamic cast operation on the source value.
enum class CastConsumptionKind : uint8_t {
  /// The source value is always taken, regardless of whether the cast
  /// succeeds.  That is, if the cast fails, the source value is
  /// destroyed.
  TakeAlways,

  /// The source value is taken only on a successful cast; otherwise,
  /// it is left in place.
  TakeOnSuccess,

  /// The source value is always left in place, and the destination
  /// value is copied into on success.
  CopyOnSuccess,

  /// The source value is never taken, regardless of whether the cast
  /// succeeds. Instead, we always borrow the source value and feed it through.
  ///
  /// NOTE: This can only be used with objects. We do not support borrowing of
  /// addresses. If an address is needed for a cast operation, a BorrowAlways
  /// value must be copied into a temporary and operated upon. If the result of
  /// the cast is a loadable type then the value is loaded using a
  /// load_borrow. If an address only value is returned, we continue processing
  /// the value as an owned TakeAlways value.
  BorrowAlways,
};

/// Should the source value be destroyed if the cast fails?
inline bool shouldDestroyOnFailure(CastConsumptionKind kind) {
  switch (kind) {
  case CastConsumptionKind::TakeAlways:
    return true;
  case CastConsumptionKind::TakeOnSuccess:
  case CastConsumptionKind::CopyOnSuccess:
  case CastConsumptionKind::BorrowAlways:
    return false;
  }
  toolchain_unreachable("covered switch");
}

/// Should the source value be taken if the cast succeeds?
inline IsTake_t shouldTakeOnSuccess(CastConsumptionKind kind) {
  switch (kind) {
  case CastConsumptionKind::TakeAlways:
  case CastConsumptionKind::TakeOnSuccess:
    return IsTake;
  case CastConsumptionKind::CopyOnSuccess:
  case CastConsumptionKind::BorrowAlways:
    return IsNotTake;
  }
  toolchain_unreachable("covered switch");
}

} // end namespace language

#endif
