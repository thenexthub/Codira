//===-- XCoreAsmPrinter.cpp - XCore LLVM assembly writer ------------------===//
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
// of machine-dependent LLVM code to the XAS-format XCore assembly language.
//
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/XCoreInstPrinter.h"
#include "MCTargetDesc/XCoreTargetStreamer.h"
#include "TargetInfo/XCoreTargetInfo.h"
#include "XCore.h"
#include "XCoreMCInstLower.h"
#include "XCoreSubtarget.h"
#include "XCoreTargetMachine.h"
#include "vm/core/ADT/SmallString.h"
#include "vm/core/ADT/StringExtras.h"
#include "vm/core/CodeGen/AsmPrinter.h"
#include "vm/core/CodeGen/MachineConstantPool.h"
#include "vm/core/CodeGen/MachineInstr.h"
#include "vm/core/CodeGen/MachineJumpTableInfo.h"
#include "vm/core/CodeGen/MachineModuleInfo.h"
#include "vm/core/IR/DataLayout.h"
#include "vm/core/IR/DerivedTypes.h"
#include "vm/core/IR/Mangler.h"
#include "vm/core/MC/MCAsmInfo.h"
#include "vm/core/MC/MCExpr.h"
#include "vm/core/MC/MCInst.h"
#include "vm/core/MC/MCStreamer.h"
#include "vm/core/MC/TargetRegistry.h"
#include "vm/core/Support/Compiler.h"
#include "vm/core/Support/ErrorHandling.h"
#include "vm/core/Support/raw_ostream.h"
#include "vm/core/Target/TargetLoweringObjectFile.h"
#include <algorithm>
#include <cctype>
using namespace vm::core;

#define DEBUG_TYPE "asm-printer"

namespace {
  class XCoreAsmPrinter : public AsmPrinter {
    XCoreMCInstLower MCInstLowering;
    XCoreTargetStreamer &getTargetStreamer();

  public:
    static char ID;

    explicit XCoreAsmPrinter(TargetMachine &TM,
                             std::unique_ptr<MCStreamer> Streamer)
        : AsmPrinter(TM, std::move(Streamer), ID), MCInstLowering(*this) {}

    StringRef getPassName() const override { return "XCore Assembly Printer"; }

    void printInlineJT(const MachineInstr *MI, int opNum, raw_ostream &O,
                       const std::string &directive = ".jmptable");
    void printInlineJT32(const MachineInstr *MI, int opNum, raw_ostream &O) {
      printInlineJT(MI, opNum, O, ".jmptable32");
    }
    void printOperand(const MachineInstr *MI, int opNum, raw_ostream &O);
    bool PrintAsmOperand(const MachineInstr *MI, unsigned OpNo,
                         const char *ExtraCode, raw_ostream &O) override;
    bool PrintAsmMemoryOperand(const MachineInstr *MI, unsigned OpNum,
                               const char *ExtraCode, raw_ostream &O) override;

    void emitArrayBound(MCSymbol *Sym, const GlobalVariable *GV);
    void emitGlobalVariable(const GlobalVariable *GV) override;

    void emitFunctionEntryLabel() override;
    void emitInstruction(const MachineInstr *MI) override;
    void emitFunctionBodyStart() override;
    void emitFunctionBodyEnd() override;
  };
} // end of anonymous namespace

XCoreTargetStreamer &XCoreAsmPrinter::getTargetStreamer() {
  return static_cast<XCoreTargetStreamer&>(*OutStreamer->getTargetStreamer());
}

void XCoreAsmPrinter::emitArrayBound(MCSymbol *Sym, const GlobalVariable *GV) {
  assert( ( GV->hasExternalLinkage() || GV->hasWeakLinkage() ||
            GV->hasLinkOnceLinkage() || GV->hasCommonLinkage() ) &&
          "Unexpected linkage");
  if (ArrayType *ATy = dyn_cast<ArrayType>(GV->getValueType())) {

    MCSymbol *SymGlob = OutContext.getOrCreateSymbol(
                          Twine(Sym->getName() + StringRef(".globound")));
    OutStreamer->emitSymbolAttribute(SymGlob, MCSA_Global);
    OutStreamer->emitAssignment(SymGlob,
                                MCConstantExpr::create(ATy->getNumElements(),
                                                       OutContext));
    if (GV->hasWeakLinkage() || GV->hasLinkOnceLinkage() ||
        GV->hasCommonLinkage()) {
      OutStreamer->emitSymbolAttribute(SymGlob, MCSA_Weak);
    }
  }
}

void XCoreAsmPrinter::emitGlobalVariable(const GlobalVariable *GV) {
  // Check to see if this is a special global used by LLVM, if so, emit it.
  if (!GV->hasInitializer() || emitSpecialLLVMGlobal(GV))
    return;

  const DataLayout &DL = getDataLayout();
  OutStreamer->switchSection(getObjFileLowering().SectionForGlobal(GV, TM));

  MCSymbol *GVSym = getSymbol(GV);
  const Constant *C = GV->getInitializer();
  const Align Alignment = DL.getPrefTypeAlign(C->getType());

  // Mark the start of the global
  getTargetStreamer().emitCCTopData(GVSym->getName());

  switch (GV->getLinkage()) {
  case GlobalValue::AppendingLinkage:
    report_fatal_error("AppendingLinkage is not supported by this target!");
  case GlobalValue::LinkOnceAnyLinkage:
  case GlobalValue::LinkOnceODRLinkage:
  case GlobalValue::WeakAnyLinkage:
  case GlobalValue::WeakODRLinkage:
  case GlobalValue::ExternalLinkage:
  case GlobalValue::CommonLinkage:
    emitArrayBound(GVSym, GV);
    OutStreamer->emitSymbolAttribute(GVSym, MCSA_Global);

    if (GV->hasWeakLinkage() || GV->hasLinkOnceLinkage() ||
        GV->hasCommonLinkage())
      OutStreamer->emitSymbolAttribute(GVSym, MCSA_Weak);
    [[fallthrough]];
  case GlobalValue::InternalLinkage:
  case GlobalValue::PrivateLinkage:
    break;
  default:
    llvm_unreachable("Unknown linkage type!");
  }

  emitAlignment(std::max(Alignment, Align(4)), GV);

  if (GV->isThreadLocal()) {
    report_fatal_error("TLS is not supported by this target!");
  }
  unsigned Size = DL.getTypeAllocSize(C->getType());
  if (MAI->hasDotTypeDotSizeDirective()) {
    OutStreamer->emitSymbolAttribute(GVSym, MCSA_ELF_TypeObject);
    OutStreamer->emitELFSize(GVSym, MCConstantExpr::create(Size, OutContext));
  }
  OutStreamer->emitLabel(GVSym);

  emitGlobalConstant(DL, C);
  // The ABI requires that unsigned scalar types smaller than 32 bits
  // are padded to 32 bits.
  if (Size < 4)
    OutStreamer->emitZeros(4 - Size);

  // Mark the end of the global
  getTargetStreamer().emitCCBottomData(GVSym->getName());
}

void XCoreAsmPrinter::emitFunctionBodyStart() {
  MCInstLowering.Initialize(&MF->getContext());
}

/// EmitFunctionBodyEnd - Targets can override this to emit stuff after
/// the last basic block in the function.
void XCoreAsmPrinter::emitFunctionBodyEnd() {
  // Emit function end directives
  getTargetStreamer().emitCCBottomFunction(CurrentFnSym->getName());
}

void XCoreAsmPrinter::emitFunctionEntryLabel() {
  // Mark the start of the function
  getTargetStreamer().emitCCTopFunction(CurrentFnSym->getName());
  OutStreamer->emitLabel(CurrentFnSym);
}

void XCoreAsmPrinter::
printInlineJT(const MachineInstr *MI, int opNum, raw_ostream &O,
              const std::string &directive) {
  unsigned JTI = MI->getOperand(opNum).getIndex();
  const MachineFunction *MF = MI->getParent()->getParent();
  const MachineJumpTableInfo *MJTI = MF->getJumpTableInfo();
  const std::vector<MachineJumpTableEntry> &JT = MJTI->getJumpTables();
  const std::vector<MachineBasicBlock*> &JTBBs = JT[JTI].MBBs;
  O << "\t" << directive << " ";
  for (unsigned i = 0, e = JTBBs.size(); i != e; ++i) {
    MachineBasicBlock *MBB = JTBBs[i];
    if (i > 0)
      O << ",";
    MBB->getSymbol()->print(O, MAI);
  }
}

void XCoreAsmPrinter::printOperand(const MachineInstr *MI, int opNum,
                                   raw_ostream &O) {
  const DataLayout &DL = getDataLayout();
  const MachineOperand &MO = MI->getOperand(opNum);
  switch (MO.getType()) {
  case MachineOperand::MO_Register:
    O << XCoreInstPrinter::getRegisterName(MO.getReg());
    break;
  case MachineOperand::MO_Immediate:
    O << MO.getImm();
    break;
  case MachineOperand::MO_MachineBasicBlock:
    MO.getMBB()->getSymbol()->print(O, MAI);
    break;
  case MachineOperand::MO_GlobalAddress:
    PrintSymbolOperand(MO, O);
    break;
  case MachineOperand::MO_ConstantPoolIndex:
    O << DL.getPrivateGlobalPrefix() << "CPI" << getFunctionNumber() << '_'
      << MO.getIndex();
    break;
  case MachineOperand::MO_BlockAddress:
    GetBlockAddressSymbol(MO.getBlockAddress())->print(O, MAI);
    break;
  default:
    llvm_unreachable("not implemented");
  }
}

/// PrintAsmOperand - Print out an operand for an inline asm expression.
///
bool XCoreAsmPrinter::PrintAsmOperand(const MachineInstr *MI, unsigned OpNo,
                                      const char *ExtraCode, raw_ostream &O) {
  // Print the operand if there is no operand modifier.
  if (!ExtraCode || !ExtraCode[0]) {
    printOperand(MI, OpNo, O);
    return false;
  }

  // Otherwise fallback on the default implementation.
  return AsmPrinter::PrintAsmOperand(MI, OpNo, ExtraCode, O);
}

bool XCoreAsmPrinter::PrintAsmMemoryOperand(const MachineInstr *MI,
                                            unsigned OpNum,
                                            const char *ExtraCode,
                                            raw_ostream &O) {
  if (ExtraCode && ExtraCode[0]) {
    return true; // Unknown modifier.
  }
  printOperand(MI, OpNum, O);
  O << '[';
  printOperand(MI, OpNum + 1, O);
  O << ']';
  return false;
}

void XCoreAsmPrinter::emitInstruction(const MachineInstr *MI) {
  XCore_MC::verifyInstructionPredicates(MI->getOpcode(),
                                        getSubtargetInfo().getFeatureBits());

  SmallString<128> Str;
  raw_svector_ostream O(Str);

  switch (MI->getOpcode()) {
  case XCore::DBG_VALUE:
    llvm_unreachable("Should be handled target independently");
  case XCore::ADD_2rus:
    if (MI->getOperand(2).getImm() == 0) {
      O << "\tmov "
        << XCoreInstPrinter::getRegisterName(MI->getOperand(0).getReg()) << ", "
        << XCoreInstPrinter::getRegisterName(MI->getOperand(1).getReg());
      OutStreamer->emitRawText(O.str());
      return;
    }
    break;
  case XCore::BR_JT:
  case XCore::BR_JT32:
    O << "\tbru "
      << XCoreInstPrinter::getRegisterName(MI->getOperand(1).getReg()) << '\n';
    if (MI->getOpcode() == XCore::BR_JT)
      printInlineJT(MI, 0, O);
    else
      printInlineJT32(MI, 0, O);
    O << '\n';
    OutStreamer->emitRawText(O.str());
    return;
  }

  MCInst TmpInst;
  MCInstLowering.Lower(MI, TmpInst);

  EmitToStreamer(*OutStreamer, TmpInst);
}

char XCoreAsmPrinter::ID = 0;

INITIALIZE_PASS(XCoreAsmPrinter, "xcore-asm-printer", "XCore Assembly Printer",
                false, false)

// Force static initialization.
extern "C" LLVM_ABI LLVM_EXTERNAL_VISIBILITY void
LLVMInitializeXCoreAsmPrinter() {
  RegisterAsmPrinter<XCoreAsmPrinter> X(getTheXCoreTarget());
}
