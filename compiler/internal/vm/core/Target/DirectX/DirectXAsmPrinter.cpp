//===-- DirectXAsmPrinter.cpp - DirectX assembly writer --------*- C++ -*--===//
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
// This file contains AsmPrinters for the DirectX backend.
//
//===----------------------------------------------------------------------===//

#include "TargetInfo/DirectXTargetInfo.h"
#include "vm/core/CodeGen/AsmPrinter.h"
#include "vm/core/IR/GlobalVariable.h"
#include "vm/core/IR/Module.h"
#include "vm/core/MC/MCStreamer.h"
#include "vm/core/MC/SectionKind.h"
#include "vm/core/MC/TargetRegistry.h"
#include "vm/core/Target/TargetLoweringObjectFile.h"

using namespace vm::core;

#define DEBUG_TYPE "asm-printer"

namespace {

// The DXILAsmPrinter is mostly a stub because DXIL is just LLVM bitcode which
// gets embedded into a DXContainer file.
class DXILAsmPrinter : public AsmPrinter {
public:
  explicit DXILAsmPrinter(TargetMachine &TM,
                          std::unique_ptr<MCStreamer> Streamer)
      : AsmPrinter(TM, std::move(Streamer)) {}

  StringRef getPassName() const override { return "DXIL Assembly Printer"; }
  void emitGlobalVariable(const GlobalVariable *GV) override;
  bool runOnMachineFunction(MachineFunction &MF) override { return false; }
};
} // namespace

void DXILAsmPrinter::emitGlobalVariable(const GlobalVariable *GV) {
  // If there is no initializer, or no explicit section do nothing
  if (!GV->hasInitializer() || GV->hasImplicitSection() || !GV->hasSection())
    return;
  // Skip the LLVM metadata
  if (GV->getSection() == "toolchain.metadata")
    return;
  SectionKind GVKind = TargetLoweringObjectFile::getKindForGlobal(GV, TM);
  MCSection *TheSection = getObjFileLowering().SectionForGlobal(GV, GVKind, TM);
  OutStreamer->switchSection(TheSection);
  emitGlobalConstant(GV->getDataLayout(), GV->getInitializer());
}

extern "C" LLVM_ABI LLVM_EXTERNAL_VISIBILITY void
LLVMInitializeDirectXAsmPrinter() {
  RegisterAsmPrinter<DXILAsmPrinter> X(getTheDirectXTarget());
}
