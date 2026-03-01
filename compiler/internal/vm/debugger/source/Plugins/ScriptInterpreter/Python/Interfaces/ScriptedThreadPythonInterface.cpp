//===-- ScriptedThreadPythonInterface.cpp ---------------------------------===//
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

#include "lldb/Host/Config.h"
#include "lldb/Target/ExecutionContext.h"
#include "lldb/Utility/Log.h"
#include "lldb/lldb-enumerations.h"

#if LLDB_ENABLE_PYTHON

// LLDB Python header must be included first
#include "../lldb-python.h"

#include "../SWIGPythonBridge.h"
#include "../ScriptInterpreterPythonImpl.h"
#include "ScriptedThreadPythonInterface.h"
#include <optional>

using namespace lldb;
using namespace lldb_private;
using namespace lldb_private::python;
using Locker = ScriptInterpreterPythonImpl::Locker;

ScriptedThreadPythonInterface::ScriptedThreadPythonInterface(
    ScriptInterpreterPythonImpl &interpreter)
    : ScriptedThreadInterface(), ScriptedPythonInterface(interpreter) {}

llvm::Expected<StructuredData::GenericSP>
ScriptedThreadPythonInterface::CreatePluginObject(
    const llvm::StringRef class_name, ExecutionContext &exe_ctx,
    StructuredData::DictionarySP args_sp, StructuredData::Generic *script_obj) {
  ExecutionContextRefSP exe_ctx_ref_sp =
      std::make_shared<ExecutionContextRef>(exe_ctx);
  StructuredDataImpl sd_impl(args_sp);
  return ScriptedPythonInterface::CreatePluginObject(class_name, script_obj,
                                                     exe_ctx_ref_sp, sd_impl);
}

lldb::tid_t ScriptedThreadPythonInterface::GetThreadID() {
  Status error;
  StructuredData::ObjectSP obj = Dispatch("get_thread_id", error);

  if (!ScriptedInterface::CheckStructuredDataObject(LLVM_PRETTY_FUNCTION, obj,
                                                    error))
    return LLDB_INVALID_THREAD_ID;

  return obj->GetUnsignedIntegerValue(LLDB_INVALID_THREAD_ID);
}

std::optional<std::string> ScriptedThreadPythonInterface::GetName() {
  Status error;
  StructuredData::ObjectSP obj = Dispatch("get_name", error);

  if (!ScriptedInterface::CheckStructuredDataObject(LLVM_PRETTY_FUNCTION, obj,
                                                    error))
    return {};

  return obj->GetStringValue().str();
}

lldb::StateType ScriptedThreadPythonInterface::GetState() {
  Status error;
  StructuredData::ObjectSP obj = Dispatch("get_state", error);

  if (!ScriptedInterface::CheckStructuredDataObject(LLVM_PRETTY_FUNCTION, obj,
                                                    error))
    return eStateInvalid;

  return static_cast<StateType>(obj->GetUnsignedIntegerValue(eStateInvalid));
}

std::optional<std::string> ScriptedThreadPythonInterface::GetQueue() {
  Status error;
  StructuredData::ObjectSP obj = Dispatch("get_queue", error);

  if (!ScriptedInterface::CheckStructuredDataObject(LLVM_PRETTY_FUNCTION, obj,
                                                    error))
    return {};

  return obj->GetStringValue().str();
}

StructuredData::DictionarySP ScriptedThreadPythonInterface::GetStopReason() {
  Status error;
  StructuredData::DictionarySP dict =
      Dispatch<StructuredData::DictionarySP>("get_stop_reason", error);

  if (!ScriptedInterface::CheckStructuredDataObject(LLVM_PRETTY_FUNCTION, dict,
                                                    error))
    return {};

  return dict;
}

StructuredData::ArraySP ScriptedThreadPythonInterface::GetStackFrames() {
  Status error;
  StructuredData::ArraySP arr =
      Dispatch<StructuredData::ArraySP>("get_stackframes", error);

  if (!ScriptedInterface::CheckStructuredDataObject(LLVM_PRETTY_FUNCTION, arr,
                                                    error))
    return {};

  return arr;
}

StructuredData::DictionarySP ScriptedThreadPythonInterface::GetRegisterInfo() {
  Status error;
  StructuredData::DictionarySP dict =
      Dispatch<StructuredData::DictionarySP>("get_register_info", error);

  if (!ScriptedInterface::CheckStructuredDataObject(LLVM_PRETTY_FUNCTION, dict,
                                                    error))
    return {};

  return dict;
}

std::optional<std::string> ScriptedThreadPythonInterface::GetRegisterContext() {
  Status error;
  StructuredData::ObjectSP obj = Dispatch("get_register_context", error);

  if (!ScriptedInterface::CheckStructuredDataObject(LLVM_PRETTY_FUNCTION, obj,
                                                    error))
    return {};

  return obj->GetAsString()->GetValue().str();
}

StructuredData::ArraySP ScriptedThreadPythonInterface::GetExtendedInfo() {
  Status error;
  StructuredData::ArraySP arr =
      Dispatch<StructuredData::ArraySP>("get_extended_info", error);

  if (!ScriptedInterface::CheckStructuredDataObject(LLVM_PRETTY_FUNCTION, arr,
                                                    error))
    return {};

  return arr;
}

std::optional<std::string>
ScriptedThreadPythonInterface::GetScriptedFramePluginName() {
  Status error;
  StructuredData::ObjectSP obj = Dispatch("get_scripted_frame_plugin", error);

  if (!ScriptedInterface::CheckStructuredDataObject(LLVM_PRETTY_FUNCTION, obj,
                                                    error))
    return {};

  return obj->GetStringValue().str();
}

lldb::ScriptedFrameInterfaceSP
ScriptedThreadPythonInterface::CreateScriptedFrameInterface() {
  return m_interpreter.CreateScriptedFrameInterface();
}

#endif
