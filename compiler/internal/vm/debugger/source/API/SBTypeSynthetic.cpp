//===-- SBTypeSynthetic.cpp -----------------------------------------------===//
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

#include "lldb/API/SBTypeSynthetic.h"
#include "lldb/Utility/Instrumentation.h"

#include "lldb/API/SBStream.h"

#include "lldb/DataFormatters/DataVisualization.h"

using namespace lldb;
using namespace lldb_private;

SBTypeSynthetic::SBTypeSynthetic() { LLDB_INSTRUMENT_VA(this); }

SBTypeSynthetic SBTypeSynthetic::CreateWithClassName(const char *data,
                                                     uint32_t options) {
  LLDB_INSTRUMENT_VA(data, options);

  if (!data || data[0] == 0)
    return SBTypeSynthetic();
  return SBTypeSynthetic(
      std::make_shared<ScriptedSyntheticChildren>(options, data, ""));
}

SBTypeSynthetic SBTypeSynthetic::CreateWithScriptCode(const char *data,
                                                      uint32_t options) {
  LLDB_INSTRUMENT_VA(data, options);

  if (!data || data[0] == 0)
    return SBTypeSynthetic();
  return SBTypeSynthetic(
      std::make_shared<ScriptedSyntheticChildren>(options, "", data));
}

SBTypeSynthetic::SBTypeSynthetic(const lldb::SBTypeSynthetic &rhs)
    : m_opaque_sp(rhs.m_opaque_sp) {
  LLDB_INSTRUMENT_VA(this, rhs);
}

SBTypeSynthetic::~SBTypeSynthetic() = default;

bool SBTypeSynthetic::IsValid() const {
  LLDB_INSTRUMENT_VA(this);
  return this->operator bool();
}
SBTypeSynthetic::operator bool() const {
  LLDB_INSTRUMENT_VA(this);

  return m_opaque_sp.get() != nullptr;
}

bool SBTypeSynthetic::IsClassCode() {
  LLDB_INSTRUMENT_VA(this);

  if (!IsValid())
    return false;
  const char *code = m_opaque_sp->GetPythonCode();
  return (code && *code);
}

bool SBTypeSynthetic::IsClassName() {
  LLDB_INSTRUMENT_VA(this);

  if (!IsValid())
    return false;
  return !IsClassCode();
}

const char *SBTypeSynthetic::GetData() {
  LLDB_INSTRUMENT_VA(this);

  if (!IsValid())
    return nullptr;
  if (IsClassCode())
    return ConstString(m_opaque_sp->GetPythonCode()).GetCString();

  return ConstString(m_opaque_sp->GetPythonClassName()).GetCString();
}

void SBTypeSynthetic::SetClassName(const char *data) {
  LLDB_INSTRUMENT_VA(this, data);

  if (IsValid() && data && *data)
    m_opaque_sp->SetPythonClassName(data);
}

void SBTypeSynthetic::SetClassCode(const char *data) {
  LLDB_INSTRUMENT_VA(this, data);

  if (IsValid() && data && *data)
    m_opaque_sp->SetPythonCode(data);
}

uint32_t SBTypeSynthetic::GetOptions() {
  LLDB_INSTRUMENT_VA(this);

  if (!IsValid())
    return lldb::eTypeOptionNone;
  return m_opaque_sp->GetOptions();
}

void SBTypeSynthetic::SetOptions(uint32_t value) {
  LLDB_INSTRUMENT_VA(this, value);

  if (!CopyOnWrite_Impl())
    return;
  m_opaque_sp->SetOptions(value);
}

bool SBTypeSynthetic::GetDescription(lldb::SBStream &description,
                                     lldb::DescriptionLevel description_level) {
  LLDB_INSTRUMENT_VA(this, description, description_level);

  if (m_opaque_sp) {
    description.Printf("%s\n", m_opaque_sp->GetDescription().c_str());
    return true;
  }
  return false;
}

lldb::SBTypeSynthetic &SBTypeSynthetic::
operator=(const lldb::SBTypeSynthetic &rhs) {
  LLDB_INSTRUMENT_VA(this, rhs);

  if (this != &rhs) {
    m_opaque_sp = rhs.m_opaque_sp;
  }
  return *this;
}

bool SBTypeSynthetic::operator==(lldb::SBTypeSynthetic &rhs) {
  LLDB_INSTRUMENT_VA(this, rhs);

  if (!IsValid())
    return !rhs.IsValid();
  return m_opaque_sp == rhs.m_opaque_sp;
}

bool SBTypeSynthetic::IsEqualTo(lldb::SBTypeSynthetic &rhs) {
  LLDB_INSTRUMENT_VA(this, rhs);

  if (!IsValid())
    return !rhs.IsValid();

  if (m_opaque_sp->IsScripted() != rhs.m_opaque_sp->IsScripted())
    return false;

  if (IsClassCode() != rhs.IsClassCode())
    return false;

  if (strcmp(GetData(), rhs.GetData()))
    return false;

  return GetOptions() == rhs.GetOptions();
}

bool SBTypeSynthetic::operator!=(lldb::SBTypeSynthetic &rhs) {
  LLDB_INSTRUMENT_VA(this, rhs);

  if (!IsValid())
    return !rhs.IsValid();
  return m_opaque_sp != rhs.m_opaque_sp;
}

lldb::ScriptedSyntheticChildrenSP SBTypeSynthetic::GetSP() {
  return m_opaque_sp;
}

void SBTypeSynthetic::SetSP(
    const lldb::ScriptedSyntheticChildrenSP &TypeSynthetic_impl_sp) {
  m_opaque_sp = TypeSynthetic_impl_sp;
}

SBTypeSynthetic::SBTypeSynthetic(
    const lldb::ScriptedSyntheticChildrenSP &TypeSynthetic_impl_sp)
    : m_opaque_sp(TypeSynthetic_impl_sp) {}

bool SBTypeSynthetic::CopyOnWrite_Impl() {
  if (!IsValid())
    return false;
  if (m_opaque_sp.use_count() == 1)
    return true;

  ScriptedSyntheticChildrenSP new_sp(new ScriptedSyntheticChildren(
      m_opaque_sp->GetOptions(), m_opaque_sp->GetPythonClassName(),
      m_opaque_sp->GetPythonCode()));

  SetSP(new_sp);

  return true;
}
