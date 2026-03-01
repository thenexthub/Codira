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

#include "lldb/Core/PluginManager.h"
#include "lldb/Host/Config.h"
#include "lldb/Target/Thread.h"
#include "lldb/Utility/Log.h"
#include "lldb/lldb-enumerations.h"

#if LLDB_ENABLE_PYTHON

// LLDB Python header must be included first
#include "../lldb-python.h"

#include "../SWIGPythonBridge.h"
#include "../ScriptInterpreterPythonImpl.h"
#include "ScriptedFrameProviderPythonInterface.h"
#include <optional>

using namespace lldb;
using namespace lldb_private;
using namespace lldb_private::python;
using Locker = ScriptInterpreterPythonImpl::Locker;

ScriptedFrameProviderPythonInterface::ScriptedFrameProviderPythonInterface(
    ScriptInterpreterPythonImpl &interpreter)
    : ScriptedFrameProviderInterface(), ScriptedPythonInterface(interpreter) {}

bool ScriptedFrameProviderPythonInterface::AppliesToThread(
    llvm::StringRef class_name, lldb::ThreadSP thread_sp) {
  // If there is any issue with this method, we will just assume it also applies
  // to this thread which is the default behavior.
  constexpr bool fail_value = true;
  Status error;
  StructuredData::ObjectSP obj =
      CallStaticMethod(class_name, "applies_to_thread", error, thread_sp);
  if (!ScriptedInterface::CheckStructuredDataObject(LLVM_PRETTY_FUNCTION, obj,
                                                    error))
    return fail_value;

  return obj->GetBooleanValue(fail_value);
}

llvm::Expected<StructuredData::GenericSP>
ScriptedFrameProviderPythonInterface::CreatePluginObject(
    const llvm::StringRef class_name, lldb::StackFrameListSP input_frames,
    StructuredData::DictionarySP args_sp) {
  if (!input_frames)
    return llvm::createStringError("invalid frame list");

  StructuredDataImpl sd_impl(args_sp);
  return ScriptedPythonInterface::CreatePluginObject(class_name, nullptr,
                                                     input_frames, sd_impl);
}

std::string ScriptedFrameProviderPythonInterface::GetDescription(
    llvm::StringRef class_name) {
  Status error;
  StructuredData::ObjectSP obj =
      CallStaticMethod(class_name, "get_description", error);
  if (!ScriptedInterface::CheckStructuredDataObject(LLVM_PRETTY_FUNCTION, obj,
                                                    error))
    return {};

  return obj->GetStringValue().str();
}

std::optional<uint32_t>
ScriptedFrameProviderPythonInterface::GetPriority(llvm::StringRef class_name) {
  Status error;
  StructuredData::ObjectSP obj =
      CallStaticMethod(class_name, "get_priority", error);

  if (!ScriptedInterface::CheckStructuredDataObject(LLVM_PRETTY_FUNCTION, obj,
                                                    error))
    return std::nullopt;

  // Try to extract as unsigned integer. Return nullopt if Python returned None
  // or if extraction fails.
  if (StructuredData::UnsignedInteger *int_obj = obj->GetAsUnsignedInteger())
    return static_cast<uint32_t>(int_obj->GetValue());

  return std::nullopt;
}

StructuredData::ObjectSP
ScriptedFrameProviderPythonInterface::GetFrameAtIndex(uint32_t index) {
  Status error;
  StructuredData::ObjectSP obj = Dispatch("get_frame_at_index", error, index);

  if (!ScriptedInterface::CheckStructuredDataObject(LLVM_PRETTY_FUNCTION, obj,
                                                    error))
    return {};

  return obj;
}

bool ScriptedFrameProviderPythonInterface::CreateInstance(
    lldb::ScriptLanguage language, ScriptedInterfaceUsages usages) {
  if (language != eScriptLanguagePython)
    return false;

  return true;
}

void ScriptedFrameProviderPythonInterface::Initialize() {
  const std::vector<llvm::StringRef> ci_usages = {
      "target frame-provider register -C <script-name> [-k key -v value ...]",
      "target frame-provider list",
      "target frame-provider remove <provider-name>",
      "target frame-provider clear"};
  const std::vector<llvm::StringRef> api_usages = {
      "SBTarget.RegisterScriptedFrameProvider",
      "SBTarget.RemoveScriptedFrameProvider",
      "SBTarget.ClearScriptedFrameProvider"};
  PluginManager::RegisterPlugin(
      GetPluginNameStatic(),
      llvm::StringRef("Provide scripted stack frames for threads"),
      CreateInstance, eScriptLanguagePython, {ci_usages, api_usages});
}

void ScriptedFrameProviderPythonInterface::Terminate() {
  PluginManager::UnregisterPlugin(CreateInstance);
}

#endif
