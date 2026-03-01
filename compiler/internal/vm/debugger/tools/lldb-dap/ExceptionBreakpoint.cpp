//===-- ExceptionBreakpoint.cpp ---------------------------------*- C++ -*-===//
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

#include "ExceptionBreakpoint.h"
#include "BreakpointBase.h"
#include "DAP.h"
#include "Protocol/ProtocolTypes.h"
#include "lldb/API/SBMutex.h"
#include "lldb/API/SBTarget.h"
#include <mutex>

using namespace llvm;
using namespace lldb_dap::protocol;

namespace lldb_dap {

protocol::Breakpoint ExceptionBreakpoint::SetBreakpoint(StringRef condition) {
  lldb::SBMutex lock = m_dap.GetAPIMutex();
  std::lock_guard<lldb::SBMutex> guard(lock);

  if (!m_bp.IsValid()) {
    m_bp = m_dap.target.BreakpointCreateForException(
        m_language, m_kind == eExceptionKindCatch,
        m_kind == eExceptionKindThrow);
    m_bp.AddName(BreakpointBase::kDAPBreakpointLabel);
  }

  m_bp.SetCondition(condition.data());

  protocol::Breakpoint breakpoint;
  breakpoint.id = m_bp.GetID();
  breakpoint.verified = m_bp.IsValid();
  return breakpoint;
}

void ExceptionBreakpoint::ClearBreakpoint() {
  if (!m_bp.IsValid())
    return;
  m_dap.target.BreakpointDelete(m_bp.GetID());
  m_bp = lldb::SBBreakpoint();
}

} // namespace lldb_dap
