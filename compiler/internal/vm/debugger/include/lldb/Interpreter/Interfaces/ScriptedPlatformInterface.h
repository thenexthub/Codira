//===-- ScriptedPlatformInterface.h -----------------------------*- C++ -*-===//
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

#ifndef LLDB_INTERPRETER_INTERFACES_SCRIPTEDPLATFORMINTERFACE_H
#define LLDB_INTERPRETER_INTERFACES_SCRIPTEDPLATFORMINTERFACE_H

#include "lldb/Core/StructuredDataImpl.h"
#include "lldb/Interpreter/Interfaces/ScriptedInterface.h"

#include "lldb/lldb-private.h"

#include <string>

namespace lldb_private {
class ScriptedPlatformInterface : virtual public ScriptedInterface {
public:
  virtual llvm::Expected<StructuredData::GenericSP>
  CreatePluginObject(llvm::StringRef class_name, ExecutionContext &exe_ctx,
                     StructuredData::DictionarySP args_sp,
                     StructuredData::Generic *script_obj = nullptr) = 0;

  virtual StructuredData::DictionarySP ListProcesses() { return {}; }

  virtual StructuredData::DictionarySP GetProcessInfo(lldb::pid_t) {
    return {};
  }

  virtual Status AttachToProcess(lldb::ProcessAttachInfoSP attach_info) {
    return Status::FromErrorString(
        "ScriptedPlatformInterface cannot attach to a process");
  }

  virtual Status LaunchProcess(lldb::ProcessLaunchInfoSP launch_info) {
    return Status::FromErrorString(
        "ScriptedPlatformInterface cannot launch process");
  }

  virtual Status KillProcess(lldb::pid_t pid) {
    return Status::FromErrorString(
        "ScriptedPlatformInterface cannot kill process");
  }
};
} // namespace lldb_private

#endif // LLDB_INTERPRETER_INTERFACES_SCRIPTEDPLATFORMINTERFACE_H
