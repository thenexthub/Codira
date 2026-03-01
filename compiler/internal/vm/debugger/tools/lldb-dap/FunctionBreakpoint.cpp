//===-- FunctionBreakpoint.cpp ----------------------------------*- C++ -*-===//
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

#include "FunctionBreakpoint.h"
#include "DAP.h"
#include "lldb/API/SBMutex.h"
#include <mutex>

namespace lldb_dap {

FunctionBreakpoint::FunctionBreakpoint(
    DAP &d, const protocol::FunctionBreakpoint &breakpoint)
    : Breakpoint(d, breakpoint.condition, breakpoint.hitCondition),
      m_function_name(breakpoint.name) {}

void FunctionBreakpoint::SetBreakpoint() {
  lldb::SBMutex lock = m_dap.GetAPIMutex();
  std::lock_guard<lldb::SBMutex> guard(lock);

  if (m_function_name.empty())
    return;
  m_bp = m_dap.target.BreakpointCreateByName(m_function_name.c_str());
  Breakpoint::SetBreakpoint();
}

} // namespace lldb_dap
