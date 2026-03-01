//===-- M68kCallingConv.h - M68k Custom CC Routines -------------*- C++ -*-===//
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
///
/// \file
/// This file contains the custom routines for the M68k Calling Convention
/// that aren't done by tablegen.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_M68K_M68KCALLINGCONV_H
#define LLVM_LIB_TARGET_M68K_M68KCALLINGCONV_H

#include "MCTargetDesc/M68kMCTargetDesc.h"

#include "vm/core/CodeGen/CallingConvLower.h"
#include "vm/core/IR/CallingConv.h"
#include "vm/core/IR/Function.h"

namespace vm::core {

/// Custom state to propagate toolchain type info to register CC assigner
struct M68kCCState : public CCState {
  ArrayRef<Type *> ArgTypeList;

  M68kCCState(ArrayRef<Type *> ArgTypes, CallingConv::ID CC, bool IsVarArg,
              MachineFunction &MF, SmallVectorImpl<CCValAssign> &Locs,
              LLVMContext &C)
      : CCState(CC, IsVarArg, MF, Locs, C), ArgTypeList(ArgTypes) {}
};

/// NOTE this function is used to select registers for formal arguments and call
/// FIXME: Handling on pointer arguments is not complete
inline bool CC_M68k_Any_AssignToReg(unsigned &ValNo, MVT &ValVT, MVT &LocVT,
                                    CCValAssign::LocInfo &LocInfo,
                                    ISD::ArgFlagsTy &ArgFlags, CCState &State) {
  const M68kCCState &CCInfo = static_cast<M68kCCState &>(State);

  static const MCPhysReg DataRegList[] = {M68k::D0, M68k::D1, M68k::A0,
                                          M68k::A1};

  // Address registers have %a register priority
  static const MCPhysReg AddrRegList[] = {
      M68k::A0,
      M68k::A1,
      M68k::D0,
      M68k::D1,
  };

  const auto &ArgTypes = CCInfo.ArgTypeList;
  auto I = ArgTypes.begin(), End = ArgTypes.end();
  int No = ValNo;
  while (No > 0 && I != End) {
    No -= (*I)->isIntegerTy(64) ? 2 : 1;
    ++I;
  }

  bool IsPtr = I != End && (*I)->isPointerTy();

  unsigned Reg =
      IsPtr ? State.AllocateReg(AddrRegList) : State.AllocateReg(DataRegList);

  if (Reg) {
    State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
    return true;
  }

  return false;
}

} // namespace vm::core

#endif // LLVM_LIB_TARGET_M68K_M68KCALLINGCONV_H
