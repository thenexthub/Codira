//===-- ValueObjectConstResultChild.h ----------------------------*- C++-*-===//
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

#ifndef LLDB_VALUEOBJECT_VALUEOBJECTCONSTRESULTCHILD_H
#define LLDB_VALUEOBJECT_VALUEOBJECTCONSTRESULTCHILD_H

#include "lldb/Symbol/CompilerType.h"
#include "lldb/Utility/ConstString.h"
#include "lldb/ValueObject/ValueObjectChild.h"
#include "lldb/ValueObject/ValueObjectConstResultImpl.h"
#include "lldb/lldb-defines.h"
#include "lldb/lldb-forward.h"
#include "lldb/lldb-types.h"

#include <cstddef>
#include <cstdint>

namespace lldb_private {
class DataExtractor;
class Status;
class ValueObject;

// A child of a ValueObjectConstResult.
class ValueObjectConstResultChild : public ValueObjectChild {
public:
  ValueObjectConstResultChild(
      ValueObject &parent, const CompilerType &compiler_type, ConstString name,
      uint32_t byte_size, int32_t byte_offset, uint32_t bitfield_bit_size,
      uint32_t bitfield_bit_offset, bool is_base_class, bool is_deref_of_parent,
      lldb::addr_t live_address, uint64_t language_flags);

  ~ValueObjectConstResultChild() override;

  lldb::ValueObjectSP Dereference(Status &error) override;

  virtual CompilerType GetCompilerType() {
    return ValueObjectChild::GetCompilerType();
  }

  lldb::ValueObjectSP GetSyntheticChildAtOffset(
      uint32_t offset, const CompilerType &type, bool can_create,
      ConstString name_const_str = ConstString()) override;

  lldb::ValueObjectSP AddressOf(Status &error) override;

  AddrAndType GetAddressOf(bool scalar_is_load_address = true) override;

  size_t GetPointeeData(DataExtractor &data, uint32_t item_idx = 0,
                        uint32_t item_count = 1) override;

  lldb::ValueObjectSP DoCast(const CompilerType &compiler_type) override;

protected:
  ValueObjectConstResultImpl m_impl;

private:
  friend class ValueObject;
  friend class ValueObjectConstResult;
  friend class ValueObjectConstResultImpl;

  ValueObject *CreateChildAtIndex(size_t idx) override {
    return m_impl.CreateChildAtIndex(idx);
  }
  ValueObject *CreateSyntheticArrayMember(size_t idx) override {
    return m_impl.CreateSyntheticArrayMember(idx);
  }

  ValueObjectConstResultChild(const ValueObjectConstResultChild &) = delete;
  const ValueObjectConstResultChild &
  operator=(const ValueObjectConstResultChild &) = delete;
};

} // namespace lldb_private

#endif // LLDB_VALUEOBJECT_VALUEOBJECTCONSTRESULTCHILD_H
