//===-- ThreadPlanCallFunctionUsingABI.cpp --------------------------------===//
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

#include "lldb/Target/ThreadPlanCallFunctionUsingABI.h"
#include "lldb/Core/Address.h"
#include "lldb/Target/Process.h"
#include "lldb/Target/RegisterContext.h"
#include "lldb/Target/Target.h"
#include "lldb/Target/Thread.h"
#include "lldb/Utility/Log.h"
#include "lldb/Utility/Stream.h"

using namespace lldb;
using namespace lldb_private;

// ThreadPlanCallFunctionUsingABI: Plan to call a single function using the ABI
// instead of JIT
ThreadPlanCallFunctionUsingABI::ThreadPlanCallFunctionUsingABI(
    Thread &thread, const Address &function, llvm::Type &prototype,
    llvm::Type &return_type, llvm::ArrayRef<ABI::CallArgument> args,
    const EvaluateExpressionOptions &options)
    : ThreadPlanCallFunction(thread, function, options),
      m_return_type(return_type) {
  lldb::addr_t start_load_addr = LLDB_INVALID_ADDRESS;
  lldb::addr_t function_load_addr = LLDB_INVALID_ADDRESS;
  ABI *abi = nullptr;

  if (!ConstructorSetup(thread, abi, start_load_addr, function_load_addr))
    return;

  if (!abi->PrepareTrivialCall(thread, m_function_sp, function_load_addr,
                               start_load_addr, prototype, args))
    return;

  ReportRegisterState("ABI Function call was set up.  Register state was:");

  m_valid = true;
}

ThreadPlanCallFunctionUsingABI::~ThreadPlanCallFunctionUsingABI() = default;

void ThreadPlanCallFunctionUsingABI::GetDescription(Stream *s,
                                                    DescriptionLevel level) {
  if (level == eDescriptionLevelBrief) {
    s->Printf("Function call thread plan using ABI instead of JIT");
  } else {
    s->Printf("Thread plan to call 0x%" PRIx64 " using ABI instead of JIT",
              m_function_addr.GetLoadAddress(&GetTarget()));
  }
}

void ThreadPlanCallFunctionUsingABI::SetReturnValue() {
  const ABI *abi = m_process.GetABI().get();

  // Ask the abi for the return value
  if (abi) {
    const bool persistent = false;
    m_return_valobj_sp =
        abi->GetReturnValueObject(GetThread(), m_return_type, persistent);
  }
}
