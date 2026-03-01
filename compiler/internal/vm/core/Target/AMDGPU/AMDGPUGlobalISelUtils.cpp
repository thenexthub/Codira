//===- AMDGPUGlobalISelUtils.cpp ---------------------------------*- C++ -*-==//
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

#include "AMDGPUGlobalISelUtils.h"
#include "AMDGPURegisterBankInfo.h"
#include "MCTargetDesc/AMDGPUMCTargetDesc.h"
#include "vm/core/ADT/DenseSet.h"
#include "vm/core/CodeGen/GlobalISel/GISelValueTracking.h"
#include "vm/core/CodeGen/GlobalISel/GenericMachineInstrs.h"
#include "vm/core/CodeGen/GlobalISel/MIPatternMatch.h"
#include "vm/core/CodeGen/GlobalISel/MachineIRBuilder.h"
#include "vm/core/CodeGenTypes/LowLevelType.h"
#include "vm/core/IR/Constants.h"
#include "vm/core/IR/IntrinsicsAMDGPU.h"

using namespace vm::core;
using namespace AMDGPU;
using namespace MIPatternMatch;

std::pair<Register, unsigned>
AMDGPU::getBaseWithConstantOffset(MachineRegisterInfo &MRI, Register Reg,
                                  GISelValueTracking *ValueTracking,
                                  bool CheckNUW) {
  MachineInstr *Def = getDefIgnoringCopies(Reg, MRI);
  if (Def->getOpcode() == TargetOpcode::G_CONSTANT) {
    unsigned Offset;
    const MachineOperand &Op = Def->getOperand(1);
    if (Op.isImm())
      Offset = Op.getImm();
    else
      Offset = Op.getCImm()->getZExtValue();

    return std::pair(Register(), Offset);
  }

  int64_t Offset;
  if (Def->getOpcode() == TargetOpcode::G_ADD) {
    // A 32-bit (address + offset) should not cause unsigned 32-bit integer
    // wraparound, because s_load instructions perform the addition in 64 bits.
    if (CheckNUW && !Def->getFlag(MachineInstr::NoUWrap)) {
      assert(MRI.getType(Reg).getScalarSizeInBits() == 32);
      return std::pair(Reg, 0);
    }
    // TODO: Handle G_OR used for add case
    if (mi_match(Def->getOperand(2).getReg(), MRI, m_ICst(Offset)))
      return std::pair(Def->getOperand(1).getReg(), Offset);

    // FIXME: matcher should ignore copies
    if (mi_match(Def->getOperand(2).getReg(), MRI, m_Copy(m_ICst(Offset))))
      return std::pair(Def->getOperand(1).getReg(), Offset);
  }

  Register Base;
  if (ValueTracking && mi_match(Reg, MRI, m_GOr(m_Reg(Base), m_ICst(Offset))) &&
      ValueTracking->maskedValueIsZero(Base,
                                       APInt(32, Offset, /*isSigned=*/true)))
    return std::pair(Base, Offset);

  // Handle G_PTRTOINT (G_PTR_ADD base, const) case
  if (Def->getOpcode() == TargetOpcode::G_PTRTOINT) {
    MachineInstr *Base;
    if (mi_match(Def->getOperand(1).getReg(), MRI,
                 m_GPtrAdd(m_MInstr(Base), m_ICst(Offset)))) {
      // If Base was int converted to pointer, simply return int and offset.
      if (Base->getOpcode() == TargetOpcode::G_INTTOPTR)
        return std::pair(Base->getOperand(1).getReg(), Offset);

      // Register returned here will be of pointer type.
      return std::pair(Base->getOperand(0).getReg(), Offset);
    }
  }

  return std::pair(Reg, 0);
}

IntrinsicLaneMaskAnalyzer::IntrinsicLaneMaskAnalyzer(MachineFunction &MF)
    : MRI(MF.getRegInfo()) {
  initLaneMaskIntrinsics(MF);
}

bool IntrinsicLaneMaskAnalyzer::isS32S64LaneMask(Register Reg) const {
  return S32S64LaneMask.contains(Reg);
}

void IntrinsicLaneMaskAnalyzer::initLaneMaskIntrinsics(MachineFunction &MF) {
  for (auto &MBB : MF) {
    for (auto &MI : MBB) {
      GIntrinsic *GI = dyn_cast<GIntrinsic>(&MI);
      if (GI && GI->is(Intrinsic::amdgcn_if_break)) {
        S32S64LaneMask.insert(MI.getOperand(3).getReg());
        S32S64LaneMask.insert(MI.getOperand(0).getReg());
      }

      if (MI.getOpcode() == AMDGPU::SI_IF ||
          MI.getOpcode() == AMDGPU::SI_ELSE) {
        S32S64LaneMask.insert(MI.getOperand(0).getReg());
      }
    }
  }
}

static LLT getReadAnyLaneSplitTy(LLT Ty) {
  if (Ty.isVector()) {
    LLT ElTy = Ty.getElementType();
    if (ElTy.getSizeInBits() == 16)
      return LLT::fixed_vector(2, ElTy);
    // S32, S64 or pointer
    return ElTy;
  }

  // Large scalars and 64-bit pointers
  return LLT::scalar(32);
}

template <typename ReadLaneFnTy>
static Register buildReadLane(MachineIRBuilder &, Register,
                              const RegisterBankInfo &, ReadLaneFnTy);

template <typename ReadLaneFnTy>
static void
unmergeReadAnyLane(MachineIRBuilder &B, SmallVectorImpl<Register> &SgprDstParts,
                   LLT UnmergeTy, Register VgprSrc, const RegisterBankInfo &RBI,
                   ReadLaneFnTy BuildRL) {
  const RegisterBank *VgprRB = &RBI.getRegBank(AMDGPU::VGPRRegBankID);
  auto Unmerge = B.buildUnmerge({VgprRB, UnmergeTy}, VgprSrc);
  for (unsigned i = 0; i < Unmerge->getNumOperands() - 1; ++i) {
    SgprDstParts.push_back(buildReadLane(B, Unmerge.getReg(i), RBI, BuildRL));
  }
}

template <typename ReadLaneFnTy>
static Register buildReadLane(MachineIRBuilder &B, Register VgprSrc,
                              const RegisterBankInfo &RBI,
                              ReadLaneFnTy BuildRL) {
  LLT Ty = B.getMRI()->getType(VgprSrc);
  const RegisterBank *SgprRB = &RBI.getRegBank(AMDGPU::SGPRRegBankID);
  if (Ty.getSizeInBits() == 32) {
    Register SgprDst = B.getMRI()->createVirtualRegister({SgprRB, Ty});
    return BuildRL(B, SgprDst, VgprSrc).getReg(0);
  }

  SmallVector<Register, 8> SgprDstParts;
  unmergeReadAnyLane(B, SgprDstParts, getReadAnyLaneSplitTy(Ty), VgprSrc, RBI,
                     BuildRL);

  return B.buildMergeLikeInstr({SgprRB, Ty}, SgprDstParts).getReg(0);
}

template <typename ReadLaneFnTy>
static void buildReadLane(MachineIRBuilder &B, Register SgprDst,
                          Register VgprSrc, const RegisterBankInfo &RBI,
                          ReadLaneFnTy BuildReadLane) {
  LLT Ty = B.getMRI()->getType(VgprSrc);
  if (Ty.getSizeInBits() == 32) {
    BuildReadLane(B, SgprDst, VgprSrc);
    return;
  }

  SmallVector<Register, 8> SgprDstParts;
  unmergeReadAnyLane(B, SgprDstParts, getReadAnyLaneSplitTy(Ty), VgprSrc, RBI,
                     BuildReadLane);

  B.buildMergeLikeInstr(SgprDst, SgprDstParts).getReg(0);
}

void AMDGPU::buildReadAnyLane(MachineIRBuilder &B, Register SgprDst,
                              Register VgprSrc, const RegisterBankInfo &RBI) {
  return buildReadLane(
      B, SgprDst, VgprSrc, RBI,
      [](MachineIRBuilder &B, Register SgprDst, Register VgprSrc) {
        return B.buildInstr(AMDGPU::G_AMDGPU_READANYLANE, {SgprDst}, {VgprSrc});
      });
}

void AMDGPU::buildReadFirstLane(MachineIRBuilder &B, Register SgprDst,
                                Register VgprSrc, const RegisterBankInfo &RBI) {
  return buildReadLane(
      B, SgprDst, VgprSrc, RBI,
      [](MachineIRBuilder &B, Register SgprDst, Register VgprSrc) {
        return B.buildIntrinsic(Intrinsic::amdgcn_readfirstlane, SgprDst)
            .addReg(VgprSrc);
      });
}
