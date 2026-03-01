//===-- Breakpoint.h --------------------------------------------*- C++ -*-===//
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

#ifndef LLDB_TOOLS_LLDB_DAP_BREAKPOINT_H
#define LLDB_TOOLS_LLDB_DAP_BREAKPOINT_H

#include "BreakpointBase.h"
#include "DAPForward.h"
#include "lldb/API/SBBreakpoint.h"

namespace lldb_dap {

class Breakpoint : public BreakpointBase {
public:
  Breakpoint(DAP &d, const std::optional<std::string> &condition,
             const std::optional<std::string> &hit_condition)
      : BreakpointBase(d, condition, hit_condition) {}
  Breakpoint(DAP &d, lldb::SBBreakpoint bp) : BreakpointBase(d), m_bp(bp) {}

  lldb::break_id_t GetID() const { return m_bp.GetID(); }

  void SetCondition() override;
  void SetHitCondition() override;
  protocol::Breakpoint ToProtocolBreakpoint() override;

  bool MatchesName(const char *name);
  void SetBreakpoint();

protected:
  /// The LLDB breakpoint associated wit this source breakpoint.
  lldb::SBBreakpoint m_bp;
};
} // namespace lldb_dap

#endif
