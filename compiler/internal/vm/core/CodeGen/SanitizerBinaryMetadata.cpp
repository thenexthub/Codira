//===- SanitizerBinaryMetadata.cpp
//----------------------------------------------===//
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
// This file is a part of SanitizerBinaryMetadata.
//
//===----------------------------------------------------------------------===//

#include "vm/core/CodeGen/SanitizerBinaryMetadata.h"
#include "vm/core/CodeGen/MachineFrameInfo.h"
#include "vm/core/CodeGen/MachineFunction.h"
#include "vm/core/CodeGen/MachineFunctionPass.h"
#include "vm/core/CodeGen/Passes.h"
#include "vm/core/IR/IRBuilder.h"
#include "vm/core/IR/MDBuilder.h"
#include "vm/core/InitializePasses.h"
#include "vm/core/Pass.h"
#include "vm/core/Support/VirtualFileSystem.h"
#include "vm/core/Transforms/Instrumentation/SanitizerBinaryMetadata.h"
#include <algorithm>

using namespace vm::core;

namespace {
// FIXME: This pass modifies Function metadata, which is not to be done in
// MachineFunctionPass. It should probably be moved to a FunctionPass.
class MachineSanitizerBinaryMetadataLegacy : public MachineFunctionPass {
public:
  static char ID;

  MachineSanitizerBinaryMetadataLegacy();
  bool runOnMachineFunction(MachineFunction &F) override;
};

struct MachineSanitizerBinaryMetadata {
  bool run(MachineFunction &MF);
};

} // namespace

INITIALIZE_PASS(MachineSanitizerBinaryMetadataLegacy, "machine-sanmd",
                "Machine Sanitizer Binary Metadata", false, false)

char MachineSanitizerBinaryMetadataLegacy::ID = 0;
char &toolchain::MachineSanitizerBinaryMetadataID =
    MachineSanitizerBinaryMetadataLegacy::ID;

MachineSanitizerBinaryMetadataLegacy::MachineSanitizerBinaryMetadataLegacy()
    : MachineFunctionPass(ID) {
  initializeMachineSanitizerBinaryMetadataLegacyPass(
      *PassRegistry::getPassRegistry());
}

bool MachineSanitizerBinaryMetadataLegacy::runOnMachineFunction(
    MachineFunction &MF) {
  return MachineSanitizerBinaryMetadata().run(MF);
}

PreservedAnalyses
MachineSanitizerBinaryMetadataPass::run(MachineFunction &MF,
                                        MachineFunctionAnalysisManager &MFAM) {
  if (!MachineSanitizerBinaryMetadata().run(MF))
    return PreservedAnalyses::all();

  return getMachineFunctionPassPreservedAnalyses();
}

bool MachineSanitizerBinaryMetadata::run(MachineFunction &MF) {
  MDNode *MD = MF.getFunction().getMetadata(LLVMContext::MD_pcsections);
  if (!MD)
    return false;
  const auto &Section = *cast<MDString>(MD->getOperand(0));
  if (!Section.getString().starts_with(kSanitizerBinaryMetadataCoveredSection))
    return false;
  auto &AuxMDs = *cast<MDTuple>(MD->getOperand(1));
  // Assume it currently only has features.
  assert(AuxMDs.getNumOperands() == 1);
  Constant *Features =
      cast<ConstantAsMetadata>(AuxMDs.getOperand(0))->getValue();
  if (!Features->getUniqueInteger()[kSanitizerBinaryMetadataUARBit])
    return false;
  // Calculate size of stack args for the function.
  int64_t Size = 0;
  uint64_t Align = 0;
  const MachineFrameInfo &MFI = MF.getFrameInfo();
  for (int i = -1; i >= (int)-MFI.getNumFixedObjects(); --i) {
    Size = std::max(Size, MFI.getObjectOffset(i) + MFI.getObjectSize(i));
    Align = std::max(Align, MFI.getObjectAlign(i).value());
  }
  Size = (Size + Align - 1) & ~(Align - 1);
  if (!Size)
    return false;
  // Non-zero size, update metadata.
  auto &F = MF.getFunction();
  IRBuilder<> IRB(F.getContext());
  MDBuilder MDB(F.getContext());
  // Keep the features and append size of stack args to the metadata.
  APInt NewFeatures = Features->getUniqueInteger();
  NewFeatures.setBit(kSanitizerBinaryMetadataUARHasSizeBit);
  F.setMetadata(
      LLVMContext::MD_pcsections,
      MDB.createPCSections({{Section.getString(),
                             {IRB.getInt(NewFeatures), IRB.getInt32(Size)}}}));
  return false;
}
