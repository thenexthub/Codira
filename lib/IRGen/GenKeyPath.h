//===--- GenKeyPath.h - IR generation for KeyPath ---------------*- C++ -*-===//
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
//  This file provides the private interface to the emission of KeyPath
//
//===----------------------------------------------------------------------===//

#ifndef SWIFT_IRGEN_GENKEYPATH_H
#define SWIFT_IRGEN_GENKEYPATH_H

#include "GenericRequirement.h"
#include "language/AST/SubstitutionMap.h"
#include "language/Basic/LLVM.h"
#include "language/SIL/SILValue.h"
#include "llvm/IR/Value.h"

namespace language {
namespace irgen {
class Explosion;
class IRGenFunction;
class StackAddress;

std::pair<llvm::Value *, llvm::Value *>
emitKeyPathArgument(IRGenFunction &IGF, SubstitutionMap subs,
                    const CanGenericSignature &sig,
                    ArrayRef<SILType> indiceTypes, Explosion &indiceValues,
                    std::optional<StackAddress> &dynamicArgsBuf);
} // end namespace irgen
} // end namespace language

#endif
