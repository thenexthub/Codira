//===----------------------------------------------------------------------===//
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

#include "ThreadWasm.h"

#include "ProcessWasm.h"
#include "RegisterContextWasm.h"
#include "UnwindWasm.h"
#include "lldb/Target/Target.h"

using namespace lldb;
using namespace lldb_private;
using namespace lldb_private::wasm;

Unwind &ThreadWasm::GetUnwinder() {
  if (!m_unwinder_up) {
    assert(CalculateTarget()->GetArchitecture().GetMachine() ==
           llvm::Triple::wasm32);
    m_unwinder_up.reset(new wasm::UnwindWasm(*this));
  }
  return *m_unwinder_up;
}

llvm::Expected<std::vector<lldb::addr_t>> ThreadWasm::GetWasmCallStack() {
  if (ProcessSP process_sp = GetProcess()) {
    ProcessWasm *wasm_process = static_cast<ProcessWasm *>(process_sp.get());
    return wasm_process->GetWasmCallStack(GetID());
  }
  return llvm::createStringError("no process");
}

lldb::RegisterContextSP
ThreadWasm::CreateRegisterContextForFrame(StackFrame *frame) {
  uint32_t concrete_frame_idx = 0;
  ProcessSP process_sp(GetProcess());
  ProcessWasm *wasm_process = static_cast<ProcessWasm *>(process_sp.get());

  if (frame)
    concrete_frame_idx = frame->GetConcreteFrameIndex();

  if (concrete_frame_idx == 0)
    return std::make_shared<RegisterContextWasm>(
        *this, concrete_frame_idx, wasm_process->GetRegisterInfo());

  return GetUnwinder().CreateRegisterContextForFrame(frame);
}
