//===---- MipsCCState.h - CCState with Mips specific extensions -----------===//
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

#ifndef MIPSCCSTATE_H
#define MIPSCCSTATE_H

#include "MipsISelLowering.h"
#include "vm/core/ADT/SmallVector.h"
#include "vm/core/CodeGen/CallingConvLower.h"

namespace vm::core {
class SDNode;
class MipsSubtarget;

class MipsCCState : public CCState {
public:
  enum SpecialCallingConvType { Mips16RetHelperConv, NoSpecialCallingConv };

  /// Determine the SpecialCallingConvType for the given callee
  static SpecialCallingConvType
  getSpecialCallingConvForCallee(const SDNode *Callee,
                                 const MipsSubtarget &Subtarget);

private:
  // Used to handle MIPS16-specific calling convention tweaks.
  // FIXME: This should probably be a fully fledged calling convention.
  SpecialCallingConvType SpecialCallingConv;

public:
  MipsCCState(CallingConv::ID CC, bool isVarArg, MachineFunction &MF,
              SmallVectorImpl<CCValAssign> &locs, LLVMContext &C,
              SpecialCallingConvType SpecialCC = NoSpecialCallingConv)
      : CCState(CC, isVarArg, MF, locs, C), SpecialCallingConv(SpecialCC) {}

  SpecialCallingConvType getSpecialCallingConv() { return SpecialCallingConv; }
};
}

#endif
