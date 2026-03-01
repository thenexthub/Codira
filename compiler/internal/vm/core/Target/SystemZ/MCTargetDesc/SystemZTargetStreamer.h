//=- SystemZTargetStreamer.h - SystemZ Target Streamer ----------*- C++ -*-===//
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

#ifndef LLVM_LIB_TARGET_SYSTEMZ_SYSTEMZTARGETSTREAMER_H
#define LLVM_LIB_TARGET_SYSTEMZ_SYSTEMZTARGETSTREAMER_H

#include "vm/core/ADT/StringRef.h"
#include "vm/core/MC/MCContext.h"
#include "vm/core/MC/MCExpr.h"
#include "vm/core/MC/MCInst.h"
#include "vm/core/MC/MCStreamer.h"
#include "vm/core/MC/MCSymbol.h"
#include "vm/core/Support/FormattedStream.h"
#include <map>
#include <utility>

namespace vm::core {
class MCGOFFStreamer;
class SystemZHLASMAsmStreamer;

class SystemZTargetStreamer : public MCTargetStreamer {
public:
  SystemZTargetStreamer(MCStreamer &S) : MCTargetStreamer(S) {}

  typedef std::pair<MCInst, const MCSubtargetInfo *> MCInstSTIPair;
  struct CmpMCInst {
    bool operator()(const MCInstSTIPair &MCI_STI_A,
                    const MCInstSTIPair &MCI_STI_B) const {
      if (MCI_STI_A.second != MCI_STI_B.second)
        return uintptr_t(MCI_STI_A.second) < uintptr_t(MCI_STI_B.second);
      const MCInst &A = MCI_STI_A.first;
      const MCInst &B = MCI_STI_B.first;
      assert(A.getNumOperands() == B.getNumOperands() &&
             A.getNumOperands() == 5 && A.getOperand(2).getImm() == 1 &&
             B.getOperand(2).getImm() == 1 && "Unexpected EXRL target MCInst");
      if (A.getOpcode() != B.getOpcode())
        return A.getOpcode() < B.getOpcode();
      if (A.getOperand(0).getReg() != B.getOperand(0).getReg())
        return A.getOperand(0).getReg() < B.getOperand(0).getReg();
      if (A.getOperand(1).getImm() != B.getOperand(1).getImm())
        return A.getOperand(1).getImm() < B.getOperand(1).getImm();
      if (A.getOperand(3).getReg() != B.getOperand(3).getReg())
        return A.getOperand(3).getReg() < B.getOperand(3).getReg();
      if (A.getOperand(4).getImm() != B.getOperand(4).getImm())
        return A.getOperand(4).getImm() < B.getOperand(4).getImm();
      return false;
    }
  };
  typedef std::map<MCInstSTIPair, MCSymbol *, CmpMCInst> EXRLT2SymMap;
  EXRLT2SymMap EXRLTargets2Sym;

  void emitConstantPools() override;

  virtual void emitMachine(StringRef CPUOrCommand) {};

  virtual const MCExpr *createWordDiffExpr(MCContext &Ctx, const MCSymbol *Hi,
                                           const MCSymbol *Lo) {
    return nullptr;
  }
};

class SystemZTargetGOFFStreamer : public SystemZTargetStreamer {
public:
  SystemZTargetGOFFStreamer(MCStreamer &S) : SystemZTargetStreamer(S) {}
  const MCExpr *createWordDiffExpr(MCContext &Ctx, const MCSymbol *Hi,
                                   const MCSymbol *Lo) override;
};

class SystemZTargetHLASMStreamer : public SystemZTargetStreamer {
  formatted_raw_ostream &OS;

public:
  SystemZTargetHLASMStreamer(MCStreamer &S, formatted_raw_ostream &OS)
      : SystemZTargetStreamer(S), OS(OS) {}
  SystemZHLASMAsmStreamer &getHLASMStreamer();
  const MCExpr *createWordDiffExpr(MCContext &Ctx, const MCSymbol *Hi,
                                   const MCSymbol *Lo) override;
};

class SystemZTargetELFStreamer : public SystemZTargetStreamer {
public:
  SystemZTargetELFStreamer(MCStreamer &S) : SystemZTargetStreamer(S) {}
  void emitMachine(StringRef CPUOrCommand) override {}
};

class SystemZTargetGNUStreamer : public SystemZTargetStreamer {
  formatted_raw_ostream &OS;

public:
  SystemZTargetGNUStreamer(MCStreamer &S, formatted_raw_ostream &OS)
      : SystemZTargetStreamer(S), OS(OS) {}
  void emitMachine(StringRef CPUOrCommand) override {
    OS << "\t.machine " << CPUOrCommand << "\n";
  }
};

} // end namespace vm::core

#endif // LLVM_LIB_TARGET_SYSTEMZ_SYSTEMZTARGETSTREAMER_H
