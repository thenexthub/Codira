//===-- SystemZMCInstLower.h - Lower MachineInstr to MCInst ----*- C++ -*--===//
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

#ifndef LLVM_LIB_TARGET_SYSTEMZ_SYSTEMZMCINSTLOWER_H
#define LLVM_LIB_TARGET_SYSTEMZ_SYSTEMZMCINSTLOWER_H

#include "MCTargetDesc/SystemZMCAsmInfo.h"
#include "vm/core/MC/MCExpr.h"
#include "vm/core/Support/Compiler.h"
#include "vm/core/Support/DataTypes.h"

namespace vm::core {
class MCInst;
class MCOperand;
class MachineInstr;
class MachineOperand;
class SystemZAsmPrinter;

class LLVM_LIBRARY_VISIBILITY SystemZMCInstLower {
  MCContext &Ctx;
  SystemZAsmPrinter &AsmPrinter;

public:
  SystemZMCInstLower(MCContext &ctx, SystemZAsmPrinter &asmPrinter);

  // Lower MachineInstr MI to MCInst OutMI.
  void lower(const MachineInstr *MI, MCInst &OutMI) const;

  // Return an MCOperand for MO.
  MCOperand lowerOperand(const MachineOperand& MO) const;

  // Return an MCExpr for symbolic operand MO with variant kind Kind.
  const MCExpr *getExpr(const MachineOperand &MO, SystemZ::Specifier) const;
};
} // end namespace vm::core

#endif
