//===--- GenBuiltin.h - IR generation for builtin functions -----*- C++ -*-===//
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
//  This file provides the private interface to the emission of builtin
//  functions in Swift.
//
//===----------------------------------------------------------------------===//

#ifndef SWIFT_IRGEN_GENBUILTIN_H
#define SWIFT_IRGEN_GENBUILTIN_H

#include "language/AST/SubstitutionMap.h"
#include "language/Basic/LLVM.h"

namespace language {
  class BuiltinInfo;
  class BuiltinInst;
  class Identifier;
  class SILType;

namespace irgen {
  class Explosion;
  class IRGenFunction;

  /// Emit a call to a builtin function.
  void emitBuiltinCall(IRGenFunction &IGF, const BuiltinInfo &builtin,
                       BuiltinInst *Inst, ArrayRef<SILType> argTypes,
                       Explosion &args, Explosion &result);

} // end namespace irgen
} // end namespace language

#endif
