//===-- MCAsmInfoWasm.cpp - Wasm asm properties -----------------*- C++ -*-===//
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
// This file defines target asm properties related what form asm statements
// should take in general on Wasm-based targets
//
//===----------------------------------------------------------------------===//

#include "vm/core/MC/MCAsmInfoWasm.h"
#include "vm/core/MC/MCSectionWasm.h"
#include "vm/core/MC/MCSymbolWasm.h"
#include "vm/core/Support/raw_ostream.h"

using namespace vm::core;

MCAsmInfoWasm::MCAsmInfoWasm() {
  HasIdentDirective = true;
  HasNoDeadStrip = true;
  WeakRefDirective = "\t.weak\t";
  PrivateGlobalPrefix = ".L";
  PrivateLabelPrefix = ".L";
}

static void printName(raw_ostream &OS, StringRef Name) {
  if (Name.find_first_not_of("0123456789_."
                             "abcdefghijklmnopqrstuvwxyz"
                             "ABCDEFGHIJKLMNOPQRSTUVWXYZ") == Name.npos) {
    OS << Name;
    return;
  }
  OS << '"';
  for (const char *B = Name.begin(), *E = Name.end(); B < E; ++B) {
    if (*B == '"') // Unquoted "
      OS << "\\\"";
    else if (*B != '\\') // Neither " or backslash
      OS << *B;
    else if (B + 1 == E) // Trailing backslash
      OS << "\\\\";
    else {
      OS << B[0] << B[1]; // Quoted character
      ++B;
    }
  }
  OS << '"';
}

void MCAsmInfoWasm::printSwitchToSection(const MCSection &Section,
                                         uint32_t Subsection, const Triple &T,
                                         raw_ostream &OS) const {
  auto &Sec = static_cast<const MCSectionWasm &>(Section);
  if (shouldOmitSectionDirective(Sec.getName())) {
    OS << '\t' << Sec.getName();
    if (Subsection)
      OS << '\t' << Subsection;
    OS << '\n';
    return;
  }

  OS << "\t.section\t";
  printName(OS, Sec.getName());
  OS << ",\"";

  if (Sec.IsPassive)
    OS << 'p';
  if (Sec.Group)
    OS << 'G';
  if (Sec.SegmentFlags & wasm::WASM_SEG_FLAG_STRINGS)
    OS << 'S';
  if (Sec.SegmentFlags & wasm::WASM_SEG_FLAG_TLS)
    OS << 'T';
  if (Sec.SegmentFlags & wasm::WASM_SEG_FLAG_RETAIN)
    OS << 'R';

  OS << '"';

  OS << ',';

  // If comment string is '@', e.g. as on ARM - use '%' instead
  if (getCommentString()[0] == '@')
    OS << '%';
  else
    OS << '@';

  // TODO: Print section type.

  if (Sec.Group) {
    OS << ",";
    printName(OS, Sec.Group->getName());
    OS << ",comdat";
  }

  if (Sec.isUnique())
    OS << ",unique," << Sec.UniqueID;

  OS << '\n';

  if (Subsection)
    OS << "\t.subsection\t" << Subsection << '\n';
}
