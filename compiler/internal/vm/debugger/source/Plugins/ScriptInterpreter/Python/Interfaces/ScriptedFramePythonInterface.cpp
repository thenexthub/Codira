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

#include "lldb/Host/Config.h"
#include "lldb/Target/ExecutionContext.h"
#include "lldb/Utility/Log.h"
#include "lldb/lldb-enumerations.h"

#if LLDB_ENABLE_PYTHON

// LLDB Python header must be included first
#include "../lldb-python.h"

#include "../SWIGPythonBridge.h"
#include "../ScriptInterpreterPythonImpl.h"
#include "ScriptedFramePythonInterface.h"
#include <optional>

using namespace lldb;
using namespace lldb_private;
using namespace lldb_private::python;
using Locker = ScriptInterpreterPythonImpl::Locker;

ScriptedFramePythonInterface::ScriptedFramePythonInterface(
    ScriptInterpreterPythonImpl &interpreter)
    : ScriptedFrameInterface(), ScriptedPythonInterface(interpreter) {}

llvm::Expected<StructuredData::GenericSP>
ScriptedFramePythonInterface::CreatePluginObject(
    const llvm::StringRef class_name, ExecutionContext &exe_ctx,
    StructuredData::DictionarySP args_sp, StructuredData::Generic *script_obj) {
  ExecutionContextRefSP exe_ctx_ref_sp =
      std::make_shared<ExecutionContextRef>(exe_ctx);
  StructuredDataImpl sd_impl(args_sp);
  return ScriptedPythonInterface::CreatePluginObject(class_name, script_obj,
                                                     exe_ctx_ref_sp, sd_impl);
}

lldb::user_id_t ScriptedFramePythonInterface::GetID() {
  Status error;
  StructuredData::ObjectSP obj = Dispatch("get_id", error);

  if (!ScriptedInterface::CheckStructuredDataObject(LLVM_PRETTY_FUNCTION, obj,
                                                    error))
    return LLDB_INVALID_FRAME_ID;

  return obj->GetUnsignedIntegerValue(LLDB_INVALID_FRAME_ID);
}

lldb::addr_t ScriptedFramePythonInterface::GetPC() {
  Status error;
  StructuredData::ObjectSP obj = Dispatch("get_pc", error);

  if (!ScriptedInterface::CheckStructuredDataObject(LLVM_PRETTY_FUNCTION, obj,
                                                    error))
    return LLDB_INVALID_ADDRESS;

  return obj->GetUnsignedIntegerValue(LLDB_INVALID_ADDRESS);
}

std::optional<SymbolContext> ScriptedFramePythonInterface::GetSymbolContext() {
  Status error;
  auto sym_ctx = Dispatch<SymbolContext>("get_symbol_context", error);

  if (error.Fail()) {
    return ErrorWithMessage<SymbolContext>(LLVM_PRETTY_FUNCTION,
                                           error.AsCString(), error);
  }

  return sym_ctx;
}

std::optional<std::string> ScriptedFramePythonInterface::GetFunctionName() {
  Status error;
  StructuredData::ObjectSP obj = Dispatch("get_function_name", error);

  if (!ScriptedInterface::CheckStructuredDataObject(LLVM_PRETTY_FUNCTION, obj,
                                                    error))
    return {};

  return obj->GetStringValue().str();
}

std::optional<std::string>
ScriptedFramePythonInterface::GetDisplayFunctionName() {
  Status error;
  StructuredData::ObjectSP obj = Dispatch("get_display_function_name", error);

  if (!ScriptedInterface::CheckStructuredDataObject(LLVM_PRETTY_FUNCTION, obj,
                                                    error))
    return {};

  return obj->GetStringValue().str();
}

bool ScriptedFramePythonInterface::IsInlined() {
  Status error;
  StructuredData::ObjectSP obj = Dispatch("is_inlined", error);

  if (!ScriptedInterface::CheckStructuredDataObject(LLVM_PRETTY_FUNCTION, obj,
                                                    error))
    return false;

  return obj->GetBooleanValue();
}

bool ScriptedFramePythonInterface::IsArtificial() {
  Status error;
  StructuredData::ObjectSP obj = Dispatch("is_artificial", error);

  if (!ScriptedInterface::CheckStructuredDataObject(LLVM_PRETTY_FUNCTION, obj,
                                                    error))
    return false;

  return obj->GetBooleanValue();
}

bool ScriptedFramePythonInterface::IsHidden() {
  Status error;
  StructuredData::ObjectSP obj = Dispatch("is_hidden", error);

  if (!ScriptedInterface::CheckStructuredDataObject(LLVM_PRETTY_FUNCTION, obj,
                                                    error))
    return false;

  return obj->GetBooleanValue();
}

StructuredData::DictionarySP ScriptedFramePythonInterface::GetRegisterInfo() {
  Status error;
  StructuredData::DictionarySP dict =
      Dispatch<StructuredData::DictionarySP>("get_register_info", error);

  if (!ScriptedInterface::CheckStructuredDataObject(LLVM_PRETTY_FUNCTION, dict,
                                                    error))
    return {};

  return dict;
}

std::optional<std::string> ScriptedFramePythonInterface::GetRegisterContext() {
  Status error;
  StructuredData::ObjectSP obj = Dispatch("get_register_context", error);

  if (!ScriptedInterface::CheckStructuredDataObject(LLVM_PRETTY_FUNCTION, obj,
                                                    error))
    return {};

  return obj->GetAsString()->GetValue().str();
}

#endif
