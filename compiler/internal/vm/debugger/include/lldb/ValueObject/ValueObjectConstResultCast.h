//===-- ValueObjectConstResultCast.h ----------------------------*- C++ -*-===//
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

#ifndef LLDB_VALUEOBJECT_VALUEOBJECTCONSTRESULTCAST_H
#define LLDB_VALUEOBJECT_VALUEOBJECTCONSTRESULTCAST_H

#include "lldb/Symbol/CompilerType.h"
#include "lldb/Utility/ConstString.h"
#include "lldb/ValueObject/ValueObjectCast.h"
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

class ValueObjectConstResultCast : public ValueObjectCast {
public:
  ValueObjectConstResultCast(ValueObject &parent, ConstString name,
                             const CompilerType &cast_type,
                             lldb::addr_t live_address = LLDB_INVALID_ADDRESS);

  ~ValueObjectConstResultCast() override;

  lldb::ValueObjectSP Dereference(Status &error) override;

  virtual CompilerType GetCompilerType() {
    return ValueObjectCast::GetCompilerType();
  }

  lldb::ValueObjectSP GetSyntheticChildAtOffset(
      uint32_t offset, const CompilerType &type, bool can_create,
      ConstString name_const_str = ConstString()) override;

  lldb::ValueObjectSP AddressOf(Status &error) override;

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

  ValueObjectConstResultCast(const ValueObjectConstResultCast &) = delete;
  const ValueObjectConstResultCast &
  operator=(const ValueObjectConstResultCast &) = delete;
};

} // namespace lldb_private

#endif // LLDB_VALUEOBJECT_VALUEOBJECTCONSTRESULTCAST_H
