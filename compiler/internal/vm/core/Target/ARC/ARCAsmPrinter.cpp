//===- ARCAsmPrinter.cpp - ARC LLVM assembly writer -------------*- C++ -*-===//
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
// This file contains a printer that converts from our internal representation
// of machine-dependent LLVM code to GNU format ARC assembly language.
//
//===----------------------------------------------------------------------===//

#include "ARC.h"
#include "ARCMCInstLower.h"
#include "ARCSubtarget.h"
#include "ARCTargetMachine.h"
#include "MCTargetDesc/ARCInstPrinter.h"
#include "TargetInfo/ARCTargetInfo.h"
#include "vm/core/CodeGen/AsmPrinter.h"
#include "vm/core/CodeGen/MachineInstr.h"
#include "vm/core/MC/MCAsmInfo.h"
#include "vm/core/MC/MCInst.h"
#include "vm/core/MC/MCStreamer.h"
#include "vm/core/MC/TargetRegistry.h"
#include "vm/core/Support/raw_ostream.h"

using namespace vm::core;

#define DEBUG_TYPE "asm-printer"

namespace {

class ARCAsmPrinter : public AsmPrinter {
  ARCMCInstLower MCInstLowering;

public:
  static char ID;

  explicit ARCAsmPrinter(TargetMachine &TM,
                         std::unique_ptr<MCStreamer> Streamer)
      : AsmPrinter(TM, std::move(Streamer), ID),
        MCInstLowering(&OutContext, *this) {}

  StringRef getPassName() const override { return "ARC Assembly Printer"; }
  void emitInstruction(const MachineInstr *MI) override;

  bool runOnMachineFunction(MachineFunction &MF) override;
};

} // end anonymous namespace

void ARCAsmPrinter::emitInstruction(const MachineInstr *MI) {
  ARC_MC::verifyInstructionPredicates(MI->getOpcode(),
                                      getSubtargetInfo().getFeatureBits());

  SmallString<128> Str;
  raw_svector_ostream O(Str);

  switch (MI->getOpcode()) {
  case ARC::DBG_VALUE:
    llvm_unreachable("Should be handled target independently");
    break;
  }

  MCInst TmpInst;
  MCInstLowering.Lower(MI, TmpInst);
  EmitToStreamer(*OutStreamer, TmpInst);
}

bool ARCAsmPrinter::runOnMachineFunction(MachineFunction &MF) {
  // Functions are 4-byte aligned.
  MF.ensureAlignment(Align(4));
  return AsmPrinter::runOnMachineFunction(MF);
}

char ARCAsmPrinter::ID = 0;

INITIALIZE_PASS(ARCAsmPrinter, "arc-asm-printer", "ARC Assmebly Printer", false,
                false)

// Force static initialization.
extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeARCAsmPrinter() {
  RegisterAsmPrinter<ARCAsmPrinter> X(getTheARCTarget());
}
