//===-- FunctionBreakpoint.h ------------------------------------*- C++ -*-===//
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

#ifndef LLDB_TOOLS_LLDB_DAP_FUNCTIONBREAKPOINT_H
#define LLDB_TOOLS_LLDB_DAP_FUNCTIONBREAKPOINT_H

#include "Breakpoint.h"
#include "DAPForward.h"
#include "Protocol/ProtocolTypes.h"

namespace lldb_dap {

class FunctionBreakpoint : public Breakpoint {
public:
  FunctionBreakpoint(DAP &dap, const protocol::FunctionBreakpoint &breakpoint);

  /// Set this breakpoint in LLDB as a new breakpoint.
  void SetBreakpoint();

  llvm::StringRef GetFunctionName() const { return m_function_name; }

protected:
  std::string m_function_name;
};

} // namespace lldb_dap

#endif
