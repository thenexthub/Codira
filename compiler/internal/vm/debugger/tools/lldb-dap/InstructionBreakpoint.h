//===-- InstructionBreakpoint.h --------------------------------------*- C++
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

#ifndef LLDB_TOOLS_LLDB_DAP_INSTRUCTIONBREAKPOINT_H
#define LLDB_TOOLS_LLDB_DAP_INSTRUCTIONBREAKPOINT_H

#include "Breakpoint.h"
#include "DAPForward.h"
#include "Protocol/ProtocolTypes.h"
#include "lldb/lldb-types.h"
#include <cstdint>

namespace lldb_dap {

/// Instruction Breakpoint
class InstructionBreakpoint : public Breakpoint {
public:
  InstructionBreakpoint(DAP &d,
                        const protocol::InstructionBreakpoint &breakpoint);

  /// Set instruction breakpoint in LLDB as a new breakpoint.
  void SetBreakpoint();

  lldb::addr_t GetInstructionAddressReference() const {
    return m_instruction_address_reference;
  }

protected:
  lldb::addr_t m_instruction_address_reference;
  int32_t m_offset;
};

} // namespace lldb_dap

#endif
