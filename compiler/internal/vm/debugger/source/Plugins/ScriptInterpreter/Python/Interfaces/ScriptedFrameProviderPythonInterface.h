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

#ifndef LLDB_PLUGINS_SCRIPTINTERPRETER_PYTHON_INTERFACES_SCRIPTEDFRAMEPROVIDERPYTHONINTERFACE_H
#define LLDB_PLUGINS_SCRIPTINTERPRETER_PYTHON_INTERFACES_SCRIPTEDFRAMEPROVIDERPYTHONINTERFACE_H

#include "lldb/Host/Config.h"

#if LLDB_ENABLE_PYTHON

#include "ScriptedPythonInterface.h"
#include "lldb/Core/PluginInterface.h"
#include "lldb/Interpreter/Interfaces/ScriptedFrameProviderInterface.h"
#include <optional>

namespace lldb_private {
class ScriptedFrameProviderPythonInterface
    : public ScriptedFrameProviderInterface,
      public ScriptedPythonInterface,
      public PluginInterface {
public:
  ScriptedFrameProviderPythonInterface(
      ScriptInterpreterPythonImpl &interpreter);

  bool AppliesToThread(llvm::StringRef class_name,
                       lldb::ThreadSP thread_sp) override;

  llvm::Expected<StructuredData::GenericSP>
  CreatePluginObject(llvm::StringRef class_name,
                     lldb::StackFrameListSP input_frames,
                     StructuredData::DictionarySP args_sp) override;

  llvm::SmallVector<AbstractMethodRequirement>
  GetAbstractMethodRequirements() const override {
    return llvm::SmallVector<AbstractMethodRequirement>(
        {{"get_description"}, {"get_frame_at_index"}});
  }

  std::string GetDescription(llvm::StringRef class_name) override;

  std::optional<uint32_t> GetPriority(llvm::StringRef class_name) override;

  StructuredData::ObjectSP GetFrameAtIndex(uint32_t index) override;

  static void Initialize();
  static void Terminate();

  static bool CreateInstance(lldb::ScriptLanguage language,
                             ScriptedInterfaceUsages usages);

  static llvm::StringRef GetPluginNameStatic() {
    return "ScriptedFrameProviderPythonInterface";
  }

  llvm::StringRef GetPluginName() override { return GetPluginNameStatic(); }
};
} // namespace lldb_private

#endif // LLDB_ENABLE_PYTHON
#endif // LLDB_PLUGINS_SCRIPTINTERPRETER_PYTHON_INTERFACES_SCRIPTEDFRAMEPROVIDERPYTHONINTERFACE_H
