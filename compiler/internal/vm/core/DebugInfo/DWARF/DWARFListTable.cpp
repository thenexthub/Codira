//===- DWARFListTable.cpp ---------------------------------------------===//
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

#include "vm/core/DebugInfo/DWARF/DWARFListTable.h"
#include "vm/core/BinaryFormat/Dwarf.h"
#include "vm/core/DebugInfo/DWARF/DWARFContext.h"
#include "vm/core/Support/Errc.h"
#include "vm/core/Support/Error.h"
#include "vm/core/Support/Format.h"
#include "vm/core/Support/raw_ostream.h"

using namespace vm::core;

Error DWARFListTableHeader::extract(DWARFDataExtractor Data,
                                    uint64_t *OffsetPtr) {
  HeaderOffset = *OffsetPtr;
  Error Err = Error::success();

  std::tie(HeaderData.Length, Format) = Data.getInitialLength(OffsetPtr, &Err);
  if (Err)
    return createStringError(
        errc::invalid_argument, "parsing %s table at offset 0x%" PRIx64 ": %s",
        SectionName.data(), HeaderOffset, toString(std::move(Err)).c_str());

  uint8_t OffsetByteSize = Format == dwarf::DWARF64 ? 8 : 4;
  uint64_t FullLength =
      HeaderData.Length + dwarf::getUnitLengthFieldByteSize(Format);
  if (FullLength < getHeaderSize(Format))
    return createStringError(errc::invalid_argument,
                       "%s table at offset 0x%" PRIx64
                       " has too small length (0x%" PRIx64
                       ") to contain a complete header",
                       SectionName.data(), HeaderOffset, FullLength);
  assert(FullLength == length() && "Inconsistent calculation of length.");
  uint64_t End = HeaderOffset + FullLength;
  if (!Data.isValidOffsetForDataOfSize(HeaderOffset, FullLength))
    return createStringError(errc::invalid_argument,
                       "section is not large enough to contain a %s table "
                       "of length 0x%" PRIx64 " at offset 0x%" PRIx64,
                       SectionName.data(), FullLength, HeaderOffset);

  HeaderData.Version = Data.getU16(OffsetPtr);
  HeaderData.AddrSize = Data.getU8(OffsetPtr);
  HeaderData.SegSize = Data.getU8(OffsetPtr);
  HeaderData.OffsetEntryCount = Data.getU32(OffsetPtr);

  // Perform basic validation of the remaining header fields.
  if (HeaderData.Version != 5)
    return createStringError(errc::invalid_argument,
                       "unrecognised %s table version %" PRIu16
                       " in table at offset 0x%" PRIx64,
                       SectionName.data(), HeaderData.Version, HeaderOffset);
  if (Error SizeErr = DWARFContext::checkAddressSizeSupported(
          HeaderData.AddrSize, errc::not_supported,
          "%s table at offset 0x%" PRIx64, SectionName.data(), HeaderOffset))
    return SizeErr;
  if (HeaderData.SegSize != 0)
    return createStringError(errc::not_supported,
                       "%s table at offset 0x%" PRIx64
                       " has unsupported segment selector size %" PRIu8,
                       SectionName.data(), HeaderOffset, HeaderData.SegSize);
  if (End < HeaderOffset + getHeaderSize(Format) +
                HeaderData.OffsetEntryCount * OffsetByteSize)
    return createStringError(errc::invalid_argument,
        "%s table at offset 0x%" PRIx64 " has more offset entries (%" PRIu32
        ") than there is space for",
        SectionName.data(), HeaderOffset, HeaderData.OffsetEntryCount);
  Data.setAddressSize(HeaderData.AddrSize);
  *OffsetPtr += HeaderData.OffsetEntryCount * OffsetByteSize;
  return Error::success();
}

void DWARFListTableHeader::dump(DataExtractor Data, raw_ostream &OS,
                                DIDumpOptions DumpOpts) const {
  if (DumpOpts.Verbose)
    OS << format("0x%8.8" PRIx64 ": ", HeaderOffset);
  int OffsetDumpWidth = 2 * dwarf::getDwarfOffsetByteSize(Format);
  OS << format("%s list header: length = 0x%0*" PRIx64, ListTypeString.data(),
               OffsetDumpWidth, HeaderData.Length)
     << ", format = " << dwarf::FormatString(Format)
     << format(", version = 0x%4.4" PRIx16 ", addr_size = 0x%2.2" PRIx8
               ", seg_size = 0x%2.2" PRIx8
               ", offset_entry_count = 0x%8.8" PRIx32 "\n",
               HeaderData.Version, HeaderData.AddrSize, HeaderData.SegSize,
               HeaderData.OffsetEntryCount);

  if (HeaderData.OffsetEntryCount > 0) {
    OS << "offsets: [";
    for (uint32_t I = 0; I < HeaderData.OffsetEntryCount; ++I) {
      auto Off = *getOffsetEntry(Data, I);
      OS << format("\n0x%0*" PRIx64, OffsetDumpWidth, Off);
      if (DumpOpts.Verbose)
        OS << format(" => 0x%08" PRIx64,
                     Off + HeaderOffset + getHeaderSize(Format));
    }
    OS << "\n]\n";
  }
}

uint64_t DWARFListTableHeader::length() const {
  if (HeaderData.Length == 0)
    return 0;
  return HeaderData.Length + dwarf::getUnitLengthFieldByteSize(Format);
}
