//===-- SBWatchpointOptions.cpp -------------------------------------------===//
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

#include "lldb/API/SBWatchpointOptions.h"
#include "lldb/Breakpoint/Watchpoint.h"
#include "lldb/Utility/Instrumentation.h"

#include "Utils.h"

using namespace lldb;
using namespace lldb_private;

class WatchpointOptionsImpl {
public:
  bool m_read = false;
  bool m_write = false;
  bool m_modify = false;
};


SBWatchpointOptions::SBWatchpointOptions()
    : m_opaque_up(new WatchpointOptionsImpl()) {
  LLDB_INSTRUMENT_VA(this);
}

SBWatchpointOptions::SBWatchpointOptions(const SBWatchpointOptions &rhs) {
  LLDB_INSTRUMENT_VA(this, rhs);

  m_opaque_up = clone(rhs.m_opaque_up);
}

const SBWatchpointOptions &
SBWatchpointOptions::operator=(const SBWatchpointOptions &rhs) {
  LLDB_INSTRUMENT_VA(this, rhs);

  if (this != &rhs)
    m_opaque_up = clone(rhs.m_opaque_up);
  return *this;
}

SBWatchpointOptions::~SBWatchpointOptions() = default;

void SBWatchpointOptions::SetWatchpointTypeRead(bool read) {
  m_opaque_up->m_read = read;
}
bool SBWatchpointOptions::GetWatchpointTypeRead() const {
  return m_opaque_up->m_read;
}

void SBWatchpointOptions::SetWatchpointTypeWrite(
    WatchpointWriteType write_type) {
  if (write_type == eWatchpointWriteTypeOnModify) {
    m_opaque_up->m_write = false;
    m_opaque_up->m_modify = true;
  } else if (write_type == eWatchpointWriteTypeAlways) {
    m_opaque_up->m_write = true;
    m_opaque_up->m_modify = false;
  } else
    m_opaque_up->m_write = m_opaque_up->m_modify = false;
}

WatchpointWriteType SBWatchpointOptions::GetWatchpointTypeWrite() const {
  if (m_opaque_up->m_modify)
    return eWatchpointWriteTypeOnModify;
  if (m_opaque_up->m_write)
    return eWatchpointWriteTypeAlways;
  return eWatchpointWriteTypeDisabled;
}
