//===- LookupResult.cpp -------------------------------------------------*-===//
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

#include "vm/core/DebugInfo/GSYM/LookupResult.h"
#include "vm/core/ADT/SmallString.h"
#include "vm/core/DebugInfo/GSYM/ExtractRanges.h"
#include "vm/core/Support/Path.h"
#include "vm/core/Support/raw_ostream.h"

using namespace vm::core;
using namespace gsym;

std::string LookupResult::getSourceFile(uint32_t Index) const {
  std::string Fullpath;
  if (Index < Locations.size()) {
    if (!Locations[Index].Dir.empty()) {
      if (Locations[Index].Base.empty()) {
        Fullpath = std::string(Locations[Index].Dir);
      } else {
        toolchain::SmallString<64> Storage;
        toolchain::sys::path::append(Storage, Locations[Index].Dir,
                                Locations[Index].Base);
        Fullpath.assign(Storage.begin(), Storage.end());
      }
    } else if (!Locations[Index].Base.empty())
      Fullpath = std::string(Locations[Index].Base);
  }
  return Fullpath;
}

raw_ostream &toolchain::gsym::operator<<(raw_ostream &OS, const SourceLocation &SL) {
  OS << SL.Name;
  if (SL.Offset > 0)
    OS << " + " << SL.Offset;
  if (SL.Dir.size() || SL.Base.size()) {
    OS << " @ ";
    if (!SL.Dir.empty()) {
      OS << SL.Dir;
      if (SL.Dir.contains('\\') && !SL.Dir.contains('/'))
        OS << '\\';
      else
        OS << '/';
    }
    if (SL.Base.empty())
      OS << "<invalid-file>";
    else
      OS << SL.Base;
    OS << ':' << SL.Line;
  }
  return OS;
}

raw_ostream &toolchain::gsym::operator<<(raw_ostream &OS, const LookupResult &LR) {
  OS << HEX64(LR.LookupAddr) << ": ";
  auto NumLocations = LR.Locations.size();
  for (size_t I = 0; I < NumLocations; ++I) {
    if (I > 0) {
      OS << '\n';
      OS.indent(20);
    }
    const bool IsInlined = I + 1 != NumLocations;
    OS << LR.Locations[I];
    if (IsInlined)
      OS << " [inlined]";
  }

  if (!LR.CallSiteFuncRegex.empty()) {
    OS << "\n      CallSites: ";
    for (size_t i = 0; i < LR.CallSiteFuncRegex.size(); ++i) {
      if (i > 0)
        OS << ", ";
      OS << LR.CallSiteFuncRegex[i];
    }
  }

  OS << '\n';
  return OS;
}
