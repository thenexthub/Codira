//===-- ScriptedPlatformPythonInterface.h -----------------------*- C++ -*-===//
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

#ifndef LLDB_PLUGINS_SCRIPTINTERPRETER_PYTHON_INTERFACES_SCRIPTEDPLATFORMPYTHONINTERFACE_H
#define LLDB_PLUGINS_SCRIPTINTERPRETER_PYTHON_INTERFACES_SCRIPTEDPLATFORMPYTHONINTERFACE_H

#include "lldb/Host/Config.h"
#include "lldb/Interpreter/Interfaces/ScriptedPlatformInterface.h"

#if LLDB_ENABLE_PYTHON

#include "ScriptedPythonInterface.h"

namespace lldb_private {
class ScriptedPlatformPythonInterface : public ScriptedPlatformInterface,
                                        public ScriptedPythonInterface,
                                        public PluginInterface {
public:
  ScriptedPlatformPythonInterface(ScriptInterpreterPythonImpl &interpreter);

  llvm::Expected<StructuredData::GenericSP>
  CreatePluginObject(const llvm::StringRef class_name,
                     ExecutionContext &exe_ctx,
                     StructuredData::DictionarySP args_sp,
                     StructuredData::Generic *script_obj = nullptr) override;

  llvm::SmallVector<AbstractMethodRequirement>
  GetAbstractMethodRequirements() const override {
    return llvm::SmallVector<AbstractMethodRequirement>(
        {{"list_processes"},
         {"attach_to_process", 2},
         {"launch_process", 2},
         {"kill_process", 2}});
  }

  StructuredData::DictionarySP ListProcesses() override;

  StructuredData::DictionarySP GetProcessInfo(lldb::pid_t) override;

  Status AttachToProcess(lldb::ProcessAttachInfoSP attach_info) override;

  Status LaunchProcess(lldb::ProcessLaunchInfoSP launch_info) override;

  Status KillProcess(lldb::pid_t pid) override;

  static void Initialize();

  static void Terminate();

  static llvm::StringRef GetPluginNameStatic() {
    return "ScriptedPlatformPythonInterface";
  }

  llvm::StringRef GetPluginName() override { return GetPluginNameStatic(); }
};
} // namespace lldb_private

#endif // LLDB_ENABLE_PYTHON
#endif // LLDB_PLUGINS_SCRIPTINTERPRETER_PYTHON_INTERFACES_SCRIPTEDPLATFORMPYTHONINTERFACE_H
