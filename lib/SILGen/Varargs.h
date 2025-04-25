//===--- Varargs.h - SIL generation for (native) Swift varargs --*- C++ -*-===//
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
// A storage structure for holding a destructured rvalue with an optional
// cleanup(s).
// Ownership of the rvalue can be "forwarded" to disable the associated
// cleanup(s).
//
//===----------------------------------------------------------------------===//

#ifndef SWIFT_LOWERING_VARARGS_H
#define SWIFT_LOWERING_VARARGS_H

#include "ManagedValue.h"
#include "language/SIL/AbstractionPattern.h"

namespace language {
namespace Lowering {
class SILGenFunction;
class TypeLowering;

/// Information about a varargs emission.
class VarargsInfo {
  ManagedValue Array;
  CleanupHandle AbortCleanup;
  SILValue BaseAddress;
  AbstractionPattern BasePattern;
  const TypeLowering &BaseTL;
public:
  VarargsInfo(ManagedValue array, CleanupHandle abortCleanup,
              SILValue baseAddress, const TypeLowering &baseTL,
              AbstractionPattern basePattern)
    : Array(array), AbortCleanup(abortCleanup),
      BaseAddress(baseAddress), BasePattern(basePattern), BaseTL(baseTL) {}

  /// Return the array value.  emitEndVarargs() is really the only
  /// function that should be accessing this directly.
  ManagedValue getArray() const {
    return Array;
  }
  CleanupHandle getAbortCleanup() const { return AbortCleanup; }

  /// An address of the lowered type.
  SILValue getBaseAddress() const { return BaseAddress; }

  AbstractionPattern getBaseAbstractionPattern() const {
    return BasePattern;
  }

  const TypeLowering &getBaseTypeLowering() const {
    return BaseTL;
  }
};

/// Begin a varargs emission sequence.
VarargsInfo emitBeginVarargs(SILGenFunction &SGF, SILLocation loc,
                             CanType baseTy, CanType arrayTy,
                             unsigned numElements);

/// Successfully end a varargs emission sequence.
ManagedValue emitEndVarargs(SILGenFunction &SGF, SILLocation loc,
                            VarargsInfo &&varargs,
                             unsigned numElements); 

} // end namespace Lowering
} // end namespace language

#endif
