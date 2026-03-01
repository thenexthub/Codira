//===-- SBProcessInfo.cpp -------------------------------------------------===//
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

#include "lldb/API/SBProcessInfo.h"
#include "Utils.h"
#include "lldb/API/SBFileSpec.h"
#include "lldb/Utility/Instrumentation.h"
#include "lldb/Utility/ProcessInfo.h"

using namespace lldb;
using namespace lldb_private;

SBProcessInfo::SBProcessInfo() { LLDB_INSTRUMENT_VA(this); }

SBProcessInfo::SBProcessInfo(const SBProcessInfo &rhs) {
  LLDB_INSTRUMENT_VA(this, rhs);

  m_opaque_up = clone(rhs.m_opaque_up);
}

SBProcessInfo::~SBProcessInfo() = default;

SBProcessInfo &SBProcessInfo::operator=(const SBProcessInfo &rhs) {
  LLDB_INSTRUMENT_VA(this, rhs);

  if (this != &rhs)
    m_opaque_up = clone(rhs.m_opaque_up);
  return *this;
}

ProcessInstanceInfo &SBProcessInfo::ref() {
  if (m_opaque_up == nullptr) {
    m_opaque_up = std::make_unique<ProcessInstanceInfo>();
  }
  return *m_opaque_up;
}

void SBProcessInfo::SetProcessInfo(const ProcessInstanceInfo &proc_info_ref) {
  ref() = proc_info_ref;
}

bool SBProcessInfo::IsValid() const {
  LLDB_INSTRUMENT_VA(this);
  return this->operator bool();
}
SBProcessInfo::operator bool() const {
  LLDB_INSTRUMENT_VA(this);

  return m_opaque_up != nullptr;
}

const char *SBProcessInfo::GetName() {
  LLDB_INSTRUMENT_VA(this);

  if (!m_opaque_up)
    return nullptr;

  return ConstString(m_opaque_up->GetName()).GetCString();
}

SBFileSpec SBProcessInfo::GetExecutableFile() {
  LLDB_INSTRUMENT_VA(this);

  SBFileSpec file_spec;
  if (m_opaque_up) {
    file_spec.SetFileSpec(m_opaque_up->GetExecutableFile());
  }
  return file_spec;
}

lldb::pid_t SBProcessInfo::GetProcessID() {
  LLDB_INSTRUMENT_VA(this);

  lldb::pid_t proc_id = LLDB_INVALID_PROCESS_ID;
  if (m_opaque_up) {
    proc_id = m_opaque_up->GetProcessID();
  }
  return proc_id;
}

uint32_t SBProcessInfo::GetUserID() {
  LLDB_INSTRUMENT_VA(this);

  uint32_t user_id = UINT32_MAX;
  if (m_opaque_up) {
    user_id = m_opaque_up->GetUserID();
  }
  return user_id;
}

uint32_t SBProcessInfo::GetGroupID() {
  LLDB_INSTRUMENT_VA(this);

  uint32_t group_id = UINT32_MAX;
  if (m_opaque_up) {
    group_id = m_opaque_up->GetGroupID();
  }
  return group_id;
}

bool SBProcessInfo::UserIDIsValid() {
  LLDB_INSTRUMENT_VA(this);

  bool is_valid = false;
  if (m_opaque_up) {
    is_valid = m_opaque_up->UserIDIsValid();
  }
  return is_valid;
}

bool SBProcessInfo::GroupIDIsValid() {
  LLDB_INSTRUMENT_VA(this);

  bool is_valid = false;
  if (m_opaque_up) {
    is_valid = m_opaque_up->GroupIDIsValid();
  }
  return is_valid;
}

uint32_t SBProcessInfo::GetEffectiveUserID() {
  LLDB_INSTRUMENT_VA(this);

  uint32_t user_id = UINT32_MAX;
  if (m_opaque_up) {
    user_id = m_opaque_up->GetEffectiveUserID();
  }
  return user_id;
}

uint32_t SBProcessInfo::GetEffectiveGroupID() {
  LLDB_INSTRUMENT_VA(this);

  uint32_t group_id = UINT32_MAX;
  if (m_opaque_up) {
    group_id = m_opaque_up->GetEffectiveGroupID();
  }
  return group_id;
}

bool SBProcessInfo::EffectiveUserIDIsValid() {
  LLDB_INSTRUMENT_VA(this);

  bool is_valid = false;
  if (m_opaque_up) {
    is_valid = m_opaque_up->EffectiveUserIDIsValid();
  }
  return is_valid;
}

bool SBProcessInfo::EffectiveGroupIDIsValid() {
  LLDB_INSTRUMENT_VA(this);

  bool is_valid = false;
  if (m_opaque_up) {
    is_valid = m_opaque_up->EffectiveGroupIDIsValid();
  }
  return is_valid;
}

lldb::pid_t SBProcessInfo::GetParentProcessID() {
  LLDB_INSTRUMENT_VA(this);

  lldb::pid_t proc_id = LLDB_INVALID_PROCESS_ID;
  if (m_opaque_up) {
    proc_id = m_opaque_up->GetParentProcessID();
  }
  return proc_id;
}

const char *SBProcessInfo::GetTriple() {
  LLDB_INSTRUMENT_VA(this);

  if (!m_opaque_up)
    return nullptr;

  const auto &arch = m_opaque_up->GetArchitecture();
  if (!arch.IsValid())
    return nullptr;

  return ConstString(arch.GetTriple().getTriple().c_str()).GetCString();
}
