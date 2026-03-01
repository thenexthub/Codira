//===-- CFBasicHash.h -------------------------------------------*- C++ -*-===//
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

#ifndef LLDB_SOURCE_PLUGINS_LANGUAGE_OBJC_CFBASICHASH_H
#define LLDB_SOURCE_PLUGINS_LANGUAGE_OBJC_CFBASICHASH_H

#include "lldb/Target/Process.h"
#include "lldb/Target/Target.h"

namespace lldb_private {

class CFBasicHash {
public:
  enum class HashType { set = 0, dict };

  CFBasicHash() = default;
  ~CFBasicHash() = default;

  bool Update(lldb::addr_t addr, ExecutionContextRef exe_ctx_rf);

  bool IsValid() const;

  bool IsMutable() const { return m_mutable; };
  bool IsMultiVariant() const { return m_multi; }
  HashType GetType() const { return m_type; }

  size_t GetCount() const;
  lldb::addr_t GetKeyPointer() const;
  lldb::addr_t GetValuePointer() const;

private:
  template <typename T> struct __CFBasicHash {
    struct RuntimeBase {
      T cfisa;
      T cfinfoa;
    } base;

    struct Bits {
      uint16_t __reserved0;
      uint16_t __reserved1 : 2;
      uint16_t keys_offset : 1;
      uint16_t counts_offset : 2;
      uint16_t counts_width : 2;
      uint16_t __reserved2 : 9;
      uint32_t used_buckets;        // number of used buckets
      uint64_t deleted : 16;        // number of elements deleted
      uint64_t num_buckets_idx : 8; // index to number of buckets
      uint64_t __reserved3 : 40;
      uint64_t __reserved4;
    } bits;

    T pointers[3];
  };
  template <typename T> bool UpdateFor(std::unique_ptr<__CFBasicHash<T>> &m_ht);

  size_t GetPointerCount() const;

  uint32_t m_ptr_size = UINT32_MAX;
  lldb::ByteOrder m_byte_order = lldb::eByteOrderInvalid;
  Address m_address = LLDB_INVALID_ADDRESS;
  std::unique_ptr<__CFBasicHash<uint32_t>> m_ht_32 = nullptr;
  std::unique_ptr<__CFBasicHash<uint64_t>> m_ht_64 = nullptr;
  ExecutionContextRef m_exe_ctx_ref;
  bool m_mutable = true;
  bool m_multi = false;
  HashType m_type = HashType::set;
};

} // namespace lldb_private

#endif // LLDB_SOURCE_PLUGINS_LANGUAGE_OBJC_CFBASICHASH_H
