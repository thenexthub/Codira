//===-- SBTypeEnumMember.cpp ----------------------------------------------===//
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

#include "lldb/API/SBTypeEnumMember.h"
#include "Utils.h"
#include "lldb/API/SBDefines.h"
#include "lldb/API/SBStream.h"
#include "lldb/API/SBType.h"
#include "lldb/Symbol/CompilerType.h"
#include "lldb/Symbol/Type.h"
#include "lldb/Utility/Instrumentation.h"
#include "lldb/Utility/Stream.h"

#include <memory>

using namespace lldb;
using namespace lldb_private;

SBTypeEnumMember::SBTypeEnumMember() { LLDB_INSTRUMENT_VA(this); }

SBTypeEnumMember::~SBTypeEnumMember() = default;

SBTypeEnumMember::SBTypeEnumMember(
    const lldb::TypeEnumMemberImplSP &enum_member_sp)
    : m_opaque_sp(enum_member_sp) {}

SBTypeEnumMember::SBTypeEnumMember(const SBTypeEnumMember &rhs) {
  LLDB_INSTRUMENT_VA(this, rhs);

  m_opaque_sp = clone(rhs.m_opaque_sp);
}

SBTypeEnumMember &SBTypeEnumMember::operator=(const SBTypeEnumMember &rhs) {
  LLDB_INSTRUMENT_VA(this, rhs);

  if (this != &rhs)
    m_opaque_sp = clone(rhs.m_opaque_sp);
  return *this;
}

bool SBTypeEnumMember::IsValid() const {
  LLDB_INSTRUMENT_VA(this);
  return this->operator bool();
}
SBTypeEnumMember::operator bool() const {
  LLDB_INSTRUMENT_VA(this);

  return m_opaque_sp.get();
}

const char *SBTypeEnumMember::GetName() {
  LLDB_INSTRUMENT_VA(this);

  if (m_opaque_sp.get())
    return m_opaque_sp->GetName().GetCString();
  return nullptr;
}

int64_t SBTypeEnumMember::GetValueAsSigned() {
  LLDB_INSTRUMENT_VA(this);

  if (m_opaque_sp.get())
    return m_opaque_sp->GetValueAsSigned();
  return 0;
}

uint64_t SBTypeEnumMember::GetValueAsUnsigned() {
  LLDB_INSTRUMENT_VA(this);

  if (m_opaque_sp.get())
    return m_opaque_sp->GetValueAsUnsigned();
  return 0;
}

SBType SBTypeEnumMember::GetType() {
  LLDB_INSTRUMENT_VA(this);

  SBType sb_type;
  if (m_opaque_sp.get()) {
    sb_type.SetSP(m_opaque_sp->GetIntegerType());
  }
  return sb_type;
}

void SBTypeEnumMember::reset(TypeEnumMemberImpl *type_member_impl) {
  m_opaque_sp.reset(type_member_impl);
}

TypeEnumMemberImpl &SBTypeEnumMember::ref() {
  if (m_opaque_sp.get() == nullptr)
    m_opaque_sp = std::make_shared<TypeEnumMemberImpl>();
  return *m_opaque_sp.get();
}

const TypeEnumMemberImpl &SBTypeEnumMember::ref() const {
  return *m_opaque_sp.get();
}

SBTypeEnumMemberList::SBTypeEnumMemberList()
    : m_opaque_up(new TypeEnumMemberListImpl()) {
  LLDB_INSTRUMENT_VA(this);
}

SBTypeEnumMemberList::SBTypeEnumMemberList(const SBTypeEnumMemberList &rhs)
    : m_opaque_up(new TypeEnumMemberListImpl()) {
  LLDB_INSTRUMENT_VA(this, rhs);

  for (uint32_t i = 0,
                rhs_size = const_cast<SBTypeEnumMemberList &>(rhs).GetSize();
       i < rhs_size; i++)
    Append(const_cast<SBTypeEnumMemberList &>(rhs).GetTypeEnumMemberAtIndex(i));
}

bool SBTypeEnumMemberList::IsValid() {
  LLDB_INSTRUMENT_VA(this);
  return this->operator bool();
}
SBTypeEnumMemberList::operator bool() const {
  LLDB_INSTRUMENT_VA(this);

  return (m_opaque_up != nullptr);
}

SBTypeEnumMemberList &SBTypeEnumMemberList::
operator=(const SBTypeEnumMemberList &rhs) {
  LLDB_INSTRUMENT_VA(this, rhs);

  if (this != &rhs) {
    m_opaque_up = std::make_unique<TypeEnumMemberListImpl>();
    for (uint32_t i = 0,
                  rhs_size = const_cast<SBTypeEnumMemberList &>(rhs).GetSize();
         i < rhs_size; i++)
      Append(
          const_cast<SBTypeEnumMemberList &>(rhs).GetTypeEnumMemberAtIndex(i));
  }
  return *this;
}

void SBTypeEnumMemberList::Append(SBTypeEnumMember enum_member) {
  LLDB_INSTRUMENT_VA(this, enum_member);

  if (enum_member.IsValid())
    m_opaque_up->Append(enum_member.m_opaque_sp);
}

SBTypeEnumMember
SBTypeEnumMemberList::GetTypeEnumMemberAtIndex(uint32_t index) {
  LLDB_INSTRUMENT_VA(this, index);

  if (m_opaque_up)
    return SBTypeEnumMember(m_opaque_up->GetTypeEnumMemberAtIndex(index));
  return SBTypeEnumMember();
}

uint32_t SBTypeEnumMemberList::GetSize() {
  LLDB_INSTRUMENT_VA(this);

  return m_opaque_up->GetSize();
}

SBTypeEnumMemberList::~SBTypeEnumMemberList() = default;

bool SBTypeEnumMember::GetDescription(
    lldb::SBStream &description, lldb::DescriptionLevel description_level) {
  LLDB_INSTRUMENT_VA(this, description, description_level);

  Stream &strm = description.ref();

  if (m_opaque_sp.get()) {
    if (m_opaque_sp->GetIntegerType()->GetDescription(strm,
                                                      description_level)) {
      strm.Printf(" %s", m_opaque_sp->GetName().GetCString());
    }
  } else {
    strm.PutCString("No value");
  }
  return true;
}
