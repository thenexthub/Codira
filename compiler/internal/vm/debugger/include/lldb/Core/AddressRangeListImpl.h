//===-- AddressRangeListImpl.h ----------------------------------*- C++ -*-===//
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

#ifndef LLDB_CORE_ADDRESSRANGELISTIMPL_H
#define LLDB_CORE_ADDRESSRANGELISTIMPL_H

#include "lldb/Core/AddressRange.h"
#include <cstddef>

namespace lldb {
class SBAddressRangeList;
class SBBlock;
class SBProcess;
}

namespace lldb_private {

class AddressRangeListImpl {
public:
  AddressRangeListImpl();

  explicit AddressRangeListImpl(AddressRanges ranges)
      : m_ranges(std::move(ranges)) {}

  size_t GetSize() const;

  void Reserve(size_t capacity);

  void Append(const AddressRange &sb_region);

  void Append(const AddressRangeListImpl &list);

  void Clear();

  lldb_private::AddressRange GetAddressRangeAtIndex(size_t index);

private:
  friend class lldb::SBAddressRangeList;
  friend class lldb::SBBlock;
  friend class lldb::SBProcess;

  AddressRanges &ref();

  AddressRanges m_ranges;
};

} // namespace lldb_private

#endif // LLDB_CORE_ADDRESSRANGE_H
