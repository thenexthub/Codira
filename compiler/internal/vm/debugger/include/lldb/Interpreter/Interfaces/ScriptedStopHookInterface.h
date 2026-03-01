//===-- ScriptedStopHookInterface.h -----------------------------*- C++ -*-===//
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

#ifndef LLDB_INTERPRETER_INTERFACES_SCRIPTEDSTOPHOOKINTERFACE_H
#define LLDB_INTERPRETER_INTERFACES_SCRIPTEDSTOPHOOKINTERFACE_H

#include "lldb/lldb-private.h"

#include "ScriptedInterface.h"

namespace lldb_private {
class ScriptedStopHookInterface : public ScriptedInterface {
public:
  virtual llvm::Expected<StructuredData::GenericSP>
  CreatePluginObject(llvm::StringRef class_name, lldb::TargetSP target_sp,
                     const StructuredDataImpl &args_sp) = 0;

  /// "handle_stop" will return a bool with the meaning "should_stop"...
  /// If nothing is returned, we'll assume we are going to stop.
  /// Also any errors should return true, since we should stop on error.
  virtual llvm::Expected<bool> HandleStop(ExecutionContext &exe_ctx,
                                          lldb::StreamSP &output_sp) {
    return true;
  }
};
} // namespace lldb_private

#endif // LLDB_INTERPRETER_INTERFACES_SCRIPTEDSTOPHOOKINTERFACE_H
