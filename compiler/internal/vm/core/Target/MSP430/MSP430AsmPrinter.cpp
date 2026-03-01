//===-- MSP430AsmPrinter.cpp - MSP430 LLVM assembly writer ----------------===//
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
// of machine-dependent LLVM code to the MSP430 assembly language.
//
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/MSP430InstPrinter.h"
#include "MSP430MCInstLower.h"
#include "MSP430TargetMachine.h"
#include "TargetInfo/MSP430TargetInfo.h"
#include "vm/core/BinaryFormat/ELF.h"
#include "vm/core/CodeGen/AsmPrinter.h"
#include "vm/core/CodeGen/MachineConstantPool.h"
#include "vm/core/CodeGen/MachineInstr.h"
#include "vm/core/CodeGen/MachineModuleInfo.h"
#include "vm/core/IR/Mangler.h"
#include "vm/core/MC/MCAsmInfo.h"
#include "vm/core/MC/MCInst.h"
#include "vm/core/MC/MCSectionELF.h"
#include "vm/core/MC/MCStreamer.h"
#include "vm/core/MC/MCSymbol.h"
#include "vm/core/MC/TargetRegistry.h"
#include "vm/core/Support/Compiler.h"
#include "vm/core/Support/raw_ostream.h"
using namespace vm::core;

#define DEBUG_TYPE "asm-printer"

namespace {
  class MSP430AsmPrinter : public AsmPrinter {
  public:
    MSP430AsmPrinter(TargetMachine &TM, std::unique_ptr<MCStreamer> Streamer)
        : AsmPrinter(TM, std::move(Streamer), ID) {}

    StringRef getPassName() const override { return "MSP430 Assembly Printer"; }

    bool runOnMachineFunction(MachineFunction &MF) override;

    void PrintSymbolOperand(const MachineOperand &MO, raw_ostream &O) override;
    void printOperand(const MachineInstr *MI, int OpNum, raw_ostream &O,
                      bool PrefixHash = true);
    void printSrcMemOperand(const MachineInstr *MI, int OpNum,
                            raw_ostream &O);
    bool PrintAsmOperand(const MachineInstr *MI, unsigned OpNo,
                         const char *ExtraCode, raw_ostream &O) override;
    bool PrintAsmMemoryOperand(const MachineInstr *MI, unsigned OpNo,
                               const char *ExtraCode, raw_ostream &O) override;
    void emitInstruction(const MachineInstr *MI) override;

    void EmitInterruptVectorSection(MachineFunction &ISR);

    static char ID;
  };
} // end of anonymous namespace

void MSP430AsmPrinter::PrintSymbolOperand(const MachineOperand &MO,
                                          raw_ostream &O) {
  uint64_t Offset = MO.getOffset();
  if (Offset)
    O << '(' << Offset << '+';

  getSymbol(MO.getGlobal())->print(O, MAI);

  if (Offset)
    O << ')';
}

void MSP430AsmPrinter::printOperand(const MachineInstr *MI, int OpNum,
                                    raw_ostream &O, bool PrefixHash) {
  const MachineOperand &MO = MI->getOperand(OpNum);
  switch (MO.getType()) {
  default: llvm_unreachable("Not implemented yet!");
  case MachineOperand::MO_Register:
    O << MSP430InstPrinter::getRegisterName(MO.getReg());
    return;
  case MachineOperand::MO_Immediate:
    if (PrefixHash)
      O << '#';
    O << MO.getImm();
    return;
  case MachineOperand::MO_MachineBasicBlock:
    MO.getMBB()->getSymbol()->print(O, MAI);
    return;
  case MachineOperand::MO_GlobalAddress: {
    // If the global address expression is a part of displacement field with a
    // register base, we should not emit any prefix symbol here, e.g.
    //   mov.w glb(r1), r2
    // Otherwise (!) msp430-as will silently miscompile the output :(
    if (PrefixHash)
      O << '#';
    PrintSymbolOperand(MO, O);
    return;
  }
  }
}

void MSP430AsmPrinter::printSrcMemOperand(const MachineInstr *MI, int OpNum,
                                          raw_ostream &O) {
  const MachineOperand &Base = MI->getOperand(OpNum);
  const MachineOperand &Disp = MI->getOperand(OpNum+1);

  // Print displacement first

  // Imm here is in fact global address - print extra modifier.
  if (Disp.isImm() && Base.getReg() == MSP430::SR)
    O << '&';
  printOperand(MI, OpNum + 1, O, /*PrefixHash=*/false);

  // Print register base field
  if (Base.getReg() != MSP430::SR && Base.getReg() != MSP430::PC) {
    O << '(';
    printOperand(MI, OpNum, O);
    O << ')';
  }
}

/// PrintAsmOperand - Print out an operand for an inline asm expression.
///
bool MSP430AsmPrinter::PrintAsmOperand(const MachineInstr *MI, unsigned OpNo,
                                       const char *ExtraCode, raw_ostream &O) {
  // Does this asm operand have a single letter operand modifier?
  if (ExtraCode && ExtraCode[0])
    return AsmPrinter::PrintAsmOperand(MI, OpNo, ExtraCode, O);

  printOperand(MI, OpNo, O);
  return false;
}

bool MSP430AsmPrinter::PrintAsmMemoryOperand(const MachineInstr *MI,
                                             unsigned OpNo,
                                             const char *ExtraCode,
                                             raw_ostream &O) {
  if (ExtraCode && ExtraCode[0]) {
    return true; // Unknown modifier.
  }
  printSrcMemOperand(MI, OpNo, O);
  return false;
}

//===----------------------------------------------------------------------===//
void MSP430AsmPrinter::emitInstruction(const MachineInstr *MI) {
  MSP430_MC::verifyInstructionPredicates(MI->getOpcode(),
                                         getSubtargetInfo().getFeatureBits());

  MSP430MCInstLower MCInstLowering(OutContext, *this);

  MCInst TmpInst;
  MCInstLowering.Lower(MI, TmpInst);
  EmitToStreamer(*OutStreamer, TmpInst);
}

void MSP430AsmPrinter::EmitInterruptVectorSection(MachineFunction &ISR) {
  MCSection *Cur = OutStreamer->getCurrentSectionOnly();
  const auto *F = &ISR.getFunction();
  if (F->getCallingConv() != CallingConv::MSP430_INTR) {
    report_fatal_error("Functions with 'interrupt' attribute must have msp430_intrcc CC");
  }
  StringRef IVIdx = F->getFnAttribute("interrupt").getValueAsString();
  MCSection *IV = OutStreamer->getContext().getELFSection(
    "__interrupt_vector_" + IVIdx,
    ELF::SHT_PROGBITS, ELF::SHF_ALLOC | ELF::SHF_EXECINSTR);
  OutStreamer->switchSection(IV);

  const MCSymbol *FunctionSymbol = getSymbol(F);
  OutStreamer->emitSymbolValue(FunctionSymbol, TM.getProgramPointerSize());
  OutStreamer->switchSection(Cur);
}

bool MSP430AsmPrinter::runOnMachineFunction(MachineFunction &MF) {
  // Emit separate section for an interrupt vector if ISR
  if (MF.getFunction().hasFnAttribute("interrupt")) {
    EmitInterruptVectorSection(MF);
  }

  SetupMachineFunction(MF);
  emitFunctionBody();
  return false;
}

char MSP430AsmPrinter::ID = 0;

INITIALIZE_PASS(MSP430AsmPrinter, "msp430-asm-printer",
                "MSP430 Assembly Printer", false, false)

// Force static initialization.
extern "C" LLVM_ABI LLVM_EXTERNAL_VISIBILITY void
LLVMInitializeMSP430AsmPrinter() {
  RegisterAsmPrinter<MSP430AsmPrinter> X(getTheMSP430Target());
}
