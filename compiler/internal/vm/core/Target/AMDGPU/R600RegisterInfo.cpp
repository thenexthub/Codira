//===-- R600RegisterInfo.cpp - R600 Register Information ------------------===//
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
/// R600 implementation of the TargetRegisterInfo class.
//
//===----------------------------------------------------------------------===//

#include "R600RegisterInfo.h"
#include "MCTargetDesc/R600MCTargetDesc.h"
#include "R600Defines.h"
#include "R600Subtarget.h"

using namespace vm::core;

#define GET_REGINFO_TARGET_DESC
#include "R600GenRegisterInfo.inc"

unsigned R600RegisterInfo::getSubRegFromChannel(unsigned Channel) {
  static const uint16_t SubRegFromChannelTable[] = {
    R600::sub0, R600::sub1, R600::sub2, R600::sub3,
    R600::sub4, R600::sub5, R600::sub6, R600::sub7,
    R600::sub8, R600::sub9, R600::sub10, R600::sub11,
    R600::sub12, R600::sub13, R600::sub14, R600::sub15
  };

  assert(Channel < std::size(SubRegFromChannelTable));
  return SubRegFromChannelTable[Channel];
}

BitVector R600RegisterInfo::getReservedRegs(const MachineFunction &MF) const {
  BitVector Reserved(getNumRegs());

  const R600Subtarget &ST = MF.getSubtarget<R600Subtarget>();
  const R600InstrInfo *TII = ST.getInstrInfo();

  reserveRegisterTuples(Reserved, R600::ZERO);
  reserveRegisterTuples(Reserved, R600::HALF);
  reserveRegisterTuples(Reserved, R600::ONE);
  reserveRegisterTuples(Reserved, R600::ONE_INT);
  reserveRegisterTuples(Reserved, R600::NEG_HALF);
  reserveRegisterTuples(Reserved, R600::NEG_ONE);
  reserveRegisterTuples(Reserved, R600::PV_X);
  reserveRegisterTuples(Reserved, R600::ALU_LITERAL_X);
  reserveRegisterTuples(Reserved, R600::ALU_CONST);
  reserveRegisterTuples(Reserved, R600::PREDICATE_BIT);
  reserveRegisterTuples(Reserved, R600::PRED_SEL_OFF);
  reserveRegisterTuples(Reserved, R600::PRED_SEL_ZERO);
  reserveRegisterTuples(Reserved, R600::PRED_SEL_ONE);
  reserveRegisterTuples(Reserved, R600::INDIRECT_BASE_ADDR);

  for (MCPhysReg R : R600::R600_AddrRegClass)
    reserveRegisterTuples(Reserved, R);

  TII->reserveIndirectRegisters(Reserved, MF, *this);

  return Reserved;
}

// Dummy to not crash RegisterClassInfo.
static const MCPhysReg CalleeSavedReg = R600::NoRegister;

const MCPhysReg *R600RegisterInfo::getCalleeSavedRegs(
  const MachineFunction *) const {
  return &CalleeSavedReg;
}

Register R600RegisterInfo::getFrameRegister(const MachineFunction &MF) const {
  return R600::NoRegister;
}

unsigned R600RegisterInfo::getHWRegChan(unsigned reg) const {
  return this->getEncodingValue(reg) >> HW_CHAN_SHIFT;
}

unsigned R600RegisterInfo::getHWRegIndex(unsigned Reg) const {
  return GET_REG_INDEX(getEncodingValue(Reg));
}

const TargetRegisterClass * R600RegisterInfo::getCFGStructurizerRegClass(
                                                                   MVT VT) const {
  switch(VT.SimpleTy) {
  default:
  case MVT::i32: return &R600::R600_TReg32RegClass;
  }
}

bool R600RegisterInfo::isPhysRegLiveAcrossClauses(Register Reg) const {
  assert(!Reg.isVirtual());

  switch (Reg) {
  case R600::OQAP:
  case R600::OQBP:
  case R600::AR_X:
    return false;
  default:
    return true;
  }
}

bool R600RegisterInfo::eliminateFrameIndex(MachineBasicBlock::iterator MI,
                                           int SPAdj,
                                           unsigned FIOperandNum,
                                           RegScavenger *RS) const {
  llvm_unreachable("Subroutines not supported yet");
}

void R600RegisterInfo::reserveRegisterTuples(BitVector &Reserved, unsigned Reg) const {
  MCRegAliasIterator R(Reg, this, true);

  for (; R.isValid(); ++R)
    Reserved.set(*R);
}
