//===-- ThreadPlanCallFunctionUsingABI.h --------------------------------*- C++
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

#ifndef LLDB_TARGET_THREADPLANCALLFUNCTIONUSINGABI_H
#define LLDB_TARGET_THREADPLANCALLFUNCTIONUSINGABI_H

#include "lldb/Target/ABI.h"
#include "lldb/Target/Thread.h"
#include "lldb/Target/ThreadPlanCallFunction.h"
#include "lldb/lldb-private.h"

#include "llvm/ADT/ArrayRef.h"
#include "llvm/IR/DerivedTypes.h"

namespace lldb_private {

class ThreadPlanCallFunctionUsingABI : public ThreadPlanCallFunction {
  // Create a thread plan to call a function at the address passed in the
  // "function" argument, this function is executed using register manipulation
  // instead of JIT. Class derives from ThreadPlanCallFunction and differs by
  // calling a alternative
  // ABI interface ABI::PrepareTrivialCall() which provides more detailed
  // information.
public:
  ThreadPlanCallFunctionUsingABI(Thread &thread,
                                 const Address &function_address,
                                 llvm::Type &function_prototype,
                                 llvm::Type &return_type,
                                 llvm::ArrayRef<ABI::CallArgument> args,
                                 const EvaluateExpressionOptions &options);

  ~ThreadPlanCallFunctionUsingABI() override;

  void GetDescription(Stream *s, lldb::DescriptionLevel level) override;

protected:
  void SetReturnValue() override;

private:
  llvm::Type &m_return_type;
  ThreadPlanCallFunctionUsingABI(const ThreadPlanCallFunctionUsingABI &) =
      delete;
  const ThreadPlanCallFunctionUsingABI &
  operator=(const ThreadPlanCallFunctionUsingABI &) = delete;
};

} // namespace lldb_private

#endif // LLDB_TARGET_THREADPLANCALLFUNCTIONUSINGABI_H
