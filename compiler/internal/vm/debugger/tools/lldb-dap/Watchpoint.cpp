//===-- Watchpoint.cpp ------------------------------------------*- C++ -*-===//
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

#include "Watchpoint.h"
#include "DAP.h"
#include "Protocol/ProtocolTypes.h"
#include "lldb/API/SBTarget.h"
#include "lldb/lldb-enumerations.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/ADT/StringRef.h"
#include <cstdint>
#include <string>

namespace lldb_dap {
Watchpoint::Watchpoint(DAP &d, const protocol::DataBreakpoint &breakpoint)
    : BreakpointBase(d, breakpoint.condition, breakpoint.hitCondition) {
  llvm::StringRef dataId = breakpoint.dataId;
  auto [addr_str, size_str] = dataId.split('/');
  llvm::to_integer(addr_str, m_addr, 16);
  llvm::to_integer(size_str, m_size);
  m_options.SetWatchpointTypeRead(breakpoint.accessType !=
                                  protocol::eDataBreakpointAccessTypeWrite);
  if (breakpoint.accessType != protocol::eDataBreakpointAccessTypeRead)
    m_options.SetWatchpointTypeWrite(lldb::eWatchpointWriteTypeOnModify);
}

void Watchpoint::SetCondition() { m_wp.SetCondition(m_condition.c_str()); }

void Watchpoint::SetHitCondition() {
  uint64_t hitCount = 0;
  if (llvm::to_integer(m_hit_condition, hitCount))
    m_wp.SetIgnoreCount(hitCount - 1);
}

protocol::Breakpoint Watchpoint::ToProtocolBreakpoint() {
  protocol::Breakpoint breakpoint;
  if (!m_error.IsValid() || m_error.Fail()) {
    breakpoint.verified = false;
    if (m_error.Fail())
      breakpoint.message = m_error.GetCString();
  } else {
    breakpoint.verified = true;
    breakpoint.id = m_wp.GetID();
  }

  return breakpoint;
}

void Watchpoint::SetWatchpoint() {
  m_wp = m_dap.target.WatchpointCreateByAddress(m_addr, m_size, m_options,
                                                m_error);
  if (!m_condition.empty())
    SetCondition();
  if (!m_hit_condition.empty())
    SetHitCondition();
}
} // namespace lldb_dap
