//===-- VMRange.h -----------------------------------------------*- C++ -*-===//
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

#ifndef LLDB_UTILITY_VMRANGE_H
#define LLDB_UTILITY_VMRANGE_H

#include "lldb/lldb-types.h"
#include "llvm/Support/raw_ostream.h"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace lldb_private {

// A vm address range. These can represent offsets ranges or actual
// addresses.
class VMRange {
public:
  typedef std::vector<VMRange> collection;
  typedef collection::iterator iterator;
  typedef collection::const_iterator const_iterator;

  VMRange() = default;

  VMRange(lldb::addr_t start_addr, lldb::addr_t end_addr)
      : m_base_addr(start_addr),
        m_byte_size(end_addr > start_addr ? end_addr - start_addr : 0) {}

  ~VMRange() = default;

  void Clear() {
    m_base_addr = 0;
    m_byte_size = 0;
  }

  // Set the start and end values
  void Reset(lldb::addr_t start_addr, lldb::addr_t end_addr) {
    SetBaseAddress(start_addr);
    SetEndAddress(end_addr);
  }

  // Set the start value for the range, and keep the same size
  void SetBaseAddress(lldb::addr_t base_addr) { m_base_addr = base_addr; }

  void SetEndAddress(lldb::addr_t end_addr) {
    const lldb::addr_t base_addr = GetBaseAddress();
    if (end_addr > base_addr)
      m_byte_size = end_addr - base_addr;
    else
      m_byte_size = 0;
  }

  lldb::addr_t GetByteSize() const { return m_byte_size; }

  void SetByteSize(lldb::addr_t byte_size) { m_byte_size = byte_size; }

  lldb::addr_t GetBaseAddress() const { return m_base_addr; }

  lldb::addr_t GetEndAddress() const { return GetBaseAddress() + m_byte_size; }

  bool IsValid() const { return m_byte_size > 0; }

  bool Contains(lldb::addr_t addr) const {
    return (GetBaseAddress() <= addr) && (addr < GetEndAddress());
  }

  bool Contains(const VMRange &range) const {
    if (Contains(range.GetBaseAddress())) {
      lldb::addr_t range_end = range.GetEndAddress();
      return (GetBaseAddress() <= range_end) && (range_end <= GetEndAddress());
    }
    return false;
  }

  void Dump(llvm::raw_ostream &s, lldb::addr_t base_addr = 0,
            uint32_t addr_width = 8) const;

  static bool ContainsValue(const VMRange::collection &coll,
                            lldb::addr_t value);

  static bool ContainsRange(const VMRange::collection &coll,
                            const VMRange &range);

protected:
  lldb::addr_t m_base_addr = 0;
  lldb::addr_t m_byte_size = 0;
};

bool operator==(const VMRange &lhs, const VMRange &rhs);
bool operator!=(const VMRange &lhs, const VMRange &rhs);
bool operator<(const VMRange &lhs, const VMRange &rhs);
bool operator<=(const VMRange &lhs, const VMRange &rhs);
bool operator>(const VMRange &lhs, const VMRange &rhs);
bool operator>=(const VMRange &lhs, const VMRange &rhs);

} // namespace lldb_private

#endif // LLDB_UTILITY_VMRANGE_H
