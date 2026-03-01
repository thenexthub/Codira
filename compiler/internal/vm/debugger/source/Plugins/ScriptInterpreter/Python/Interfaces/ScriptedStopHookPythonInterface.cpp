//===-- ScriptedStopHookPythonInterface.cpp -------------------------------===//
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

#include "lldb/Core/PluginManager.h"
#include "lldb/Host/Config.h"
#include "lldb/Target/ExecutionContext.h"
#include "lldb/Utility/Log.h"
#include "lldb/lldb-enumerations.h"

#if LLDB_ENABLE_PYTHON

// clang-format off
// LLDB Python header must be included first
#include "../lldb-python.h"
//clang-format on

#include "../SWIGPythonBridge.h"
#include "../ScriptInterpreterPythonImpl.h"
#include "ScriptedStopHookPythonInterface.h"

using namespace lldb;
using namespace lldb_private;
using namespace lldb_private::python;

ScriptedStopHookPythonInterface::ScriptedStopHookPythonInterface(
    ScriptInterpreterPythonImpl &interpreter)
    : ScriptedStopHookInterface(), ScriptedPythonInterface(interpreter) {}

llvm::Expected<StructuredData::GenericSP>
ScriptedStopHookPythonInterface::CreatePluginObject(llvm::StringRef class_name,
                                                    lldb::TargetSP target_sp,
                                                    const StructuredDataImpl &args_sp) {
  return ScriptedPythonInterface::CreatePluginObject(class_name, nullptr,
                                                     target_sp, args_sp);
}

llvm::Expected<bool>
ScriptedStopHookPythonInterface::HandleStop(ExecutionContext &exe_ctx,
                                            lldb::StreamSP& output_sp) {
  ExecutionContextRefSP exe_ctx_ref_sp =
      std::make_shared<ExecutionContextRef>(exe_ctx);
  Status error;
  StructuredData::ObjectSP obj = Dispatch("handle_stop", error, exe_ctx_ref_sp, output_sp);

  if (!ScriptedInterface::CheckStructuredDataObject(LLVM_PRETTY_FUNCTION, obj,
                                                    error)) {
    if (!obj)
      return true;
    return error.ToError();
  }

  return obj->GetBooleanValue();
}


void ScriptedStopHookPythonInterface::Initialize() {
  const std::vector<llvm::StringRef> ci_usages = {
      "target stop-hook add -P <script-name> [-k key -v value ...]"};
  const std::vector<llvm::StringRef> api_usages = {};
  PluginManager::RegisterPlugin(
      GetPluginNameStatic(),
      llvm::StringRef("Perform actions whenever the process stops, before control is returned to the user."),
      CreateInstance, eScriptLanguagePython, {ci_usages, api_usages});
}

void ScriptedStopHookPythonInterface::Terminate() {
  PluginManager::UnregisterPlugin(CreateInstance);
}

#endif
