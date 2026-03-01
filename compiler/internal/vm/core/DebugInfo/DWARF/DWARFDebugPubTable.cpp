//===- DWARFDebugPubTable.cpp ---------------------------------------------===//
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

#include "vm/core/DebugInfo/DWARF/DWARFDebugPubTable.h"
#include "vm/core/DebugInfo/DWARF/DWARFDataExtractor.h"
#include "vm/core/ADT/StringRef.h"
#include "vm/core/BinaryFormat/Dwarf.h"
#include "vm/core/Support/DataExtractor.h"
#include "vm/core/Support/Errc.h"
#include "vm/core/Support/Format.h"
#include "vm/core/Support/raw_ostream.h"
#include <cstdint>

using namespace vm::core;
using namespace dwarf;

void DWARFDebugPubTable::extract(
    DWARFDataExtractor Data, bool GnuStyle,
    function_ref<void(Error)> RecoverableErrorHandler) {
  this->GnuStyle = GnuStyle;
  Sets.clear();
  uint64_t Offset = 0;
  while (Data.isValidOffset(Offset)) {
    uint64_t SetOffset = Offset;
    Sets.push_back({});
    Set &NewSet = Sets.back();

    DataExtractor::Cursor C(Offset);
    std::tie(NewSet.Length, NewSet.Format) = Data.getInitialLength(C);
    if (!C) {
      // Drop the newly added set because it does not contain anything useful
      // to dump.
      Sets.pop_back();
      RecoverableErrorHandler(createStringError(
          errc::invalid_argument,
          "name lookup table at offset 0x%" PRIx64 " parsing failed: %s",
          SetOffset, toString(C.takeError()).c_str()));
      return;
    }

    Offset = C.tell() + NewSet.Length;
    DWARFDataExtractor SetData(Data, Offset);
    const unsigned OffsetSize = dwarf::getDwarfOffsetByteSize(NewSet.Format);

    NewSet.Version = SetData.getU16(C);
    NewSet.Offset = SetData.getRelocatedValue(C, OffsetSize);
    NewSet.Size = SetData.getUnsigned(C, OffsetSize);

    if (!C) {
      // Preserve the newly added set because at least some fields of the header
      // are read and can be dumped.
      RecoverableErrorHandler(
          createStringError(errc::invalid_argument,
                            "name lookup table at offset 0x%" PRIx64
                            " does not have a complete header: %s",
                            SetOffset, toString(C.takeError()).c_str()));
      continue;
    }

    while (C) {
      uint64_t DieRef = SetData.getUnsigned(C, OffsetSize);
      if (DieRef == 0)
        break;
      uint8_t IndexEntryValue = GnuStyle ? SetData.getU8(C) : 0;
      StringRef Name = SetData.getCStrRef(C);
      if (C)
        NewSet.Entries.push_back(
            {DieRef, PubIndexEntryDescriptor(IndexEntryValue), Name});
    }

    if (!C) {
      RecoverableErrorHandler(createStringError(
          errc::invalid_argument,
          "name lookup table at offset 0x%" PRIx64 " parsing failed: %s",
          SetOffset, toString(C.takeError()).c_str()));
      continue;
    }
    if (C.tell() != Offset)
      RecoverableErrorHandler(createStringError(
          errc::invalid_argument,
          "name lookup table at offset 0x%" PRIx64
          " has a terminator at offset 0x%" PRIx64
          " before the expected end at 0x%" PRIx64,
          SetOffset, C.tell() - OffsetSize, Offset - OffsetSize));
  }
}

void DWARFDebugPubTable::dump(raw_ostream &OS) const {
  for (const Set &S : Sets) {
    int OffsetDumpWidth = 2 * dwarf::getDwarfOffsetByteSize(S.Format);
    OS << "length = " << format("0x%0*" PRIx64, OffsetDumpWidth, S.Length);
    OS << ", format = " << dwarf::FormatString(S.Format);
    OS << ", version = " << format("0x%04x", S.Version);
    OS << ", unit_offset = "
       << format("0x%0*" PRIx64, OffsetDumpWidth, S.Offset);
    OS << ", unit_size = " << format("0x%0*" PRIx64, OffsetDumpWidth, S.Size)
       << '\n';
    OS << (GnuStyle ? "Offset     Linkage  Kind     Name\n"
                    : "Offset     Name\n");

    for (const Entry &E : S.Entries) {
      OS << format("0x%0*" PRIx64 " ", OffsetDumpWidth, E.SecOffset);
      if (GnuStyle) {
        StringRef EntryLinkage =
            GDBIndexEntryLinkageString(E.Descriptor.Linkage);
        StringRef EntryKind = dwarf::GDBIndexEntryKindString(E.Descriptor.Kind);
        OS << format("%-8s", EntryLinkage.data()) << ' '
           << format("%-8s", EntryKind.data()) << ' ';
      }
      OS << '\"' << E.Name << "\"\n";
    }
  }
}
