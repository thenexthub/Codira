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

#ifndef LLDB_PLUGINS_SCRIPTINTERPRETER_PYTHON_INTERFACES_SCRIPTEDFRAMEPYTHONINTERFACE_H
#define LLDB_PLUGINS_SCRIPTINTERPRETER_PYTHON_INTERFACES_SCRIPTEDFRAMEPYTHONINTERFACE_H

#include "lldb/Host/Config.h"

#if LLDB_ENABLE_PYTHON

#include "ScriptedPythonInterface.h"
#include "lldb/Interpreter/Interfaces/ScriptedFrameInterface.h"
#include <optional>

namespace lldb_private {
class ScriptedFramePythonInterface : public ScriptedFrameInterface,
                                     public ScriptedPythonInterface {
public:
  ScriptedFramePythonInterface(ScriptInterpreterPythonImpl &interpreter);

  llvm::Expected<StructuredData::GenericSP>
  CreatePluginObject(llvm::StringRef class_name, ExecutionContext &exe_ctx,
                     StructuredData::DictionarySP args_sp,
                     StructuredData::Generic *script_obj = nullptr) override;

  llvm::SmallVector<AbstractMethodRequirement>
  GetAbstractMethodRequirements() const override {
    return llvm::SmallVector<AbstractMethodRequirement>({{"get_id"}});
  }

  lldb::user_id_t GetID() override;

  lldb::addr_t GetPC() override;

  std::optional<SymbolContext> GetSymbolContext() override;

  std::optional<std::string> GetFunctionName() override;

  std::optional<std::string> GetDisplayFunctionName() override;

  bool IsInlined() override;

  bool IsArtificial() override;

  bool IsHidden() override;

  StructuredData::DictionarySP GetRegisterInfo() override;

  std::optional<std::string> GetRegisterContext() override;
};
} // namespace lldb_private

#endif // LLDB_ENABLE_PYTHON
#endif // LLDB_PLUGINS_SCRIPTINTERPRETER_PYTHON_INTERFACES_SCRIPTEDFRAMEPYTHONINTERFACE_H
