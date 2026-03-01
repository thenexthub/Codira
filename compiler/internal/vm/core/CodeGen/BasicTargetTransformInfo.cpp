//===- BasicTargetTransformInfo.cpp - Basic target-independent TTI impl ---===//
//
// Copyright (c) NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
// Middletown, DE 19709, New Castle County, USA.
//
//===----------------------------------------------------------------------===//
/// \file
/// This file provides the implementation of a basic TargetTransformInfo pass
/// predicated on the target abstractions present in the target independent
/// code generator. It uses these (primarily TargetLowering) to model as much
/// of the TTI query interface as possible. It is included by most targets so
/// that they can specialize only a small subset of the query space.
///
//===----------------------------------------------------------------------===//

#include "vm/core/CodeGen/BasicTTIImpl.h"
#include "vm/core/CodeGen/TargetSubtargetInfo.h"
#include "vm/core/IR/Function.h"
#include "vm/core/Support/CommandLine.h"
#include "vm/core/Target/TargetMachine.h"

using namespace vm::core;

// This flag is used by the template base class for BasicTTIImpl, and here to
// provide a definition.
cl::opt<unsigned>
toolchain::PartialUnrollingThreshold("partial-unrolling-threshold", cl::init(0),
                                cl::desc("Threshold for partial unrolling"),
                                cl::Hidden);

BasicTTIImpl::BasicTTIImpl(const TargetMachine *TM, const Function &F)
    : BaseT(TM, F.getDataLayout()), ST(TM->getSubtargetImpl(F)),
      TLI(ST->getTargetLowering()) {}
