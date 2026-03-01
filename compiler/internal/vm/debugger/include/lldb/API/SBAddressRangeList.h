//===-- SBAddressRangeList.h ------------------------------------*- C++ -*-===//
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

#ifndef LLDB_API_SBADDRESSRANGELIST_H
#define LLDB_API_SBADDRESSRANGELIST_H

#include <memory>

#include "lldb/API/SBDefines.h"

namespace lldb_private {
class AddressRangeListImpl;
}

namespace lldb {

class LLDB_API SBAddressRangeList {
public:
  SBAddressRangeList();

  SBAddressRangeList(const lldb::SBAddressRangeList &rhs);

  ~SBAddressRangeList();

  const lldb::SBAddressRangeList &
  operator=(const lldb::SBAddressRangeList &rhs);

  uint32_t GetSize() const;

  void Clear();

  SBAddressRange GetAddressRangeAtIndex(uint64_t idx);

  void Append(const lldb::SBAddressRange &addr_range);

  void Append(const lldb::SBAddressRangeList &addr_range_list);

  bool GetDescription(lldb::SBStream &description, const SBTarget &target);

private:
  friend class SBBlock;
  friend class SBProcess;
  friend class SBFunction;

  lldb_private::AddressRangeListImpl &ref() const;

  std::unique_ptr<lldb_private::AddressRangeListImpl> m_opaque_up;
};

} // namespace lldb

#endif // LLDB_API_SBADDRESSRANGELIST_H
