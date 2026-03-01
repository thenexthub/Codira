//===-- AMDGPUInstrInfo.cpp - Base class for AMD GPU InstrInfo ------------===//
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
/// \file
/// \brief Implementation of the TargetInstrInfo class that is common to all
/// AMD GPUs.
//
//===----------------------------------------------------------------------===//

#include "AMDGPUInstrInfo.h"
#include "AMDGPU.h"
#include "vm/core/CodeGen/MachineInstr.h"
#include "vm/core/CodeGen/MachineMemOperand.h"
#include "vm/core/IR/Constants.h"
#include "vm/core/IR/Instruction.h"
#include "vm/core/IR/Value.h"

using namespace vm::core;

Intrinsic::ID AMDGPU::getIntrinsicID(const MachineInstr &I) {
  return I.getOperand(I.getNumExplicitDefs()).getIntrinsicID();
}

// TODO: Should largely merge with AMDGPUTTIImpl::isSourceOfDivergence.
bool AMDGPU::isUniformMMO(const MachineMemOperand *MMO) {
  const Value *Ptr = MMO->getValue();
  if (!Ptr) {
    if (const PseudoSourceValue *PSV = MMO->getPseudoValue()) {
      return PSV->isConstantPool() || PSV->isStack() || PSV->isGOT() ||
             PSV->isJumpTable();
    }

    // Unknown value.
    return false;
  }

  // UndefValue means this is a load of a kernel input.  These are uniform.
  // Sometimes LDS instructions have constant pointers.
  if (isa<UndefValue, Constant, GlobalValue>(Ptr))
    return true;

  if (MMO->getAddrSpace() == AMDGPUAS::CONSTANT_ADDRESS_32BIT)
    return true;

  if (const Argument *Arg = dyn_cast<Argument>(Ptr))
    return AMDGPU::isArgPassedInSGPR(Arg);

  const Instruction *I = dyn_cast<Instruction>(Ptr);
  return I && I->getMetadata("amdgpu.uniform");
}
