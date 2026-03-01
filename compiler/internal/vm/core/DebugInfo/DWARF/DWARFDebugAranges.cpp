//===- DWARFDebugAranges.cpp ----------------------------------------------===//
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

#include "vm/core/DebugInfo/DWARF/DWARFDebugAranges.h"
#include "vm/core/DebugInfo/DWARF/DWARFAddressRange.h"
#include "vm/core/DebugInfo/DWARF/DWARFContext.h"
#include "vm/core/DebugInfo/DWARF/DWARFDataExtractor.h"
#include "vm/core/DebugInfo/DWARF/DWARFDebugArangeSet.h"
#include "vm/core/DebugInfo/DWARF/DWARFObject.h"
#include "vm/core/DebugInfo/DWARF/DWARFUnit.h"
#include <cassert>
#include <cstdint>
#include <set>

using namespace vm::core;

void DWARFDebugAranges::extract(
    DWARFDataExtractor DebugArangesData,
    function_ref<void(Error)> RecoverableErrorHandler,
    function_ref<void(Error)> WarningHandler) {
  if (!DebugArangesData.isValidOffset(0))
    return;
  uint64_t Offset = 0;
  DWARFDebugArangeSet Set;

  while (DebugArangesData.isValidOffset(Offset)) {
    if (Error E = Set.extract(DebugArangesData, &Offset, WarningHandler)) {
      RecoverableErrorHandler(std::move(E));
      return;
    }
    uint64_t CUOffset = Set.getCompileUnitDIEOffset();
    for (const auto &Desc : Set.descriptors()) {
      uint64_t LowPC = Desc.Address;
      uint64_t HighPC = Desc.getEndAddress();
      appendRange(CUOffset, LowPC, HighPC);
    }
    ParsedCUOffsets.insert(CUOffset);
  }
}

void DWARFDebugAranges::generate(DWARFContext *CTX) {
  clear();
  if (!CTX)
    return;

  // Extract aranges from .debug_aranges section.
  DWARFDataExtractor ArangesData(CTX->getDWARFObj().getArangesSection(),
                                 CTX->isLittleEndian(), 0);
  extract(ArangesData, CTX->getRecoverableErrorHandler(),
          CTX->getWarningHandler());

  // Generate aranges from DIEs: even if .debug_aranges section is present,
  // it may describe only a small subset of compilation units, so we need to
  // manually build aranges for the rest of them.
  for (const auto &CU : CTX->compile_units()) {
    uint64_t CUOffset = CU->getOffset();
    if (ParsedCUOffsets.insert(CUOffset).second) {
      Expected<DWARFAddressRangesVector> CURanges = CU->collectAddressRanges();
      if (!CURanges)
        CTX->getRecoverableErrorHandler()(CURanges.takeError());
      else
        for (const auto &R : *CURanges)
          appendRange(CUOffset, R.LowPC, R.HighPC);
    }
  }

  construct();
}

void DWARFDebugAranges::clear() {
  Endpoints.clear();
  Aranges.clear();
  ParsedCUOffsets.clear();
}

void DWARFDebugAranges::appendRange(uint64_t CUOffset, uint64_t LowPC,
                                    uint64_t HighPC) {
  if (LowPC >= HighPC)
    return;
  Endpoints.emplace_back(LowPC, CUOffset, true);
  Endpoints.emplace_back(HighPC, CUOffset, false);
}

void DWARFDebugAranges::construct() {
  std::multiset<uint64_t> ValidCUs;  // Maintain the set of CUs describing
                                     // a current address range.
  toolchain::sort(Endpoints);
  uint64_t PrevAddress = -1ULL;
  for (const auto &E : Endpoints) {
    if (PrevAddress < E.Address && !ValidCUs.empty()) {
      // If the address range between two endpoints is described by some
      // CU, first try to extend the last range in Aranges. If we can't
      // do it, start a new range.
      if (!Aranges.empty() && Aranges.back().HighPC() == PrevAddress &&
          ValidCUs.find(Aranges.back().CUOffset) != ValidCUs.end()) {
        Aranges.back().setHighPC(E.Address);
      } else {
        Aranges.emplace_back(PrevAddress, E.Address, *ValidCUs.begin());
      }
    }
    // Update the set of valid CUs.
    if (E.IsRangeStart) {
      ValidCUs.insert(E.CUOffset);
    } else {
      auto CUPos = ValidCUs.find(E.CUOffset);
      assert(CUPos != ValidCUs.end());
      ValidCUs.erase(CUPos);
    }
    PrevAddress = E.Address;
  }
  assert(ValidCUs.empty());

  // Endpoints are not needed now.
  Endpoints.clear();
  Endpoints.shrink_to_fit();
}

uint64_t DWARFDebugAranges::findAddress(uint64_t Address) const {
  RangeCollIterator It =
      partition_point(Aranges, [=](Range R) { return R.HighPC() <= Address; });
  if (It != Aranges.end() && It->LowPC <= Address)
    return It->CUOffset;
  return -1ULL;
}
