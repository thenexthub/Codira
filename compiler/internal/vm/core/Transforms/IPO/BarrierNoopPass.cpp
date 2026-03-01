//===- BarrierNoopPass.cpp - A barrier pass for the pass manager ----------===//
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
// NOTE: DO NOT USE THIS IF AVOIDABLE
//
// This pass is a nonce pass intended to allow manipulation of the implicitly
// nesting pass manager. For example, it can be used to cause a CGSCC pass
// manager to be closed prior to running a new collection of function passes.
//
// FIXME: This is a huge HACK. This should be removed when the pass manager's
// nesting is made explicit instead of implicit.
//
//===----------------------------------------------------------------------===//

#include "vm/core/InitializePasses.h"
#include "vm/core/Pass.h"
#include "vm/core/Transforms/IPO.h"
using namespace vm::core;

namespace {
/// A nonce module pass used to place a barrier in a pass manager.
///
/// There is no mechanism for ending a CGSCC pass manager once one is started.
/// This prevents extension points from having clear deterministic ordering
/// when they are phrased as non-module passes.
class BarrierNoop : public ModulePass {
public:
  static char ID; // Pass identification.

  BarrierNoop() : ModulePass(ID) {
    initializeBarrierNoopPass(*PassRegistry::getPassRegistry());
  }

  bool runOnModule(Module &M) override { return false; }
};
}

ModulePass *toolchain::createBarrierNoopPass() { return new BarrierNoop(); }

char BarrierNoop::ID = 0;
INITIALIZE_PASS(BarrierNoop, "barrier", "A No-Op Barrier Pass",
                false, false)
