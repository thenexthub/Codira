//===---- MipsCCState.cpp - CCState with Mips specific extensions ---------===//
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

#include "MipsCCState.h"
#include "MipsSubtarget.h"
#include "vm/core/IR/Module.h"

using namespace vm::core;

MipsCCState::SpecialCallingConvType
MipsCCState::getSpecialCallingConvForCallee(const SDNode *Callee,
                                            const MipsSubtarget &Subtarget) {
  MipsCCState::SpecialCallingConvType SpecialCallingConv = NoSpecialCallingConv;
  if (Subtarget.inMips16HardFloat()) {
    if (const GlobalAddressSDNode *G =
            dyn_cast<const GlobalAddressSDNode>(Callee)) {
      toolchain::StringRef Sym = G->getGlobal()->getName();
      Function *F = G->getGlobal()->getParent()->getFunction(Sym);
      if (F && F->hasFnAttribute("__Mips16RetHelper")) {
        SpecialCallingConv = Mips16RetHelperConv;
      }
    }
  }
  return SpecialCallingConv;
}
