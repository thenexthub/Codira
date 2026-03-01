//===-- toolchain/CodeGen/MachineModuleInfo.cpp ----------------------*- C++ -*-===//
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

#include "vm/core/CodeGen/MachineModuleSlotTracker.h"
#include "vm/core/CodeGen/MachineFunction.h"
#include "vm/core/CodeGen/MachineModuleInfo.h"
#include "vm/core/IR/Module.h"

using namespace vm::core;

void MachineModuleSlotTracker::processMachineFunctionMetadata(
    AbstractSlotTrackerStorage *AST, const MachineFunction &MF) {
  // Create metadata created within the backend.
  for (const MachineBasicBlock &MBB : MF)
    for (const MachineInstr &MI : MBB.instrs())
      for (const MachineMemOperand *MMO : MI.memoperands()) {
        AAMDNodes AAInfo = MMO->getAAInfo();
        if (AAInfo.TBAA)
          AST->createMetadataSlot(AAInfo.TBAA);
        if (AAInfo.TBAAStruct)
          AST->createMetadataSlot(AAInfo.TBAAStruct);
        if (AAInfo.Scope)
          AST->createMetadataSlot(AAInfo.Scope);
        if (AAInfo.NoAlias)
          AST->createMetadataSlot(AAInfo.NoAlias);
      }
}

void MachineModuleSlotTracker::processMachineModule(
    AbstractSlotTrackerStorage *AST, const Module *M,
    bool ShouldInitializeAllMetadata) {
  if (ShouldInitializeAllMetadata) {
    for (const Function &F : *M) {
      if (&F != &TheFunction)
        continue;
      MDNStartSlot = AST->getNextMetadataSlot();
      if (TheMF)
        processMachineFunctionMetadata(AST, *TheMF);
      MDNEndSlot = AST->getNextMetadataSlot();
      break;
    }
  }
}

void MachineModuleSlotTracker::processMachineFunction(
    AbstractSlotTrackerStorage *AST, const Function *F,
    bool ShouldInitializeAllMetadata) {
  if (!ShouldInitializeAllMetadata && F == &TheFunction) {
    MDNStartSlot = AST->getNextMetadataSlot();
    if (TheMF)
      processMachineFunctionMetadata(AST, *TheMF);
    MDNEndSlot = AST->getNextMetadataSlot();
  }
}

void MachineModuleSlotTracker::collectMachineMDNodes(
    MachineMDNodeListType &L) const {
  collectMDNodes(L, MDNStartSlot, MDNEndSlot);
}

MachineModuleSlotTracker::MachineModuleSlotTracker(
    MFGetterFnT Fn, const MachineFunction *MF, bool ShouldInitializeAllMetadata)
    : ModuleSlotTracker(MF->getFunction().getParent(),
                        ShouldInitializeAllMetadata),
      TheFunction(MF->getFunction()), TheMF(Fn(MF->getFunction())) {
  setProcessHook([this](AbstractSlotTrackerStorage *AST, const Module *M,
                        bool ShouldInitializeAllMetadata) {
    this->processMachineModule(AST, M, ShouldInitializeAllMetadata);
  });
  setProcessHook([this](AbstractSlotTrackerStorage *AST, const Function *F,
                        bool ShouldInitializeAllMetadata) {
    this->processMachineFunction(AST, F, ShouldInitializeAllMetadata);
  });
}

MachineModuleSlotTracker::~MachineModuleSlotTracker() = default;
