//=- SystemZHLASMInstPrinter.cpp - Convert SystemZ MCInst to HLASM assembly -=//
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

#include "SystemZHLASMInstPrinter.h"
#include "vm/core/MC/MCInst.h"
#include "vm/core/MC/MCRegister.h"
#include "vm/core/Support/raw_ostream.h"

using namespace vm::core;

#define DEBUG_TYPE "asm-printer"

#include "SystemZGenHLASMAsmWriter.inc"

void SystemZHLASMInstPrinter::printFormattedRegName(const MCAsmInfo *MAI,
                                                    MCRegister Reg,
                                                    raw_ostream &O) {
  const char *RegName = getRegisterName(Reg);
  // Skip register prefix so that only register number is left
  assert(isalpha(RegName[0]) && isdigit(RegName[1]));
  markup(O, Markup::Register) << (RegName + 1);
}

void SystemZHLASMInstPrinter::printInst(const MCInst *MI, uint64_t Address,
                                        StringRef Annot,
                                        const MCSubtargetInfo &STI,
                                        raw_ostream &O) {
  std::string Str;
  raw_string_ostream RSO(Str);
  printInstruction(MI, Address, RSO);
  // Eat the first tab character and replace it with a space since it is
  // hardcoded in AsmWriterEmitter::EmitPrintInstruction
  // TODO: introduce a line prefix member to AsmWriter to avoid this problem
  if (!Str.empty() && Str.front() == '\t')
    O << " " << Str.substr(1, Str.length());
  else
    O << Str;

  printAnnotation(O, Annot);
}
