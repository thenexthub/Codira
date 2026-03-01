//===-- ExecutionContextScope.h ---------------------------------*- C++ -*-===//
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

#ifndef LLDB_TARGET_EXECUTIONCONTEXTSCOPE_H
#define LLDB_TARGET_EXECUTIONCONTEXTSCOPE_H

#include "lldb/lldb-private.h"

namespace lldb_private {

/// @class ExecutionContextScope ExecutionContextScope.h
/// "lldb/Target/ExecutionContextScope.h" Inherit from this if your object can
/// reconstruct its
///        execution context.
///
/// Many objects that have pointers back to parent execution context objects
/// can inherit from this pure virtual class can reconstruct their execution
/// context without having to keep a complete ExecutionContext object in the
/// object state. Examples of these objects include: Process, Thread,
/// RegisterContext and StackFrame.
///
/// Objects can contain a valid pointer to an instance of this so they can
/// reconstruct the execution context.
///
/// Objects that adhere to this protocol can reconstruct enough of a execution
/// context to allow functions that take a execution contexts to be called.
class ExecutionContextScope {
public:
  virtual ~ExecutionContextScope() = default;

  virtual lldb::TargetSP CalculateTarget() = 0;

  virtual lldb::ProcessSP CalculateProcess() = 0;

  virtual lldb::ThreadSP CalculateThread() = 0;

  virtual lldb::StackFrameSP CalculateStackFrame() = 0;

  /// Reconstruct the object's execution context into \a sc.
  ///
  /// The object should fill in as much of the ExecutionContextScope as it can
  /// so function calls that require a execution context can be made for the
  /// given object.
  ///
  /// \param[out] exe_ctx
  ///     A reference to an execution context object that gets filled
  ///     in.
  virtual void CalculateExecutionContext(ExecutionContext &exe_ctx) = 0;
};

} // namespace lldb_private

#endif // LLDB_TARGET_EXECUTIONCONTEXTSCOPE_H
