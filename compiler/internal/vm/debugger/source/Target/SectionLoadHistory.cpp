//===-- SectionLoadHistory.cpp --------------------------------------------===//
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

#include "lldb/Target/SectionLoadHistory.h"

#include "lldb/Target/SectionLoadList.h"
#include "lldb/Utility/Stream.h"

using namespace lldb;
using namespace lldb_private;

bool SectionLoadHistory::IsEmpty() const {
  std::lock_guard<std::recursive_mutex> guard(m_mutex);
  return m_stop_id_to_section_load_list.empty();
}

void SectionLoadHistory::Clear() {
  std::lock_guard<std::recursive_mutex> guard(m_mutex);
  m_stop_id_to_section_load_list.clear();
}

uint32_t SectionLoadHistory::GetLastStopID() const {
  std::lock_guard<std::recursive_mutex> guard(m_mutex);
  if (m_stop_id_to_section_load_list.empty())
    return 0;
  else
    return m_stop_id_to_section_load_list.rbegin()->first;
}

SectionLoadList *
SectionLoadHistory::GetSectionLoadListForStopID(uint32_t stop_id,
                                                bool read_only) {
  if (!m_stop_id_to_section_load_list.empty()) {
    if (read_only) {
      // The section load list is for reading data only so we don't need to
      // create a new SectionLoadList for the current stop ID, just return the
      // section load list for the stop ID that is equal to or less than the
      // current stop ID
      if (stop_id == eStopIDNow) {
        // If we are asking for the latest and greatest value, it is always at
        // the end of our list because that will be the highest stop ID.
        StopIDToSectionLoadList::reverse_iterator rpos =
            m_stop_id_to_section_load_list.rbegin();
        return rpos->second.get();
      } else {
        StopIDToSectionLoadList::iterator pos =
            m_stop_id_to_section_load_list.lower_bound(stop_id);
        if (pos != m_stop_id_to_section_load_list.end() &&
            pos->first == stop_id)
          return pos->second.get();
        else if (pos != m_stop_id_to_section_load_list.begin()) {
          --pos;
          return pos->second.get();
        }
      }
    } else {
      // You can only use "eStopIDNow" when reading from the section load
      // history
      assert(stop_id != eStopIDNow);

      // We are updating the section load list (not read only), so if the stop
      // ID passed in isn't the same as the last stop ID in our collection,
      // then create a new node using the current stop ID
      StopIDToSectionLoadList::iterator pos =
          m_stop_id_to_section_load_list.lower_bound(stop_id);
      if (pos != m_stop_id_to_section_load_list.end() &&
          pos->first == stop_id) {
        // We already have an entry for this value
        return pos->second.get();
      }

      // We must make a new section load list that is based on the last valid
      // section load list, so here we copy the last section load list and add
      // a new node for the current stop ID.
      StopIDToSectionLoadList::reverse_iterator rpos =
          m_stop_id_to_section_load_list.rbegin();
      SectionLoadListSP section_load_list_sp(
          new SectionLoadList(*rpos->second));
      m_stop_id_to_section_load_list[stop_id] = section_load_list_sp;
      return section_load_list_sp.get();
    }
  }
  SectionLoadListSP section_load_list_sp(new SectionLoadList());
  if (stop_id == eStopIDNow)
    stop_id = 0;
  m_stop_id_to_section_load_list[stop_id] = section_load_list_sp;
  return section_load_list_sp.get();
}

SectionLoadList &SectionLoadHistory::GetCurrentSectionLoadList() {
  const bool read_only = true;
  std::lock_guard<std::recursive_mutex> guard(m_mutex);
  SectionLoadList *section_load_list =
      GetSectionLoadListForStopID(eStopIDNow, read_only);
  assert(section_load_list != nullptr);
  return *section_load_list;
}

addr_t
SectionLoadHistory::GetSectionLoadAddress(uint32_t stop_id,
                                          const lldb::SectionSP &section_sp) {
  std::lock_guard<std::recursive_mutex> guard(m_mutex);
  const bool read_only = true;
  SectionLoadList *section_load_list =
      GetSectionLoadListForStopID(stop_id, read_only);
  return section_load_list->GetSectionLoadAddress(section_sp);
}

bool SectionLoadHistory::ResolveLoadAddress(uint32_t stop_id, addr_t load_addr,
                                            Address &so_addr,
                                            bool allow_section_end) {
  // First find the top level section that this load address exists in
  std::lock_guard<std::recursive_mutex> guard(m_mutex);
  const bool read_only = true;
  SectionLoadList *section_load_list =
      GetSectionLoadListForStopID(stop_id, read_only);
  return section_load_list->ResolveLoadAddress(load_addr, so_addr,
                                               allow_section_end);
}

bool SectionLoadHistory::SetSectionLoadAddress(
    uint32_t stop_id, const lldb::SectionSP &section_sp, addr_t load_addr,
    bool warn_multiple) {
  std::lock_guard<std::recursive_mutex> guard(m_mutex);
  const bool read_only = false;
  SectionLoadList *section_load_list =
      GetSectionLoadListForStopID(stop_id, read_only);
  return section_load_list->SetSectionLoadAddress(section_sp, load_addr,
                                                  warn_multiple);
}

size_t
SectionLoadHistory::SetSectionUnloaded(uint32_t stop_id,
                                       const lldb::SectionSP &section_sp) {
  std::lock_guard<std::recursive_mutex> guard(m_mutex);
  const bool read_only = false;
  SectionLoadList *section_load_list =
      GetSectionLoadListForStopID(stop_id, read_only);
  return section_load_list->SetSectionUnloaded(section_sp);
}

bool SectionLoadHistory::SetSectionUnloaded(uint32_t stop_id,
                                            const lldb::SectionSP &section_sp,
                                            addr_t load_addr) {
  std::lock_guard<std::recursive_mutex> guard(m_mutex);
  const bool read_only = false;
  SectionLoadList *section_load_list =
      GetSectionLoadListForStopID(stop_id, read_only);
  return section_load_list->SetSectionUnloaded(section_sp, load_addr);
}

void SectionLoadHistory::Dump(Stream &s, Target *target) {
  std::lock_guard<std::recursive_mutex> guard(m_mutex);
  StopIDToSectionLoadList::iterator pos,
      end = m_stop_id_to_section_load_list.end();
  for (pos = m_stop_id_to_section_load_list.begin(); pos != end; ++pos) {
    s.Printf("StopID = %u:\n", pos->first);
    pos->second->Dump(s, target);
    s.EOL();
  }
}
