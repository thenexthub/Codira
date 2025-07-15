//===--- GenIntegerLiteral.h - IRGen for Builtin.IntegerLiteral -*- C++ -*-===//
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
// This file defines interfaces for emitting code for Builtin.IntegerLiteral
// values.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_IRGEN_GENINTEGERLITERAL_H
#define LANGUAGE_IRGEN_GENINTEGERLITERAL_H

#include "language/Basic/APIntMap.h"

namespace toolchain {
class Constant;
class IntegerType;
class Type;
class Value;
}

namespace language {
class IntegerLiteralInst;

namespace irgen {
class Explosion;
class IRGenFunction;
class IRGenModule;

/// A constant integer literal value.
struct ConstantIntegerLiteral {
  toolchain::Constant *Data;
  toolchain::Constant *Flags;
};

/// A map for caching globally-emitted constant integers.
class ConstantIntegerLiteralMap {
  APIntMap<ConstantIntegerLiteral> map;

public:
  ConstantIntegerLiteralMap() {}

  ConstantIntegerLiteral get(IRGenModule &IGM, APInt &&value);
};

/// Construct a constant IntegerLiteral from an IntegerLiteralInst.
ConstantIntegerLiteral
emitConstantIntegerLiteral(IRGenModule &IGM, IntegerLiteralInst *ILI);

/// Emit a checked truncation of an IntegerLiteral value.
void emitIntegerLiteralCheckedTrunc(IRGenFunction &IGF, Explosion &in,
                                    toolchain::Type *FromTy,
                                    toolchain::IntegerType *resultTy,
                                    bool resultIsSigned, Explosion &out);

/// Emit a sitofp operation on an IntegerLiteral value.
toolchain::Value *emitIntegerLiteralToFP(IRGenFunction &IGF,
                                    Explosion &in,
                                    toolchain::Type *toType);

toolchain::Value *emitIntLiteralBitWidth(IRGenFunction &IGF, Explosion &in);
toolchain::Value *emitIntLiteralIsNegative(IRGenFunction &IGF, Explosion &in);
toolchain::Value *emitIntLiteralWordAtIndex(IRGenFunction &IGF, Explosion &in);

}
}

#endif
