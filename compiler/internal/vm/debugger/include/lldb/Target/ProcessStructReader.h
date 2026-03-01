//===---------------------ProcessStructReader.h ------------------*- C++-*-===//
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

#ifndef LLDB_TARGET_PROCESSSTRUCTREADER_H
#define LLDB_TARGET_PROCESSSTRUCTREADER_H

#include "lldb/lldb-defines.h"
#include "lldb/lldb-types.h"

#include "lldb/Symbol/CompilerType.h"
#include "lldb/Target/Process.h"
#include "lldb/Utility/DataBufferHeap.h"
#include "lldb/Utility/DataExtractor.h"
#include "lldb/Utility/Status.h"

#include "llvm/ADT/StringMap.h"

#include <initializer_list>
#include <map>
#include <string>

namespace lldb_private {
class ProcessStructReader {
protected:
  struct FieldImpl {
    CompilerType type;
    size_t offset;
    size_t size;
  };

  llvm::StringMap<FieldImpl> m_fields;
  DataExtractor m_data;
  lldb::ByteOrder m_byte_order;
  size_t m_addr_byte_size;

public:
  ProcessStructReader(Process *process, lldb::addr_t base_addr,
                      CompilerType struct_type)
      : m_byte_order(lldb::eByteOrderInvalid), m_addr_byte_size(0) {
    if (!process)
      return;
    if (base_addr == 0 || base_addr == LLDB_INVALID_ADDRESS)
      return;
    m_byte_order = process->GetByteOrder();
    m_addr_byte_size = process->GetAddressByteSize();

    for (size_t idx = 0; idx < struct_type.GetNumFields(); idx++) {
      std::string name;
      uint64_t bit_offset;
      uint32_t bitfield_bit_size;
      bool is_bitfield;
      CompilerType field_type = struct_type.GetFieldAtIndex(
          idx, name, &bit_offset, &bitfield_bit_size, &is_bitfield);
      // no support for bitfields in here (yet)
      if (is_bitfield)
        return;
      auto size = field_type.GetByteSize(nullptr);
      // no support for things larger than a uint64_t (yet)
      if (!size || *size > 8)
        return;
      size_t byte_index = static_cast<size_t>(bit_offset / 8);
      m_fields.insert({name, FieldImpl{field_type, byte_index,
                                       static_cast<size_t>(*size)}});
    }
    auto total_size = struct_type.GetByteSize(nullptr);
    if (!total_size)
      return;
    lldb::WritableDataBufferSP buffer_sp(new DataBufferHeap(*total_size, 0));
    Status error;
    process->ReadMemoryFromInferior(base_addr, buffer_sp->GetBytes(),
                                    *total_size, error);
    if (error.Fail())
      return;
    m_data = DataExtractor(buffer_sp, m_byte_order, m_addr_byte_size);
  }

  template <typename RetType>
  RetType GetField(llvm::StringRef name, RetType fail_value = RetType()) {
    auto iter = m_fields.find(name), end = m_fields.end();
    if (iter == end)
      return fail_value;
    auto size = iter->second.size;
    if (sizeof(RetType) < size)
      return fail_value;
    lldb::offset_t offset = iter->second.offset;
    if (offset + size > m_data.GetByteSize())
      return fail_value;
    return (RetType)(m_data.GetMaxU64(&offset, size));
  }
};
}

#endif // LLDB_TARGET_PROCESSSTRUCTREADER_H
