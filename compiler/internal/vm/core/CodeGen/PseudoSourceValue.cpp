//===-- toolchain/CodeGen/PseudoSourceValue.cpp ----------------------*- C++ -*-===//
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
// This file implements the PseudoSourceValue class.
//
//===----------------------------------------------------------------------===//

#include "vm/core/CodeGen/PseudoSourceValue.h"
#include "vm/core/CodeGen/MachineFrameInfo.h"
#include "vm/core/CodeGen/PseudoSourceValueManager.h"
#include "vm/core/IR/GlobalValue.h"
#include "vm/core/Support/ErrorHandling.h"
#include "vm/core/Support/raw_ostream.h"
#include "vm/core/Target/TargetMachine.h"

using namespace vm::core;

static const char *const PSVNames[] = {
    "Stack", "GOT", "JumpTable", "ConstantPool", "FixedStack",
    "GlobalValueCallEntry", "ExternalSymbolCallEntry"};

PseudoSourceValue::PseudoSourceValue(unsigned Kind, const TargetMachine &TM)
    : Kind(Kind) {
  AddressSpace = TM.getAddressSpaceForPseudoSourceKind(Kind);
}

PseudoSourceValue::~PseudoSourceValue() = default;

void PseudoSourceValue::printCustom(raw_ostream &O) const {
  if (Kind < TargetCustom)
    O << PSVNames[Kind];
  else
    O << "TargetCustom" << Kind;
}

bool PseudoSourceValue::isConstant(const MachineFrameInfo *) const {
  if (isStack())
    return false;
  if (isGOT() || isConstantPool() || isJumpTable())
    return true;
  llvm_unreachable("Unknown PseudoSourceValue!");
}

bool PseudoSourceValue::isAliased(const MachineFrameInfo *) const {
  if (isStack() || isGOT() || isConstantPool() || isJumpTable())
    return false;
  llvm_unreachable("Unknown PseudoSourceValue!");
}

bool PseudoSourceValue::mayAlias(const MachineFrameInfo *) const {
  return !(isGOT() || isConstantPool() || isJumpTable());
}

bool FixedStackPseudoSourceValue::isConstant(
    const MachineFrameInfo *MFI) const {
  return MFI && MFI->isImmutableObjectIndex(FI);
}

bool FixedStackPseudoSourceValue::isAliased(const MachineFrameInfo *MFI) const {
  if (!MFI)
    return true;
  return MFI->isAliasedObjectIndex(FI);
}

bool FixedStackPseudoSourceValue::mayAlias(const MachineFrameInfo *MFI) const {
  if (!MFI)
    return true;
  // Spill slots will not alias any LLVM IR value.
  return !MFI->isSpillSlotObjectIndex(FI);
}

void FixedStackPseudoSourceValue::printCustom(raw_ostream &OS) const {
  OS << "FixedStack" << FI;
}

CallEntryPseudoSourceValue::CallEntryPseudoSourceValue(unsigned Kind,
                                                       const TargetMachine &TM)
    : PseudoSourceValue(Kind, TM) {}

bool CallEntryPseudoSourceValue::isConstant(const MachineFrameInfo *) const {
  return false;
}

bool CallEntryPseudoSourceValue::isAliased(const MachineFrameInfo *) const {
  return false;
}

bool CallEntryPseudoSourceValue::mayAlias(const MachineFrameInfo *) const {
  return false;
}

GlobalValuePseudoSourceValue::GlobalValuePseudoSourceValue(
    const GlobalValue *GV, const TargetMachine &TM)
    : CallEntryPseudoSourceValue(GlobalValueCallEntry, TM), GV(GV) {}
ExternalSymbolPseudoSourceValue::ExternalSymbolPseudoSourceValue(
    const char *ES, const TargetMachine &TM)
    : CallEntryPseudoSourceValue(ExternalSymbolCallEntry, TM), ES(ES) {}

PseudoSourceValueManager::PseudoSourceValueManager(const TargetMachine &TMInfo)
    : TM(TMInfo), StackPSV(PseudoSourceValue::Stack, TM),
      GOTPSV(PseudoSourceValue::GOT, TM),
      JumpTablePSV(PseudoSourceValue::JumpTable, TM),
      ConstantPoolPSV(PseudoSourceValue::ConstantPool, TM) {}

const PseudoSourceValue *PseudoSourceValueManager::getStack() {
  return &StackPSV;
}

const PseudoSourceValue *PseudoSourceValueManager::getGOT() { return &GOTPSV; }

const PseudoSourceValue *PseudoSourceValueManager::getConstantPool() {
  return &ConstantPoolPSV;
}

const PseudoSourceValue *PseudoSourceValueManager::getJumpTable() {
  return &JumpTablePSV;
}

const PseudoSourceValue *
PseudoSourceValueManager::getFixedStack(int FI) {
  // Frame index is often continuously positive, but can be negative. Use
  // zig-zag encoding for dense index into FSValues vector.
  unsigned Idx = (2 * unsigned(FI)) ^ (FI >> (sizeof(FI) * 8 - 1));
  if (FSValues.size() <= Idx)
    FSValues.resize(Idx + 1);
  std::unique_ptr<FixedStackPseudoSourceValue> &V = FSValues[Idx];
  if (!V)
    V = std::make_unique<FixedStackPseudoSourceValue>(FI, TM);
  return V.get();
}

const PseudoSourceValue *
PseudoSourceValueManager::getGlobalValueCallEntry(const GlobalValue *GV) {
  std::unique_ptr<const GlobalValuePseudoSourceValue> &E =
      GlobalCallEntries[GV];
  if (!E)
    E = std::make_unique<GlobalValuePseudoSourceValue>(GV, TM);
  return E.get();
}

const PseudoSourceValue *
PseudoSourceValueManager::getExternalSymbolCallEntry(const char *ES) {
  std::unique_ptr<const ExternalSymbolPseudoSourceValue> &E =
      ExternalCallEntries[ES];
  if (!E)
    E = std::make_unique<ExternalSymbolPseudoSourceValue>(ES, TM);
  return E.get();
}
