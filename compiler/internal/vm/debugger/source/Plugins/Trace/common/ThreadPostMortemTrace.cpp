//===-- ThreadPostMortemTrace.cpp -----------------------------------------===//
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

#include "ThreadPostMortemTrace.h"

#include <memory>
#include <optional>

#include "Plugins/Process/Utility/RegisterContextHistory.h"
#include "lldb/Target/Process.h"
#include "lldb/Target/RegisterContext.h"

using namespace lldb;
using namespace lldb_private;
using namespace llvm;

void ThreadPostMortemTrace::RefreshStateAfterStop() {}

RegisterContextSP ThreadPostMortemTrace::GetRegisterContext() {
  if (!m_reg_context_sp)
    m_reg_context_sp = CreateRegisterContextForFrame(nullptr);

  return m_reg_context_sp;
}

RegisterContextSP
ThreadPostMortemTrace::CreateRegisterContextForFrame(StackFrame *frame) {
  // Eventually this will calculate the register context based on the current
  // trace position.
  return std::make_shared<RegisterContextHistory>(
      *this, 0, GetProcess()->GetAddressByteSize(), LLDB_INVALID_ADDRESS);
}

bool ThreadPostMortemTrace::CalculateStopInfo() { return false; }

const std::optional<FileSpec> &ThreadPostMortemTrace::GetTraceFile() const {
  return m_trace_file;
}
