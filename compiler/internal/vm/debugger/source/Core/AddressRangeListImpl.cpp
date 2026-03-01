//===-- AddressRangeListImpl.cpp ------------------------------------------===//
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

#include "lldb/Core/AddressRangeListImpl.h"

using namespace lldb;
using namespace lldb_private;

AddressRangeListImpl::AddressRangeListImpl() : m_ranges() {}

size_t AddressRangeListImpl::GetSize() const { return m_ranges.size(); }

void AddressRangeListImpl::Reserve(size_t capacity) {
  m_ranges.reserve(capacity);
}

void AddressRangeListImpl::Append(const AddressRange &sb_region) {
  m_ranges.emplace_back(sb_region);
}

void AddressRangeListImpl::Append(const AddressRangeListImpl &list) {
  Reserve(GetSize() + list.GetSize());

  for (const auto &range : list.m_ranges)
    Append(range);
}

void AddressRangeListImpl::Clear() { m_ranges.clear(); }

lldb_private::AddressRange
AddressRangeListImpl::GetAddressRangeAtIndex(size_t index) {
  if (index >= GetSize())
    return AddressRange();
  return m_ranges[index];
}

AddressRanges &AddressRangeListImpl::ref() { return m_ranges; }
