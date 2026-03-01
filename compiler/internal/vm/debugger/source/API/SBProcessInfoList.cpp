//===-- SBProcessInfoList.cpp ---------------------------------------------===//
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

#include "lldb/API/SBProcessInfoList.h"
#include "lldb/API/SBProcessInfo.h"
#include "lldb/Utility/Instrumentation.h"
#include "lldb/Utility/ProcessInfo.h"

#include "Utils.h"

using namespace lldb;
using namespace lldb_private;

SBProcessInfoList::SBProcessInfoList() = default;

SBProcessInfoList::~SBProcessInfoList() = default;

SBProcessInfoList::SBProcessInfoList(const ProcessInfoList &impl)
    : m_opaque_up(std::make_unique<ProcessInfoList>(impl)) {
  LLDB_INSTRUMENT_VA(this, impl);
}

SBProcessInfoList::SBProcessInfoList(const lldb::SBProcessInfoList &rhs) {

  LLDB_INSTRUMENT_VA(this, rhs);

  m_opaque_up = clone(rhs.m_opaque_up);
}

const lldb::SBProcessInfoList &
SBProcessInfoList::operator=(const lldb::SBProcessInfoList &rhs) {

  LLDB_INSTRUMENT_VA(this, rhs);

  if (this != &rhs)
    m_opaque_up = clone(rhs.m_opaque_up);
  return *this;
}

uint32_t SBProcessInfoList::GetSize() const {
  LLDB_INSTRUMENT_VA(this);

  if (m_opaque_up)
    return m_opaque_up->GetSize();

  return 0;
}

void SBProcessInfoList::Clear() {
  LLDB_INSTRUMENT_VA(this);

  if (m_opaque_up)
    m_opaque_up->Clear();
}

bool SBProcessInfoList::GetProcessInfoAtIndex(uint32_t idx,
                                              SBProcessInfo &info) {
  LLDB_INSTRUMENT_VA(this, idx, info);

  if (m_opaque_up) {
    lldb_private::ProcessInstanceInfo process_instance_info;
    if (m_opaque_up->GetProcessInfoAtIndex(idx, process_instance_info)) {
      info.SetProcessInfo(process_instance_info);
      return true;
    }
  }

  return false;
}
