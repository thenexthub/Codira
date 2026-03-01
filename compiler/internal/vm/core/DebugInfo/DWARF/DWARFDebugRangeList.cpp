//===- DWARFDebugRangesList.cpp -------------------------------------------===//
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

#include "vm/core/DebugInfo/DWARF/DWARFDebugRangeList.h"
#include "vm/core/DebugInfo/DWARF/DWARFContext.h"
#include "vm/core/Support/Errc.h"
#include "vm/core/Support/Format.h"
#include "vm/core/Support/raw_ostream.h"
#include <cinttypes>
#include <cstdint>

using namespace vm::core;

bool DWARFDebugRangeList::RangeListEntry::isBaseAddressSelectionEntry(
    uint8_t AddressSize) const {
  assert(DWARFContext::isAddressSizeSupported(AddressSize));
  return StartAddress == dwarf::computeTombstoneAddress(AddressSize);
}

void DWARFDebugRangeList::clear() {
  Offset = -1ULL;
  AddressSize = 0;
  Entries.clear();
}

Error DWARFDebugRangeList::extract(const DWARFDataExtractor &data,
                                   uint64_t *offset_ptr) {
  clear();
  if (!data.isValidOffset(*offset_ptr))
    return createStringError(errc::invalid_argument,
                       "invalid range list offset 0x%" PRIx64, *offset_ptr);

  AddressSize = data.getAddressSize();
  if (Error SizeErr = DWARFContext::checkAddressSizeSupported(
          AddressSize, errc::invalid_argument,
          "range list at offset 0x%" PRIx64, *offset_ptr))
    return SizeErr;
  Offset = *offset_ptr;
  while (true) {
    RangeListEntry Entry;
    Entry.SectionIndex = -1ULL;

    uint64_t prev_offset = *offset_ptr;
    Entry.StartAddress = data.getRelocatedAddress(offset_ptr);
    Entry.EndAddress =
        data.getRelocatedAddress(offset_ptr, &Entry.SectionIndex);

    // Check that both values were extracted correctly.
    if (*offset_ptr != prev_offset + 2 * AddressSize) {
      clear();
      return createStringError(errc::invalid_argument,
                         "invalid range list entry at offset 0x%" PRIx64,
                         prev_offset);
    }
    if (Entry.isEndOfListEntry())
      break;
    Entries.push_back(Entry);
  }
  return Error::success();
}

void DWARFDebugRangeList::dump(raw_ostream &OS) const {
  const char *AddrFmt;
  switch (AddressSize) {
  case 2:
    AddrFmt = "%08" PRIx64 " %04" PRIx64 " %04" PRIx64 "\n";
    break;
  case 4:
    AddrFmt = "%08" PRIx64 " %08" PRIx64 " %08" PRIx64 "\n";
    break;
  case 8:
    AddrFmt = "%08" PRIx64 " %016" PRIx64 " %016" PRIx64 "\n";
    break;
  default:
    llvm_unreachable("unsupported address size");
  }
  for (const RangeListEntry &RLE : Entries)
    OS << format(AddrFmt, Offset, RLE.StartAddress, RLE.EndAddress);
  OS << format("%08" PRIx64 " <End of list>\n", Offset);
}

DWARFAddressRangesVector DWARFDebugRangeList::getAbsoluteRanges(
    std::optional<object::SectionedAddress> BaseAddr) const {
  DWARFAddressRangesVector Res;
  // debug_addr can't use the max integer tombstone because that's used for the
  // base address specifier entry - so use max-1.
  uint64_t Tombstone = dwarf::computeTombstoneAddress(AddressSize) - 1;
  for (const RangeListEntry &RLE : Entries) {
    if (RLE.isBaseAddressSelectionEntry(AddressSize)) {
      BaseAddr = {RLE.EndAddress, RLE.SectionIndex};
      continue;
    }

    DWARFAddressRange E;
    E.LowPC = RLE.StartAddress;
    if (E.LowPC == Tombstone)
      continue;
    E.HighPC = RLE.EndAddress;
    E.SectionIndex = RLE.SectionIndex;
    // Base address of a range list entry is determined by the closest preceding
    // base address selection entry in the same range list. It defaults to the
    // base address of the compilation unit if there is no such entry.
    if (BaseAddr) {
      if (BaseAddr->Address == Tombstone)
        continue;
      E.LowPC += BaseAddr->Address;
      E.HighPC += BaseAddr->Address;
      if (E.SectionIndex == -1ULL)
        E.SectionIndex = BaseAddr->SectionIndex;
    }
    Res.push_back(E);
  }
  return Res;
}
