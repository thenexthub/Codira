//===-- BPFAsmPrinter.cpp - BPF LLVM assembly writer ----------------------===//
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
// of machine-dependent LLVM code to the BPF assembly language.
//
//===----------------------------------------------------------------------===//

#include "BPFAsmPrinter.h"
#include "BPF.h"
#include "BPFInstrInfo.h"
#include "BPFMCInstLower.h"
#include "BTFDebug.h"
#include "MCTargetDesc/BPFInstPrinter.h"
#include "TargetInfo/BPFTargetInfo.h"
#include "vm/core/BinaryFormat/ELF.h"
#include "vm/core/CodeGen/AsmPrinter.h"
#include "vm/core/CodeGen/MachineConstantPool.h"
#include "vm/core/CodeGen/MachineInstr.h"
#include "vm/core/CodeGen/MachineJumpTableInfo.h"
#include "vm/core/CodeGen/MachineModuleInfo.h"
#include "vm/core/CodeGen/TargetLowering.h"
#include "vm/core/IR/Module.h"
#include "vm/core/MC/MCAsmInfo.h"
#include "vm/core/MC/MCExpr.h"
#include "vm/core/MC/MCInst.h"
#include "vm/core/MC/MCStreamer.h"
#include "vm/core/MC/MCSymbol.h"
#include "vm/core/MC/MCSymbolELF.h"
#include "vm/core/MC/TargetRegistry.h"
#include "vm/core/Support/Compiler.h"
#include "vm/core/Support/raw_ostream.h"
#include "vm/core/Target/TargetLoweringObjectFile.h"
using namespace vm::core;

#define DEBUG_TYPE "asm-printer"

bool BPFAsmPrinter::doInitialization(Module &M) {
  AsmPrinter::doInitialization(M);

  // Only emit BTF when debuginfo available.
  if (MAI->doesSupportDebugInformation() && !M.debug_compile_units().empty()) {
    BTF = new BTFDebug(this);
    Handlers.push_back(std::unique_ptr<BTFDebug>(BTF));
  }

  return false;
}

const BPFTargetMachine &BPFAsmPrinter::getBTM() const {
  return static_cast<const BPFTargetMachine &>(TM);
}

bool BPFAsmPrinter::doFinalization(Module &M) {
  // Remove unused globals which are previously used for jump table.
  const BPFSubtarget *Subtarget = getBTM().getSubtargetImpl();
  if (Subtarget->hasGotox()) {
    std::vector<GlobalVariable *> Targets;
    for (GlobalVariable &Global : M.globals()) {
      if (Global.getLinkage() != GlobalValue::PrivateLinkage)
        continue;
      if (!Global.isConstant() || !Global.hasInitializer())
        continue;

      Constant *CV = dyn_cast<Constant>(Global.getInitializer());
      if (!CV)
        continue;
      ConstantArray *CA = dyn_cast<ConstantArray>(CV);
      if (!CA)
        continue;

      for (unsigned i = 1, e = CA->getNumOperands(); i != e; ++i) {
        if (!dyn_cast<BlockAddress>(CA->getOperand(i)))
          continue;
      }
      Targets.push_back(&Global);
    }

    for (GlobalVariable *GV : Targets) {
      GV->replaceAllUsesWith(PoisonValue::get(GV->getType()));
      GV->dropAllReferences();
      GV->eraseFromParent();
    }
  }

  for (GlobalObject &GO : M.global_objects()) {
    if (!GO.hasExternalWeakLinkage())
      continue;

    if (!SawTrapCall && GO.getName() == BPF_TRAP) {
      GO.eraseFromParent();
      break;
    }
  }

  return AsmPrinter::doFinalization(M);
}

void BPFAsmPrinter::printOperand(const MachineInstr *MI, int OpNum,
                                 raw_ostream &O) {
  const MachineOperand &MO = MI->getOperand(OpNum);

  switch (MO.getType()) {
  case MachineOperand::MO_Register:
    O << BPFInstPrinter::getRegisterName(MO.getReg());
    break;

  case MachineOperand::MO_Immediate:
    O << MO.getImm();
    break;

  case MachineOperand::MO_MachineBasicBlock:
    O << *MO.getMBB()->getSymbol();
    break;

  case MachineOperand::MO_GlobalAddress:
    O << *getSymbol(MO.getGlobal());
    break;

  case MachineOperand::MO_BlockAddress: {
    MCSymbol *BA = GetBlockAddressSymbol(MO.getBlockAddress());
    O << BA->getName();
    break;
  }

  case MachineOperand::MO_ExternalSymbol:
    O << *GetExternalSymbolSymbol(MO.getSymbolName());
    break;

  case MachineOperand::MO_JumpTableIndex:
  case MachineOperand::MO_ConstantPoolIndex:
  default:
    llvm_unreachable("<unknown operand type>");
  }
}

bool BPFAsmPrinter::PrintAsmOperand(const MachineInstr *MI, unsigned OpNo,
                                    const char *ExtraCode, raw_ostream &O) {
  if (ExtraCode && ExtraCode[0])
    return AsmPrinter::PrintAsmOperand(MI, OpNo, ExtraCode, O);

  printOperand(MI, OpNo, O);
  return false;
}

bool BPFAsmPrinter::PrintAsmMemoryOperand(const MachineInstr *MI,
                                          unsigned OpNum, const char *ExtraCode,
                                          raw_ostream &O) {
  assert(OpNum + 1 < MI->getNumOperands() && "Insufficient operands");
  const MachineOperand &BaseMO = MI->getOperand(OpNum);
  const MachineOperand &OffsetMO = MI->getOperand(OpNum + 1);
  assert(BaseMO.isReg() && "Unexpected base pointer for inline asm memory operand.");
  assert(OffsetMO.isImm() && "Unexpected offset for inline asm memory operand.");
  int Offset = OffsetMO.getImm();

  if (ExtraCode)
    return true; // Unknown modifier.

  if (Offset < 0)
    O << "(" << BPFInstPrinter::getRegisterName(BaseMO.getReg()) << " - " << -Offset << ")";
  else
    O << "(" << BPFInstPrinter::getRegisterName(BaseMO.getReg()) << " + " << Offset << ")";

  return false;
}

void BPFAsmPrinter::emitInstruction(const MachineInstr *MI) {
  if (MI->isCall()) {
    for (const MachineOperand &Op : MI->operands()) {
      if (Op.isGlobal()) {
        if (const GlobalValue *GV = Op.getGlobal())
          if (GV->getName() == BPF_TRAP)
            SawTrapCall = true;
      }
    }
  }

  BPF_MC::verifyInstructionPredicates(MI->getOpcode(),
                                      getSubtargetInfo().getFeatureBits());

  MCInst TmpInst;

  if (!BTF || !BTF->InstLower(MI, TmpInst)) {
    BPFMCInstLower MCInstLowering(OutContext, *this);
    MCInstLowering.Lower(MI, TmpInst);
  }
  EmitToStreamer(*OutStreamer, TmpInst);
}

MCSymbol *BPFAsmPrinter::getJTPublicSymbol(unsigned JTI) {
  SmallString<60> Name;
  raw_svector_ostream(Name)
      << "BPF.JT." << MF->getFunctionNumber() << '.' << JTI;
  MCSymbol *S = OutContext.getOrCreateSymbol(Name);
  if (auto *ES = static_cast<MCSymbolELF *>(S)) {
    ES->setBinding(ELF::STB_GLOBAL);
    ES->setType(ELF::STT_OBJECT);
  }
  return S;
}

void BPFAsmPrinter::emitJumpTableInfo() {
  const MachineJumpTableInfo *MJTI = MF->getJumpTableInfo();
  if (!MJTI)
    return;

  const std::vector<MachineJumpTableEntry> &JT = MJTI->getJumpTables();
  if (JT.empty())
    return;

  const TargetLoweringObjectFile &TLOF = getObjFileLowering();
  const Function &F = MF->getFunction();

  MCSection *Sec = OutStreamer->getCurrentSectionOnly();
  MCSymbol *SecStart = Sec->getBeginSymbol();

  MCSection *JTS = TLOF.getSectionForJumpTable(F, TM);
  assert(MJTI->getEntryKind() == MachineJumpTableInfo::EK_BlockAddress);
  unsigned EntrySize = MJTI->getEntrySize(getDataLayout());
  OutStreamer->switchSection(JTS);
  for (unsigned JTI = 0; JTI < JT.size(); JTI++) {
    ArrayRef<MachineBasicBlock *> JTBBs = JT[JTI].MBBs;
    if (JTBBs.empty())
      continue;

    MCSymbol *JTStart = getJTPublicSymbol(JTI);
    OutStreamer->emitLabel(JTStart);
    for (const MachineBasicBlock *MBB : JTBBs) {
      const MCExpr *Diff = MCBinaryExpr::createSub(
          MCSymbolRefExpr::create(MBB->getSymbol(), OutContext),
          MCSymbolRefExpr::create(SecStart, OutContext), OutContext);
      OutStreamer->emitValue(Diff, EntrySize);
    }
    const MCExpr *JTSize =
        MCConstantExpr::create(JTBBs.size() * EntrySize, OutContext);
    OutStreamer->emitELFSize(JTStart, JTSize);
  }
}

char BPFAsmPrinter::ID = 0;

INITIALIZE_PASS(BPFAsmPrinter, "bpf-asm-printer", "BPF Assembly Printer", false,
                false)

// Force static initialization.
extern "C" LLVM_ABI LLVM_EXTERNAL_VISIBILITY void
LLVMInitializeBPFAsmPrinter() {
  RegisterAsmPrinter<BPFAsmPrinter> X(getTheBPFleTarget());
  RegisterAsmPrinter<BPFAsmPrinter> Y(getTheBPFbeTarget());
  RegisterAsmPrinter<BPFAsmPrinter> Z(getTheBPFTarget());
}
