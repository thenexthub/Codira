//===-- ValueObjectConstResultChild.cpp -----------------------------------===//
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

#include "lldb/ValueObject/ValueObjectConstResultChild.h"

#include "lldb/lldb-private-enumerations.h"
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

ValueObjectConstResultChild::ValueObjectConstResultChild(
    ValueObject &parent, const CompilerType &compiler_type, ConstString name,
    uint32_t byte_size, int32_t byte_offset, uint32_t bitfield_bit_size,
    uint32_t bitfield_bit_offset, bool is_base_class, bool is_deref_of_parent,
    lldb::addr_t live_address, uint64_t language_flags)
    : ValueObjectChild(parent, compiler_type, name, byte_size, byte_offset,
                       bitfield_bit_size, bitfield_bit_offset, is_base_class,
                       is_deref_of_parent, eAddressTypeLoad, language_flags),
      m_impl(this, live_address) {
  m_name = name;
}

ValueObjectConstResultChild::~ValueObjectConstResultChild() = default;

lldb::ValueObjectSP ValueObjectConstResultChild::Dereference(Status &error) {
  return m_impl.Dereference(error);
}

lldb::ValueObjectSP ValueObjectConstResultChild::GetSyntheticChildAtOffset(
    uint32_t offset, const CompilerType &type, bool can_create,
    ConstString name_const_str) {
  return m_impl.GetSyntheticChildAtOffset(offset, type, can_create,
                                          name_const_str);
}

lldb::ValueObjectSP ValueObjectConstResultChild::AddressOf(Status &error) {
  return m_impl.AddressOf(error);
}

ValueObject::AddrAndType
ValueObjectConstResultChild::GetAddressOf(bool scalar_is_load_address) {
  return m_impl.GetAddressOf(scalar_is_load_address);
}

size_t ValueObjectConstResultChild::GetPointeeData(DataExtractor &data,
                                                   uint32_t item_idx,
                                                   uint32_t item_count) {
  return m_impl.GetPointeeData(data, item_idx, item_count);
}

lldb::ValueObjectSP
ValueObjectConstResultChild::DoCast(const CompilerType &compiler_type) {
  return m_impl.Cast(compiler_type);
}
