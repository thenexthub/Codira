//===-- OperatingSystemPythonInterface.h ------------------------*- C++ -*-===//
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

#ifndef LLDB_PLUGINS_SCRIPTINTERPRETER_PYTHON_INTERFACES_OPERATINGSYSTEMPYTHONINTERFACE_H
#define LLDB_PLUGINS_SCRIPTINTERPRETER_PYTHON_INTERFACES_OPERATINGSYSTEMPYTHONINTERFACE_H

#include "lldb/Host/Config.h"
#include "lldb/Interpreter/Interfaces/OperatingSystemInterface.h"

#if LLDB_ENABLE_PYTHON

#include "ScriptedThreadPythonInterface.h"

#include <optional>

namespace lldb_private {
class OperatingSystemPythonInterface
    : virtual public OperatingSystemInterface,
      virtual public ScriptedThreadPythonInterface,
      public PluginInterface {
public:
  OperatingSystemPythonInterface(ScriptInterpreterPythonImpl &interpreter);

  llvm::Expected<StructuredData::GenericSP>
  CreatePluginObject(llvm::StringRef class_name, ExecutionContext &exe_ctx,
                     StructuredData::DictionarySP args_sp,
                     StructuredData::Generic *script_obj = nullptr) override;

  llvm::SmallVector<AbstractMethodRequirement>
  GetAbstractMethodRequirements() const override {
    return llvm::SmallVector<AbstractMethodRequirement>({{"get_thread_info"}});
  }

  StructuredData::DictionarySP CreateThread(lldb::tid_t tid,
                                            lldb::addr_t context) override;

  StructuredData::ArraySP GetThreadInfo() override;

  StructuredData::DictionarySP GetRegisterInfo() override;

  std::optional<std::string> GetRegisterContextForTID(lldb::tid_t tid) override;

  std::optional<bool> DoesPluginReportAllThreads() override;

  static void Initialize();

  static void Terminate();

  static llvm::StringRef GetPluginNameStatic() {
    return "OperatingSystemPythonInterface";
  }

  llvm::StringRef GetPluginName() override { return GetPluginNameStatic(); }
};
} // namespace lldb_private

#endif // LLDB_ENABLE_PYTHON
#endif // LLDB_PLUGINS_SCRIPTINTERPRETER_PYTHON_INTERFACES_OPERATINGSYSTEMPYTHONINTERFACE_H
