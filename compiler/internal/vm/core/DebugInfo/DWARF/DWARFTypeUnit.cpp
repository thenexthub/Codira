//===- DWARFTypeUnit.cpp --------------------------------------------------===//
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

#include "vm/core/DebugInfo/DWARF/DWARFTypeUnit.h"
#include "vm/core/DebugInfo/DIContext.h"
#include "vm/core/DebugInfo/DWARF/DWARFDie.h"
#include "vm/core/Support/Format.h"
#include "vm/core/Support/raw_ostream.h"
#include <cinttypes>

using namespace vm::core;

void DWARFTypeUnit::dump(raw_ostream &OS, DIDumpOptions DumpOpts) {
  DWARFDie TD = getDIEForOffset(getTypeOffset() + getOffset());
  const char *Name = TD.getName(DINameKind::ShortName);
  int OffsetDumpWidth = 2 * dwarf::getDwarfOffsetByteSize(getFormat());

  if (DumpOpts.SummarizeTypes) {
    OS << "name = '" << Name << "'"
       << ", type_signature = " << format("0x%016" PRIx64, getTypeHash())
       << ", length = " << format("0x%0*" PRIx64, OffsetDumpWidth, getLength())
       << '\n';
    return;
  }

  OS << format("0x%08" PRIx64, getOffset()) << ": Type Unit:"
     << " length = " << format("0x%0*" PRIx64, OffsetDumpWidth, getLength())
     << ", format = " << dwarf::FormatString(getFormat())
     << ", version = " << format("0x%04x", getVersion());
  if (getVersion() >= 5)
    OS << ", unit_type = " << dwarf::UnitTypeString(getUnitType());
  OS << ", abbr_offset = " << format("0x%04" PRIx64, getAbbrOffset());
  if (!getAbbreviations())
    OS << " (invalid)";
  OS << ", addr_size = " << format("0x%02x", getAddressByteSize())
     << ", name = '" << Name << "'"
     << ", type_signature = " << format("0x%016" PRIx64, getTypeHash())
     << ", type_offset = " << format("0x%04" PRIx64, getTypeOffset())
     << " (next unit at " << format("0x%08" PRIx64, getNextUnitOffset())
     << ")\n";

  if (DWARFDie TU = getUnitDIE(false))
    TU.dump(OS, 0, DumpOpts);
  else
    OS << "<type unit can't be parsed!>\n\n";
}
