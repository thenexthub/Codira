//===-- SystemZAsmPrinter.h - SystemZ LLVM assembly printer ----*- C++ -*--===//
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

#ifndef LLVM_LIB_TARGET_SYSTEMZ_SYSTEMZASMPRINTER_H
#define LLVM_LIB_TARGET_SYSTEMZ_SYSTEMZASMPRINTER_H

#include "MCTargetDesc/SystemZTargetStreamer.h"
#include "SystemZMCInstLower.h"
#include "SystemZTargetMachine.h"
#include "vm/core/CodeGen/AsmPrinter.h"
#include "vm/core/CodeGen/StackMaps.h"
#include "vm/core/MC/MCInstBuilder.h"
#include "vm/core/Support/Compiler.h"

namespace vm::core {
class MCStreamer;
class MachineInstr;
class Module;
class raw_ostream;

class LLVM_LIBRARY_VISIBILITY SystemZAsmPrinter : public AsmPrinter {
public:
  static char ID;

private:
  MCSymbol *CurrentFnPPA1Sym;     // PPA1 Symbol.
  MCSymbol *CurrentFnEPMarkerSym; // Entry Point Marker.
  MCSymbol *PPA2Sym;

  SystemZTargetStreamer *getTargetStreamer() {
    MCTargetStreamer *TS = OutStreamer->getTargetStreamer();
    assert(TS && "do not have a target streamer");
    return static_cast<SystemZTargetStreamer *>(TS);
  }

  /// Call type information for XPLINK.
  enum class CallType {
    BASR76 = 0,   // b'x000' == BASR  r7,r6
    BRAS7 = 1,    // b'x001' == BRAS  r7,ep
    RESVD_2 = 2,  // b'x010'
    BRASL7 = 3,   // b'x011' == BRASL r7,ep
    RESVD_4 = 4,  // b'x100'
    RESVD_5 = 5,  // b'x101'
    BALR1415 = 6, // b'x110' == BALR  r14,r15
    BASR33 = 7,   // b'x111' == BASR  r3,r3
  };

  // The Associated Data Area (ADA) contains descriptors which help locating
  // external symbols. For each symbol and type, the displacement into the ADA
  // is stored.
  class AssociatedDataAreaTable {
  public:
    using DisplacementTable =
        MapVector<std::pair<const MCSymbol *, unsigned>, uint32_t>;

  private:
    const uint64_t PointerSize;

    /// The mapping of name/slot type pairs to displacements.
    DisplacementTable Displacements;

    /// The next available displacement value. Incremented when new entries into
    /// the ADA are created.
    uint32_t NextDisplacement = 0;

  public:
    AssociatedDataAreaTable(uint64_t PointerSize) : PointerSize(PointerSize) {}

    /// @brief Add a function descriptor to the ADA.
    /// @param MI Pointer to an ADA_ENTRY instruction.
    /// @return The displacement of the descriptor into the ADA.
    uint32_t insert(const MachineOperand MO);

    /// @brief Get the displacement into associated data area (ADA) for a name.
    /// If no  displacement is already associated with the name, assign one and
    /// return it.
    /// @param Sym The symbol for which the displacement should be returned.
    /// @param SlotKind The ADA type.
    /// @return The displacement of the descriptor into the ADA.
    uint32_t insert(const MCSymbol *Sym, unsigned SlotKind);

    /// Get the table of GOFF displacements.  This is 'const' since it should
    /// never be modified by anything except the APIs on this class.
    const DisplacementTable &getTable() const { return Displacements; }

    uint32_t getNextDisplacement() const { return NextDisplacement; }
  };

  AssociatedDataAreaTable ADATable;

  void emitPPA1(MCSymbol *FnEndSym);
  void emitPPA2(Module &M);
  void emitADASection();
  void emitIDRLSection(Module &M);

public:
  SystemZAsmPrinter(TargetMachine &TM, std::unique_ptr<MCStreamer> Streamer)
      : AsmPrinter(TM, std::move(Streamer), ID), CurrentFnPPA1Sym(nullptr),
        CurrentFnEPMarkerSym(nullptr), PPA2Sym(nullptr),
        ADATable(TM.getPointerSize(0)) {}

  // Override AsmPrinter.
  StringRef getPassName() const override { return "SystemZ Assembly Printer"; }
  void emitInstruction(const MachineInstr *MI) override;
  void emitMachineConstantPoolValue(MachineConstantPoolValue *MCPV) override;
  void emitEndOfAsmFile(Module &M) override;
  bool PrintAsmOperand(const MachineInstr *MI, unsigned OpNo,
                       const char *ExtraCode, raw_ostream &OS) override;
  bool PrintAsmMemoryOperand(const MachineInstr *MI, unsigned OpNo,
                             const char *ExtraCode, raw_ostream &OS) override;

  bool runOnMachineFunction(MachineFunction &MF) override {
    AsmPrinter::runOnMachineFunction(MF);

    // Emit the XRay table for this function.
    emitXRayTable();

    return false;
  }

  bool doInitialization(Module &M) override {
    SM.reset();
    return AsmPrinter::doInitialization(M);
  }
  void emitFunctionEntryLabel() override;
  void emitFunctionBodyEnd() override;
  void emitStartOfAsmFile(Module &M) override;

private:
  void emitCallInformation(CallType CT);
  void LowerFENTRY_CALL(const MachineInstr &MI, SystemZMCInstLower &MCIL);
  void LowerSTACKMAP(const MachineInstr &MI);
  void LowerPATCHPOINT(const MachineInstr &MI, SystemZMCInstLower &Lower);
  void LowerPATCHABLE_FUNCTION_ENTER(const MachineInstr &MI,
                                     SystemZMCInstLower &Lower);
  void LowerPATCHABLE_RET(const MachineInstr &MI, SystemZMCInstLower &Lower);
  void emitAttributes(Module &M);
};
} // end namespace vm::core

#endif
