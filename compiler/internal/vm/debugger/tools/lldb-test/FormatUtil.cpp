//===- FormatUtil.cpp ----------------------------------------- *- C++ --*-===//
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

#include "FormatUtil.h"
#include "llvm/Support/Format.h"
#include "llvm/Support/FormatVariadic.h"

using namespace lldb_private;
using namespace llvm;

LinePrinter::Line::~Line() {
  if (P)
    P->NewLine();
}

LinePrinter::LinePrinter(int Indent, llvm::raw_ostream &Stream)
    : OS(Stream), IndentSpaces(Indent), CurrentIndent(0) {}

void LinePrinter::Indent(uint32_t Amount) {
  if (Amount == 0)
    Amount = IndentSpaces;
  CurrentIndent += Amount;
}

void LinePrinter::Unindent(uint32_t Amount) {
  if (Amount == 0)
    Amount = IndentSpaces;
  CurrentIndent = std::max<int>(0, CurrentIndent - Amount);
}

void LinePrinter::NewLine() {
  OS << "\n";
}

void LinePrinter::formatBinary(StringRef Label, ArrayRef<uint8_t> Data,
                               uint32_t StartOffset) {
  if (Data.empty()) {
    line() << Label << " ()";
    return;
  }
  line() << Label << " (";
  OS << format_bytes_with_ascii(Data, StartOffset, 32, 4,
                                CurrentIndent + IndentSpaces, true);
  NewLine();
  line() << ")";
}

void LinePrinter::formatBinary(StringRef Label, ArrayRef<uint8_t> Data,
                               uint64_t Base, uint32_t StartOffset) {
  if (Data.empty()) {
    line() << Label << " ()";
    return;
  }
  line() << Label << " (";
  Base += StartOffset;
  OS << format_bytes_with_ascii(Data, Base, 32, 4, CurrentIndent + IndentSpaces,
                                true);
  NewLine();
  line() << ")";
}
