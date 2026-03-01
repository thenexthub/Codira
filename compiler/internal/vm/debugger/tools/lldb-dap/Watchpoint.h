//===-- Watchpoint.h --------------------------------------------*- C++ -*-===//
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

#ifndef LLDB_TOOLS_LLDB_DAP_WATCHPOINT_H
#define LLDB_TOOLS_LLDB_DAP_WATCHPOINT_H

#include "BreakpointBase.h"
#include "DAPForward.h"
#include "Protocol/ProtocolTypes.h"
#include "lldb/API/SBError.h"
#include "lldb/API/SBWatchpoint.h"
#include "lldb/API/SBWatchpointOptions.h"
#include "lldb/lldb-types.h"
#include <cstddef>

namespace lldb_dap {

class Watchpoint : public BreakpointBase {
public:
  Watchpoint(DAP &d, const protocol::DataBreakpoint &breakpoint);
  Watchpoint(DAP &d, lldb::SBWatchpoint wp) : BreakpointBase(d), m_wp(wp) {}

  void SetCondition() override;
  void SetHitCondition() override;

  protocol::Breakpoint ToProtocolBreakpoint() override;

  void SetWatchpoint();

  lldb::addr_t GetAddress() const { return m_addr; }

protected:
  lldb::addr_t m_addr;
  size_t m_size;
  lldb::SBWatchpointOptions m_options;
  /// The LLDB breakpoint associated wit this watchpoint.
  lldb::SBWatchpoint m_wp;
  lldb::SBError m_error;
};
} // namespace lldb_dap

#endif
