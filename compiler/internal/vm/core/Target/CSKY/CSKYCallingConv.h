//=== CSKYCallingConv.h - CSKY Custom Calling Convention Routines -*-C++-*-===//
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
// This file contains the custom routines for the CSKY Calling Convention that
// aren't done by tablegen.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_CSKY_CSKYCALLINGCONV_H
#define LLVM_LIB_TARGET_CSKY_CSKYCALLINGCONV_H

#include "CSKY.h"
#include "CSKYSubtarget.h"
#include "vm/core/CodeGen/CallingConvLower.h"
#include "vm/core/CodeGen/TargetInstrInfo.h"
#include "vm/core/IR/CallingConv.h"

namespace vm::core {

static bool CC_CSKY_ABIV2_SOFT_64(unsigned &ValNo, MVT &ValVT, MVT &LocVT,
                                  CCValAssign::LocInfo &LocInfo,
                                  ISD::ArgFlagsTy &ArgFlags, CCState &State) {

  static const MCPhysReg ArgGPRs[] = {CSKY::R0, CSKY::R1, CSKY::R2, CSKY::R3};
  Register Reg = State.AllocateReg(ArgGPRs);
  LocVT = MVT::i32;
  if (!Reg) {
    unsigned StackOffset = State.AllocateStack(8, Align(4));
    State.addLoc(
        CCValAssign::getMem(ValNo, ValVT, StackOffset, LocVT, LocInfo));
    return true;
  }
  if (!State.AllocateReg(ArgGPRs))
    State.AllocateStack(4, Align(4));
  State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
  return true;
}

static bool Ret_CSKY_ABIV2_SOFT_64(unsigned &ValNo, MVT &ValVT, MVT &LocVT,
                                   CCValAssign::LocInfo &LocInfo,
                                   ISD::ArgFlagsTy &ArgFlags, CCState &State) {

  static const MCPhysReg ArgGPRs[] = {CSKY::R0, CSKY::R1};
  Register Reg = State.AllocateReg(ArgGPRs);
  LocVT = MVT::i32;
  if (!Reg)
    return false;

  if (!State.AllocateReg(ArgGPRs))
    return false;

  State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
  return true;
}

} // namespace vm::core

#endif
