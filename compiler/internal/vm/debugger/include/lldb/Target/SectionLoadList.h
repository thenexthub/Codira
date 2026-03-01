//===-- SectionLoadList.h -----------------------------------------------*- C++
//-*-===//
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

#ifndef LLDB_TARGET_SECTIONLOADLIST_H
#define LLDB_TARGET_SECTIONLOADLIST_H

#include <map>
#include <mutex>

#include "llvm/ADT/DenseMap.h"
#include "lldb/Core/Section.h"
#include "lldb/lldb-public.h"

namespace lldb_private {

class SectionLoadList {
public:
  // Constructors and Destructors
  SectionLoadList() = default;

  SectionLoadList(const SectionLoadList &rhs);

  ~SectionLoadList() {
    // Call clear since this takes a lock and clears the section load list in
    // case another thread is currently using this section load list
    Clear();
  }

  void operator=(const SectionLoadList &rhs);

  bool IsEmpty() const;

  void Clear();

  lldb::addr_t GetSectionLoadAddress(const lldb::SectionSP &section_sp) const;

  bool ResolveLoadAddress(lldb::addr_t load_addr, Address &so_addr,
                          bool allow_section_end = false) const;

  bool SetSectionLoadAddress(const lldb::SectionSP &section_sp,
                             lldb::addr_t load_addr,
                             bool warn_multiple = false);

  // The old load address should be specified when unloading to ensure we get
  // the correct instance of the section as a shared library could be loaded at
  // more than one location.
  bool SetSectionUnloaded(const lldb::SectionSP &section_sp,
                          lldb::addr_t load_addr);

  // Unload all instances of a section. This function can be used on systems
  // that don't support multiple copies of the same shared library to be loaded
  // at the same time.
  size_t SetSectionUnloaded(const lldb::SectionSP &section_sp);

  void Dump(Stream &s, Target *target);

protected:
  typedef std::map<lldb::addr_t, lldb::SectionSP> addr_to_sect_collection;
  typedef llvm::DenseMap<const Section *, lldb::addr_t> sect_to_addr_collection;
  addr_to_sect_collection m_addr_to_sect;
  sect_to_addr_collection m_sect_to_addr;
  mutable std::recursive_mutex m_mutex;
};

} // namespace lldb_private

#endif // LLDB_TARGET_SECTIONLOADLIST_H
