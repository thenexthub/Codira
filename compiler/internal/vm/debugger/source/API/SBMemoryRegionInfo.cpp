//===-- SBMemoryRegionInfo.cpp --------------------------------------------===//
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

#include "lldb/API/SBMemoryRegionInfo.h"
#include "Utils.h"
#include "lldb/API/SBDefines.h"
#include "lldb/API/SBError.h"
#include "lldb/API/SBStream.h"
#include "lldb/Target/MemoryRegionInfo.h"
#include "lldb/Utility/Instrumentation.h"
#include "lldb/Utility/StreamString.h"
#include <optional>

using namespace lldb;
using namespace lldb_private;

SBMemoryRegionInfo::SBMemoryRegionInfo() : m_opaque_up(new MemoryRegionInfo()) {
  LLDB_INSTRUMENT_VA(this);
}

SBMemoryRegionInfo::SBMemoryRegionInfo(const char *name, lldb::addr_t begin,
                                       lldb::addr_t end, uint32_t permissions,
                                       bool mapped, bool stack_memory)
    : SBMemoryRegionInfo() {
  LLDB_INSTRUMENT_VA(this, name, begin, end, permissions, mapped, stack_memory);
  m_opaque_up->SetName(name);
  m_opaque_up->GetRange().SetRangeBase(begin);
  m_opaque_up->GetRange().SetRangeEnd(end);
  m_opaque_up->SetLLDBPermissions(permissions);
  m_opaque_up->SetMapped(mapped ? MemoryRegionInfo::eYes
                                : MemoryRegionInfo::eNo);
  m_opaque_up->SetIsStackMemory(stack_memory ? MemoryRegionInfo::eYes
                                             : MemoryRegionInfo::eNo);
}

SBMemoryRegionInfo::SBMemoryRegionInfo(const MemoryRegionInfo *lldb_object_ptr)
    : m_opaque_up(new MemoryRegionInfo()) {
  if (lldb_object_ptr)
    ref() = *lldb_object_ptr;
}

SBMemoryRegionInfo::SBMemoryRegionInfo(const SBMemoryRegionInfo &rhs) {
  LLDB_INSTRUMENT_VA(this, rhs);
  m_opaque_up = clone(rhs.m_opaque_up);
}

const SBMemoryRegionInfo &SBMemoryRegionInfo::
operator=(const SBMemoryRegionInfo &rhs) {
  LLDB_INSTRUMENT_VA(this, rhs);

  if (this != &rhs)
    m_opaque_up = clone(rhs.m_opaque_up);
  return *this;
}

SBMemoryRegionInfo::~SBMemoryRegionInfo() = default;

void SBMemoryRegionInfo::Clear() {
  LLDB_INSTRUMENT_VA(this);

  m_opaque_up->Clear();
}

bool SBMemoryRegionInfo::operator==(const SBMemoryRegionInfo &rhs) const {
  LLDB_INSTRUMENT_VA(this, rhs);

  return ref() == rhs.ref();
}

bool SBMemoryRegionInfo::operator!=(const SBMemoryRegionInfo &rhs) const {
  LLDB_INSTRUMENT_VA(this, rhs);

  return ref() != rhs.ref();
}

MemoryRegionInfo &SBMemoryRegionInfo::ref() { return *m_opaque_up; }

const MemoryRegionInfo &SBMemoryRegionInfo::ref() const { return *m_opaque_up; }

lldb::addr_t SBMemoryRegionInfo::GetRegionBase() {
  LLDB_INSTRUMENT_VA(this);

  return m_opaque_up->GetRange().GetRangeBase();
}

lldb::addr_t SBMemoryRegionInfo::GetRegionEnd() {
  LLDB_INSTRUMENT_VA(this);

  return m_opaque_up->GetRange().GetRangeEnd();
}

bool SBMemoryRegionInfo::IsReadable() {
  LLDB_INSTRUMENT_VA(this);

  return m_opaque_up->GetReadable() == MemoryRegionInfo::eYes;
}

bool SBMemoryRegionInfo::IsWritable() {
  LLDB_INSTRUMENT_VA(this);

  return m_opaque_up->GetWritable() == MemoryRegionInfo::eYes;
}

bool SBMemoryRegionInfo::IsExecutable() {
  LLDB_INSTRUMENT_VA(this);

  return m_opaque_up->GetExecutable() == MemoryRegionInfo::eYes;
}

bool SBMemoryRegionInfo::IsMapped() {
  LLDB_INSTRUMENT_VA(this);

  return m_opaque_up->GetMapped() == MemoryRegionInfo::eYes;
}

const char *SBMemoryRegionInfo::GetName() {
  LLDB_INSTRUMENT_VA(this);

  return m_opaque_up->GetName().AsCString();
}

bool SBMemoryRegionInfo::HasDirtyMemoryPageList() {
  LLDB_INSTRUMENT_VA(this);

  return m_opaque_up->GetDirtyPageList().has_value();
}

uint32_t SBMemoryRegionInfo::GetNumDirtyPages() {
  LLDB_INSTRUMENT_VA(this);

  uint32_t num_dirty_pages = 0;
  const std::optional<std::vector<addr_t>> &dirty_page_list =
      m_opaque_up->GetDirtyPageList();
  if (dirty_page_list)
    num_dirty_pages = dirty_page_list->size();

  return num_dirty_pages;
}

addr_t SBMemoryRegionInfo::GetDirtyPageAddressAtIndex(uint32_t idx) {
  LLDB_INSTRUMENT_VA(this, idx);

  addr_t dirty_page_addr = LLDB_INVALID_ADDRESS;
  const std::optional<std::vector<addr_t>> &dirty_page_list =
      m_opaque_up->GetDirtyPageList();
  if (dirty_page_list && idx < dirty_page_list->size())
    dirty_page_addr = (*dirty_page_list)[idx];

  return dirty_page_addr;
}

int SBMemoryRegionInfo::GetPageSize() {
  LLDB_INSTRUMENT_VA(this);

  return m_opaque_up->GetPageSize();
}

bool SBMemoryRegionInfo::GetDescription(SBStream &description) {
  LLDB_INSTRUMENT_VA(this, description);

  Stream &strm = description.ref();
  const addr_t load_addr = m_opaque_up->GetRange().base;

  strm.Printf("[0x%16.16" PRIx64 "-0x%16.16" PRIx64 " ", load_addr,
              load_addr + m_opaque_up->GetRange().size);
  strm.Printf(m_opaque_up->GetReadable() ? "R" : "-");
  strm.Printf(m_opaque_up->GetWritable() ? "W" : "-");
  strm.Printf(m_opaque_up->GetExecutable() ? "X" : "-");
  strm.Printf("]");

  return true;
}
