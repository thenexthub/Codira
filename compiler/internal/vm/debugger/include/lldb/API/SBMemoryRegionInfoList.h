//===-- SBMemoryRegionInfoList.h --------------------------------*- C++ -*-===//
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

#ifndef LLDB_API_SBMEMORYREGIONINFOLIST_H
#define LLDB_API_SBMEMORYREGIONINFOLIST_H

#include "lldb/API/SBDefines.h"

class MemoryRegionInfoListImpl;

namespace lldb {

class LLDB_API SBMemoryRegionInfoList {
public:
  SBMemoryRegionInfoList();

  SBMemoryRegionInfoList(const lldb::SBMemoryRegionInfoList &rhs);

  const SBMemoryRegionInfoList &operator=(const SBMemoryRegionInfoList &rhs);

  ~SBMemoryRegionInfoList();

  uint32_t GetSize() const;

  bool GetMemoryRegionContainingAddress(lldb::addr_t addr,
                                        SBMemoryRegionInfo &region_info);

  bool GetMemoryRegionAtIndex(uint32_t idx, SBMemoryRegionInfo &region_info);

  void Append(lldb::SBMemoryRegionInfo &region);

  void Append(lldb::SBMemoryRegionInfoList &region_list);

  void Clear();

protected:
  const MemoryRegionInfoListImpl *operator->() const;

  const MemoryRegionInfoListImpl &operator*() const;

private:
  friend class SBProcess;
  friend class SBSaveCoreOptions;

  lldb_private::MemoryRegionInfos &ref();

  const lldb_private::MemoryRegionInfos &ref() const;

  std::unique_ptr<MemoryRegionInfoListImpl> m_opaque_up;
};

} // namespace lldb

#endif // LLDB_API_SBMEMORYREGIONINFOLIST_H
