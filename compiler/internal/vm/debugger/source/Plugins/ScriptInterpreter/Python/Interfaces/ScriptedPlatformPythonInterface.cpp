//===-- ScriptedPlatformPythonInterface.cpp -------------------------------===//
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
#include "lldb/Utility/Status.h"
#include "lldb/lldb-enumerations.h"

#if LLDB_ENABLE_PYTHON

// clang-format off
// LLDB Python header must be included first
#include "../lldb-python.h"
//clang-format on

#include "../SWIGPythonBridge.h"
#include "../ScriptInterpreterPythonImpl.h"
#include "ScriptedPlatformPythonInterface.h"

using namespace lldb;
using namespace lldb_private;
using namespace lldb_private::python;
using Locker = ScriptInterpreterPythonImpl::Locker;

ScriptedPlatformPythonInterface::ScriptedPlatformPythonInterface(
    ScriptInterpreterPythonImpl &interpreter)
    : ScriptedPlatformInterface(), ScriptedPythonInterface(interpreter) {}

llvm::Expected<StructuredData::GenericSP>
ScriptedPlatformPythonInterface::CreatePluginObject(
    llvm::StringRef class_name, ExecutionContext &exe_ctx,
    StructuredData::DictionarySP args_sp, StructuredData::Generic *script_obj) {
  ExecutionContextRefSP exe_ctx_ref_sp =
      std::make_shared<ExecutionContextRef>(exe_ctx);
  StructuredDataImpl sd_impl(args_sp);
  return ScriptedPythonInterface::CreatePluginObject(class_name, script_obj,
                                                     exe_ctx_ref_sp, sd_impl);
}

StructuredData::DictionarySP ScriptedPlatformPythonInterface::ListProcesses() {
  Status error;
  StructuredData::DictionarySP dict_sp =
      Dispatch<StructuredData::DictionarySP>("list_processes", error);

  if (!dict_sp || !dict_sp->IsValid() || error.Fail()) {
    return ScriptedInterface::ErrorWithMessage<StructuredData::DictionarySP>(
        LLVM_PRETTY_FUNCTION,
        llvm::Twine("Null or invalid object (" +
                    llvm::Twine(error.AsCString()) + llvm::Twine(")."))
            .str(),
        error);
  }

  return dict_sp;
}

StructuredData::DictionarySP
ScriptedPlatformPythonInterface::GetProcessInfo(lldb::pid_t pid) {
  Status error;
  StructuredData::DictionarySP dict_sp =
      Dispatch<StructuredData::DictionarySP>("get_process_info", error, pid);

  if (!dict_sp || !dict_sp->IsValid() || error.Fail()) {
    return ScriptedInterface::ErrorWithMessage<StructuredData::DictionarySP>(
        LLVM_PRETTY_FUNCTION,
        llvm::Twine("Null or invalid object (" +
                    llvm::Twine(error.AsCString()) + llvm::Twine(")."))
            .str(),
        error);
  }

  return dict_sp;
}

Status ScriptedPlatformPythonInterface::AttachToProcess(
    ProcessAttachInfoSP attach_info) {
  // FIXME: Pass `attach_info` to method call
  return GetStatusFromMethod("attach_to_process");
}

Status ScriptedPlatformPythonInterface::LaunchProcess(
    ProcessLaunchInfoSP launch_info) {
  // FIXME: Pass `launch_info` to method call
  return GetStatusFromMethod("launch_process");
}

Status ScriptedPlatformPythonInterface::KillProcess(lldb::pid_t pid) {
  return GetStatusFromMethod("kill_process", pid);
}

void ScriptedPlatformPythonInterface::Initialize() {
  PluginManager::RegisterPlugin(
      GetPluginNameStatic(), "Mock platform and interact with its processes.",
      CreateInstance, eScriptLanguagePython, {});
}

void ScriptedPlatformPythonInterface::Terminate() {
  PluginManager::UnregisterPlugin(CreateInstance);
}

#endif // LLDB_ENABLE_PYTHON
