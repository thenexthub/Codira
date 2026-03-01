//===-- InstructionBreakpoint.cpp ------------------------------------*- C++
//-*-===//
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

#include "InstructionBreakpoint.h"
#include "DAP.h"
#include "lldb/API/SBBreakpoint.h"
#include "lldb/API/SBTarget.h"
#include "llvm/ADT/StringRef.h"

namespace lldb_dap {

InstructionBreakpoint::InstructionBreakpoint(
    DAP &d, const protocol::InstructionBreakpoint &breakpoint)
    : Breakpoint(d, breakpoint.condition, breakpoint.hitCondition),
      m_instruction_address_reference(LLDB_INVALID_ADDRESS),
      m_offset(breakpoint.offset.value_or(0)) {
  llvm::StringRef instruction_reference(breakpoint.instructionReference);
  instruction_reference.getAsInteger(0, m_instruction_address_reference);
  m_instruction_address_reference += m_offset;
}

void InstructionBreakpoint::SetBreakpoint() {
  m_bp =
      m_dap.target.BreakpointCreateByAddress(m_instruction_address_reference);
  Breakpoint::SetBreakpoint();
}

} // namespace lldb_dap
