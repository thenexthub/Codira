//===-- ValueObjectConstResultCast.cpp ------------------------------------===//
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

#include "lldb/ValueObject/ValueObjectConstResultCast.h"

namespace lldb_private {
class DataExtractor;
}
namespace lldb_private {
class Status;
}
namespace lldb_private {
class ValueObject;
}

using namespace lldb_private;

ValueObjectConstResultCast::ValueObjectConstResultCast(
    ValueObject &parent, ConstString name, const CompilerType &cast_type,
    lldb::addr_t live_address)
    : ValueObjectCast(parent, name, cast_type), m_impl(this, live_address) {
  m_name = name;
}

ValueObjectConstResultCast::~ValueObjectConstResultCast() = default;

lldb::ValueObjectSP ValueObjectConstResultCast::Dereference(Status &error) {
  return m_impl.Dereference(error);
}

lldb::ValueObjectSP ValueObjectConstResultCast::GetSyntheticChildAtOffset(
    uint32_t offset, const CompilerType &type, bool can_create,
    ConstString name_const_str) {
  return m_impl.GetSyntheticChildAtOffset(offset, type, can_create,
                                          name_const_str);
}

lldb::ValueObjectSP ValueObjectConstResultCast::AddressOf(Status &error) {
  return m_impl.AddressOf(error);
}

size_t ValueObjectConstResultCast::GetPointeeData(DataExtractor &data,
                                                  uint32_t item_idx,
                                                  uint32_t item_count) {
  return m_impl.GetPointeeData(data, item_idx, item_count);
}

lldb::ValueObjectSP
ValueObjectConstResultCast::DoCast(const CompilerType &compiler_type) {
  return m_impl.Cast(compiler_type);
}
