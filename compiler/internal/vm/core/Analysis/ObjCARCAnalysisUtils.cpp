//===- ObjCARCAnalysisUtils.cpp -------------------------------------------===//
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
//
// This file implements common infrastructure for libLLVMObjCARCOpts.a, which
// implements several scalar transformations over the LLVM intermediate
// representation, including the C bindings for that library.
//
//===----------------------------------------------------------------------===//

#include "vm/core/Analysis/ObjCARCAnalysisUtils.h"
#include "vm/core/Analysis/AliasAnalysis.h"
#include "vm/core/Support/CommandLine.h"

using namespace vm::core;
using namespace vm::core::objcarc;

/// A handy option to enable/disable all ARC Optimizations.
bool toolchain::objcarc::EnableARCOpts;
static cl::opt<bool, true> EnableARCOptimizations(
    "enable-objc-arc-opts", cl::desc("enable/disable all ARC Optimizations"),
    cl::location(EnableARCOpts), cl::init(true), cl::Hidden);

bool toolchain::objcarc::IsPotentialRetainableObjPtr(const Value *Op,
                                                AAResults &AA) {
  // First make the rudimentary check.
  if (!IsPotentialRetainableObjPtr(Op))
    return false;

  // Objects in constant memory are not reference-counted.
  if (AA.pointsToConstantMemory(Op))
    return false;

  // Pointers in constant memory are not pointing to reference-counted objects.
  if (const LoadInst *LI = dyn_cast<LoadInst>(Op))
    if (AA.pointsToConstantMemory(LI->getPointerOperand()))
      return false;

  // Otherwise assume the worst.
  return true;
}
