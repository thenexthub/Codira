//===- MCAsmMacro.h - Assembly Macros ---------------------------*- C++ -*-===//
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

#include "vm/core/MC/MCAsmMacro.h"
#include "vm/core/Support/raw_ostream.h"

using namespace vm::core;

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void MCAsmMacroParameter::dump(raw_ostream &OS) const {
  OS << "\"" << Name << "\"";
  if (Required)
    OS << ":req";
  if (Vararg)
    OS << ":vararg";
  if (!Value.empty()) {
    OS << " = ";
    bool first = true;
    for (const AsmToken &T : Value) {
      if (!first)
        OS << ", ";
      first = false;
      OS << T.getString();
    }
  }
  OS << "\n";
}

void MCAsmMacro::dump(raw_ostream &OS) const {
  OS << "Macro " << Name << ":\n";
  OS << "  Parameters:\n";
  for (const MCAsmMacroParameter &P : Parameters) {
    OS << "    ";
    P.dump();
  }
  if (!Locals.empty()) {
    OS << "  Locals:\n";
    for (StringRef L : Locals)
      OS << "    " << L << '\n';
  }
  OS << "  (BEGIN BODY)" << Body << "(END BODY)\n";
}
#endif
