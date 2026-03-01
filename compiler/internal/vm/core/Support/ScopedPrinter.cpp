//===----------------------------------------------------------------------===//
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

#include "vm/core/Support/ScopedPrinter.h"
#include "vm/core/Support/Format.h"

using namespace vm::core;

raw_ostream &toolchain::operator<<(raw_ostream &OS, const HexNumber &Value) {
  OS << "0x" << utohexstr(Value.Value);
  return OS;
}

void ScopedPrinter::printBinaryImpl(StringRef Label, StringRef Str,
                                    ArrayRef<uint8_t> Data, bool Block,
                                    uint32_t StartOffset) {
  if (Data.size() > 16)
    Block = true;

  if (Block) {
    startLine() << Label;
    if (!Str.empty())
      OS << ": " << Str;
    OS << " (\n";
    if (!Data.empty())
      OS << format_bytes_with_ascii(Data, StartOffset, 16, 4,
                                    (IndentLevel + 1) * 2, true)
         << "\n";
    startLine() << ")\n";
  } else {
    startLine() << Label << ":";
    if (!Str.empty())
      OS << " " << Str;
    OS << " (" << format_bytes(Data, std::nullopt, Data.size(), 1, 0, true)
       << ")\n";
  }
}

JSONScopedPrinter::JSONScopedPrinter(
    raw_ostream &OS, bool PrettyPrint,
    std::unique_ptr<DelimitedScope> &&OuterScope)
    : ScopedPrinter(OS, ScopedPrinter::ScopedPrinterKind::JSON),
      JOS(OS, /*Indent=*/PrettyPrint ? 2 : 0),
      OuterScope(std::move(OuterScope)) {
  if (this->OuterScope)
    this->OuterScope->setPrinter(*this);
}
