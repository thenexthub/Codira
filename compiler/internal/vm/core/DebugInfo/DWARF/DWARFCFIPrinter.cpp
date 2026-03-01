//===- DWARFCFIPrinter.cpp - Print the cfi-portions of .debug_frame -------===//
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

#include "vm/core/DebugInfo/DWARF/DWARFCFIPrinter.h"
#include "vm/core/DebugInfo/DIContext.h"
#include "vm/core/DebugInfo/DWARF/DWARFExpressionPrinter.h"
#include "vm/core/DebugInfo/DWARF/LowLevel/DWARFCFIProgram.h"
#include "vm/core/Support/Compiler.h"
#include "vm/core/Support/ErrorHandling.h"
#include "vm/core/Support/Format.h"
#include "vm/core/Support/raw_ostream.h"
#include <cassert>
#include <cinttypes>
#include <cstdint>
#include <optional>

using namespace vm::core;
using namespace dwarf;

static void printRegister(raw_ostream &OS, const DIDumpOptions &DumpOpts,
                          unsigned RegNum) {
  if (DumpOpts.GetNameForDWARFReg) {
    auto RegName = DumpOpts.GetNameForDWARFReg(RegNum, DumpOpts.IsEH);
    if (!RegName.empty()) {
      OS << RegName;
      return;
    }
  }
  OS << "reg" << RegNum;
}

/// Print \p Opcode's operand number \p OperandIdx which has value \p Operand.
static void printOperand(raw_ostream &OS, const DIDumpOptions &DumpOpts,
                         const CFIProgram &P,
                         const CFIProgram::Instruction &Instr,
                         unsigned OperandIdx, uint64_t Operand,
                         std::optional<uint64_t> &Address) {
  assert(OperandIdx < CFIProgram::MaxOperands);
  uint8_t Opcode = Instr.Opcode;
  CFIProgram::OperandType Type = P.getOperandTypes()[Opcode][OperandIdx];

  switch (Type) {
  case CFIProgram::OT_Unset: {
    OS << " Unsupported " << (OperandIdx ? "second" : "first") << " operand to";
    auto OpcodeName = P.callFrameString(Opcode);
    if (!OpcodeName.empty())
      OS << " " << OpcodeName;
    else
      OS << format(" Opcode %x", Opcode);
    break;
  }
  case CFIProgram::OT_None:
    break;
  case CFIProgram::OT_Address:
    OS << format(" %" PRIx64, Operand);
    Address = Operand;
    break;
  case CFIProgram::OT_Offset:
    // The offsets are all encoded in a unsigned form, but in practice
    // consumers use them signed. It's most certainly legacy due to
    // the lack of signed variants in the first Dwarf standards.
    OS << format(" %+" PRId64, int64_t(Operand));
    break;
  case CFIProgram::OT_FactoredCodeOffset: // Always Unsigned
    if (P.codeAlign())
      OS << format(" %" PRId64, Operand * P.codeAlign());
    else
      OS << format(" %" PRId64 "*code_alignment_factor", Operand);
    if (Address && P.codeAlign()) {
      *Address += Operand * P.codeAlign();
      OS << format(" to 0x%" PRIx64, *Address);
    }
    break;
  case CFIProgram::OT_SignedFactDataOffset:
    if (P.dataAlign())
      OS << format(" %" PRId64, int64_t(Operand) * P.dataAlign());
    else
      OS << format(" %" PRId64 "*data_alignment_factor", int64_t(Operand));
    break;
  case CFIProgram::OT_UnsignedFactDataOffset:
    if (P.dataAlign())
      OS << format(" %" PRId64, Operand * P.dataAlign());
    else
      OS << format(" %" PRId64 "*data_alignment_factor", Operand);
    break;
  case CFIProgram::OT_Register:
    OS << ' ';
    printRegister(OS, DumpOpts, Operand);
    break;
  case CFIProgram::OT_AddressSpace:
    OS << format(" in addrspace%" PRId64, Operand);
    break;
  case CFIProgram::OT_Expression:
    assert(Instr.Expression && "missing DWARFExpression object");
    OS << " ";
    printDwarfExpression(&Instr.Expression.value(), OS, DumpOpts, nullptr);
    break;
  }
}

void toolchain::dwarf::printCFIProgram(const CFIProgram &P, raw_ostream &OS,
                                  const DIDumpOptions &DumpOpts,
                                  unsigned IndentLevel,
                                  std::optional<uint64_t> Address) {
  for (const auto &Instr : P) {
    uint8_t Opcode = Instr.Opcode;
    OS.indent(2 * IndentLevel);
    OS << P.callFrameString(Opcode) << ":";
    for (size_t i = 0; i < Instr.Ops.size(); ++i)
      printOperand(OS, DumpOpts, P, Instr, i, Instr.Ops[i], Address);
    OS << '\n';
  }
}
