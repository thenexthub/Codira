//===--- TangentBuilder.h - Tangent SIL builder --------------*- C++ -*----===//
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
// This file defines a helper class for emitting tangent code for automatic
// differentiation.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_SILOPTIMIZER_UTILS_DIFFERENTIATION_TANGENTBUILDER_H
#define LANGUAGE_SILOPTIMIZER_UTILS_DIFFERENTIATION_TANGENTBUILDER_H

#include "language/SIL/SILBuilder.h"

namespace language {
namespace autodiff {

class ADContext;

class TangentBuilder: public SILBuilder {
private:
  ADContext &adContext;

public:
  TangentBuilder(SILFunction &fn, ADContext &adContext)
      : SILBuilder(fn), adContext(adContext) {}
  TangentBuilder(SILBasicBlock *bb, ADContext &adContext)
      : SILBuilder(bb), adContext(adContext) {}
  TangentBuilder(SILBasicBlock::iterator insertionPt, ADContext &adContext)
      : SILBuilder(insertionPt), adContext(adContext) {}
  TangentBuilder(SILBasicBlock *bb, SILBasicBlock::iterator insertionPt,
                 ADContext &adContext)
      : SILBuilder(bb, insertionPt), adContext(adContext) {}

  /// Emits an `AdditiveArithmetic.zero` into the given buffer. If it is not an
  /// initialization (`isInit`), a `destroy_addr` will be emitted on the buffer
  /// first. The buffer must have a type that conforms to `AdditiveArithmetic`
  /// or be a tuple thereof.
  void emitZeroIntoBuffer(SILLocation loc, SILValue buffer,
                          IsInitialization_t isInit);

  /// Emits an `AdditiveArithmetic.zero` of the given type. The type must be a
  /// loadable type, and must conform to `AdditiveArithmetic` or be a tuple
  /// thereof.
  SILValue emitZero(SILLocation loc, CanType type);

  /// Emits an `AdditiveArithmetic.+=` for the given destination buffer and
  /// operand. The type of the buffer and the operand must conform to
  /// `AdditiveArithmetic` or be a tuple thereof. The operand will not be
  /// consumed.
  void emitInPlaceAdd(SILLocation loc, SILValue destinationBuffer,
                      SILValue operand);

  /// Emits an `AdditiveArithmetic.+` for the given operands. The type of the
  /// operands must conform to `AdditiveArithmetic` or be a tuple thereof. The
  /// operands will not be consumed.
  void emitAddIntoBuffer(SILLocation loc, SILValue destinationBuffer,
                         SILValue lhsAddress, SILValue rhsAddress);

  /// Emits an `AdditiveArithmetic.+` for the given operands. The type of the
  /// operands must be a loadable type, and must conform to
  /// `AdditiveArithmetic` or be a tuple thereof. The operands will not be
  /// consumed.
  SILValue emitAdd(SILLocation loc, SILValue lhs, SILValue rhs);
};

} // end namespace autodiff
} // end namespace language

#endif /* LANGUAGE_SILOPTIMIZER_UTILS_DIFFERENTIATION_TANGENTBUILDER_H */
