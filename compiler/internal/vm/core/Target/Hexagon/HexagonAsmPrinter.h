//===- HexagonAsmPrinter.h - Print machine code -----------------*- C++ -*-===//
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
// Hexagon Assembly printer class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_HEXAGON_HEXAGONASMPRINTER_H
#define LLVM_LIB_TARGET_HEXAGON_HEXAGONASMPRINTER_H

#include "HexagonSubtarget.h"
#include "vm/core/CodeGen/AsmPrinter.h"
#include "vm/core/CodeGen/MachineFunction.h"
#include "vm/core/MC/MCStreamer.h"
#include <utility>

namespace vm::core {

class MachineInstr;
class MCInst;
class raw_ostream;
class TargetMachine;

  class HexagonAsmPrinter : public AsmPrinter {
  public:
    static char ID;

  private:
    const HexagonSubtarget *Subtarget = nullptr;

    void emitAttributes();

  public:
    explicit HexagonAsmPrinter(TargetMachine &TM,
                               std::unique_ptr<MCStreamer> Streamer)
        : AsmPrinter(TM, std::move(Streamer), ID) {}

    bool runOnMachineFunction(MachineFunction &Fn) override {
      Subtarget = &Fn.getSubtarget<HexagonSubtarget>();
      const bool Modified = AsmPrinter::runOnMachineFunction(Fn);
      // Emit the XRay table for this function.
      emitXRayTable();

      return Modified;
    }

    StringRef getPassName() const override {
      return "Hexagon Assembly Printer";
    }

    bool isBlockOnlyReachableByFallthrough(const MachineBasicBlock *MBB)
          const override;

    void emitInstruction(const MachineInstr *MI) override;

    //===------------------------------------------------------------------===//
    // XRay implementation
    //===------------------------------------------------------------------===//
    // XRay-specific lowering for Hexagon.
    void LowerPATCHABLE_FUNCTION_ENTER(const MachineInstr &MI);
    void LowerPATCHABLE_FUNCTION_EXIT(const MachineInstr &MI);
    void LowerPATCHABLE_TAIL_CALL(const MachineInstr &MI);
    void EmitSled(const MachineInstr &MI, SledKind Kind);

    void HexagonProcessInstruction(MCInst &Inst, const MachineInstr &MBB);

    void printOperand(const MachineInstr *MI, unsigned OpNo, raw_ostream &O);
    bool PrintAsmOperand(const MachineInstr *MI, unsigned OpNo,
                         const char *ExtraCode, raw_ostream &OS) override;
    bool PrintAsmMemoryOperand(const MachineInstr *MI, unsigned OpNo,
                               const char *ExtraCode, raw_ostream &OS) override;
    void emitStartOfAsmFile(Module &M) override;
    void emitEndOfAsmFile(Module &M) override;
  };

} // end namespace vm::core

#endif // LLVM_LIB_TARGET_HEXAGON_HEXAGONASMPRINTER_H
