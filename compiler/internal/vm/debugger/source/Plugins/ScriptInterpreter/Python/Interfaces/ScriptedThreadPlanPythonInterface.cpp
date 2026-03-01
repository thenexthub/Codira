//===-- ScriptedThreadPlanPythonInterface.cpp -----------------------------===//
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
#include "lldb/Utility/Log.h"
#include "lldb/lldb-enumerations.h"

#if LLDB_ENABLE_PYTHON

// clang-format off
// LLDB Python header must be included first
#include "../lldb-python.h"
//clang-format on

#include "../SWIGPythonBridge.h"
#include "../ScriptInterpreterPythonImpl.h"
#include "ScriptedThreadPlanPythonInterface.h"

using namespace lldb;
using namespace lldb_private;
using namespace lldb_private::python;

ScriptedThreadPlanPythonInterface::ScriptedThreadPlanPythonInterface(
    ScriptInterpreterPythonImpl &interpreter)
    : ScriptedThreadPlanInterface(), ScriptedPythonInterface(interpreter) {}

llvm::Expected<StructuredData::GenericSP>
ScriptedThreadPlanPythonInterface::CreatePluginObject(
    const llvm::StringRef class_name, lldb::ThreadPlanSP thread_plan_sp,
    const StructuredDataImpl &args_sp) {
  return ScriptedPythonInterface::CreatePluginObject(class_name, nullptr,
                                                     thread_plan_sp, args_sp);
}

llvm::Expected<bool>
ScriptedThreadPlanPythonInterface::ExplainsStop(Event *event) {
  Status error;
  StructuredData::ObjectSP obj = Dispatch("explains_stop", error, event);

  if (!ScriptedInterface::CheckStructuredDataObject(LLVM_PRETTY_FUNCTION, obj,
                                                    error)) {
    if (!obj)
      return false;
    return error.ToError();
  }

  return obj->GetBooleanValue();
}

llvm::Expected<bool>
ScriptedThreadPlanPythonInterface::ShouldStop(Event *event) {
  Status error;
  StructuredData::ObjectSP obj = Dispatch("should_stop", error, event);

  if (!ScriptedInterface::CheckStructuredDataObject(LLVM_PRETTY_FUNCTION, obj,
                                                    error)) {
    if (!obj)
      return false;
    return error.ToError();
  }

  return obj->GetBooleanValue();
}

llvm::Expected<bool> ScriptedThreadPlanPythonInterface::IsStale() {
  Status error;
  StructuredData::ObjectSP obj = Dispatch("is_stale", error);

  if (!ScriptedInterface::CheckStructuredDataObject(LLVM_PRETTY_FUNCTION, obj,
                                                    error)) {
    if (!obj)
      return false;
    return error.ToError();
  }

  return obj->GetBooleanValue();
}

lldb::StateType ScriptedThreadPlanPythonInterface::GetRunState() {
  Status error;
  StructuredData::ObjectSP obj = Dispatch("should_step", error);

  if (!ScriptedInterface::CheckStructuredDataObject(LLVM_PRETTY_FUNCTION, obj,
                                                    error))
    return lldb::eStateStepping;

  return static_cast<lldb::StateType>(obj->GetUnsignedIntegerValue(
      static_cast<uint32_t>(lldb::eStateStepping)));
}

llvm::Error
ScriptedThreadPlanPythonInterface::GetStopDescription(lldb::StreamSP &stream) {
  Status error;
  Dispatch("stop_description", error, stream);

  if (error.Fail())
    return error.ToError();

  return llvm::Error::success();
}

void ScriptedThreadPlanPythonInterface::Initialize() {
  const std::vector<llvm::StringRef> ci_usages = {
      "thread step-scripted -C <script-name> [-k key -v value ...]"};
  const std::vector<llvm::StringRef> api_usages = {
      "SBThread.StepUsingScriptedThreadPlan"};
  PluginManager::RegisterPlugin(
      GetPluginNameStatic(),
      llvm::StringRef("Alter thread stepping logic and stop reason"),
      CreateInstance, eScriptLanguagePython, {ci_usages, api_usages});
}

void ScriptedThreadPlanPythonInterface::Terminate() {
  PluginManager::UnregisterPlugin(CreateInstance);
}

#endif
