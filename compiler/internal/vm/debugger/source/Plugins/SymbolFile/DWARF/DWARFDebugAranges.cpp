//===-- DWARFDebugAranges.cpp ---------------------------------------------===//
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

#include "DWARFDebugAranges.h"
#include "DWARFUnit.h"
#include "LogChannelDWARF.h"
#include "lldb/Utility/Log.h"
#include "lldb/Utility/Timer.h"
#include "llvm/DebugInfo/DWARF/DWARFDebugArangeSet.h"

using namespace lldb;
using namespace lldb_private;
using namespace lldb_private::plugin::dwarf;
using llvm::DWARFDebugArangeSet;

// Constructor
DWARFDebugAranges::DWARFDebugAranges() : m_aranges() {}

// Extract
void DWARFDebugAranges::extract(const DWARFDataExtractor &debug_aranges_data) {
  llvm::DWARFDataExtractor dwarf_data = debug_aranges_data.GetAsLLVMDWARF();
  lldb::offset_t offset = 0;

  DWARFDebugArangeSet set;
  Range range;
  while (dwarf_data.isValidOffset(offset)) {
    const lldb::offset_t set_offset = offset;
    if (llvm::Error error = set.extract(dwarf_data, &offset)) {
      Log *log = GetLog(DWARFLog::DebugInfo);
      LLDB_LOG_ERROR(log, std::move(error),
                     "DWARFDebugAranges::extract failed to extract "
                     ".debug_aranges set at offset {1:x}: {0}",
                     set_offset);
      set.clear();
      return;
    }
    const uint64_t cu_offset = set.getCompileUnitDIEOffset();
    for (const auto &desc : set.descriptors()) {
      if (desc.Length != 0)
        m_aranges.Append(
            RangeToDIE::Entry(desc.Address, desc.Length, cu_offset));
    }
  }
}

void DWARFDebugAranges::Dump(Log *log) const {
  if (log == nullptr)
    return;

  const size_t num_entries = m_aranges.GetSize();
  for (size_t i = 0; i < num_entries; ++i) {
    const RangeToDIE::Entry *entry = m_aranges.GetEntryAtIndex(i);
    if (entry)
      LLDB_LOG(log, "{0:x8}: [{1:x16} - {2:x16})", entry->data,
               entry->GetRangeBase(), entry->GetRangeEnd());
  }
}

void DWARFDebugAranges::AppendRange(dw_offset_t offset, dw_addr_t low_pc,
                                    dw_addr_t high_pc) {
  if (high_pc > low_pc)
    m_aranges.Append(RangeToDIE::Entry(low_pc, high_pc - low_pc, offset));
}

void DWARFDebugAranges::Sort(bool minimize) {
  LLDB_SCOPED_TIMER();

  m_aranges.Sort();
  m_aranges.CombineConsecutiveEntriesWithEqualData();
}

// FindAddress
dw_offset_t DWARFDebugAranges::FindAddress(dw_addr_t address) const {
  const RangeToDIE::Entry *entry = m_aranges.FindEntryThatContains(address);
  if (entry)
    return entry->data;
  return DW_INVALID_OFFSET;
}
