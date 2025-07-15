//===--- GenDistributed.h - IRGen for distributed features ------*- C++ -*-===//
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
// This file defines interfaces for emitting code for various distributed
// features.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_IRGEN_GENDISTRIBUTED_H
#define LANGUAGE_IRGEN_GENDISTRIBUTED_H

#include "language/AST/Types.h"
#include "language/Basic/Toolchain.h"
#include "language/SIL/ApplySite.h"
#include "toolchain/IR/CallingConv.h"

#include "Callee.h"
#include "GenHeap.h"
#include "IRGenModule.h"

namespace toolchain {
class Value;
}

namespace language {
class CanType;
class ProtocolConformanceRef;
class SILType;

namespace irgen {
class Explosion;
class IRGenFunction;

/// Emit the '_distributedActorRemoteInitialize' call.
toolchain::Value *emitDistributedActorInitializeRemote(
    IRGenFunction &IGF,
    SILType selfType,
    toolchain::Value *actorMetatype,
    Explosion &out);

} // end namespace irgen
} // end namespace language

#endif
