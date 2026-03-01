//===-- ScriptInterpreterNone.cpp -----------------------------------------===//
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

#include "ScriptInterpreterNone.h"
#include "lldb/Core/Debugger.h"
#include "lldb/Core/PluginManager.h"
#include "lldb/Utility/Stream.h"
#include "lldb/Utility/StringList.h"

#include "llvm/Support/Threading.h"

#include <mutex>

using namespace lldb;
using namespace lldb_private;

LLDB_PLUGIN_DEFINE(ScriptInterpreterNone)

ScriptInterpreterNone::ScriptInterpreterNone(Debugger &debugger)
    : ScriptInterpreter(debugger, eScriptLanguageNone) {}

ScriptInterpreterNone::~ScriptInterpreterNone() = default;

static const char *no_interpreter_err_msg =
    "error: Embedded script interpreter unavailable. LLDB was built without "
    "scripting language support.\n";

bool ScriptInterpreterNone::ExecuteOneLine(llvm::StringRef command,
                                           CommandReturnObject *,
                                           const ExecuteScriptOptions &) {
  m_debugger.GetAsyncErrorStream()->PutCString(no_interpreter_err_msg);
  return false;
}

void ScriptInterpreterNone::ExecuteInterpreterLoop() {
  m_debugger.GetAsyncErrorStream()->PutCString(no_interpreter_err_msg);
}

void ScriptInterpreterNone::Initialize() {
  static llvm::once_flag g_once_flag;

  llvm::call_once(g_once_flag, []() {
    PluginManager::RegisterPlugin(GetPluginNameStatic(),
                                  GetPluginDescriptionStatic(),
                                  lldb::eScriptLanguageNone, CreateInstance);
  });
}

void ScriptInterpreterNone::Terminate() {}

lldb::ScriptInterpreterSP
ScriptInterpreterNone::CreateInstance(Debugger &debugger) {
  return std::make_shared<ScriptInterpreterNone>(debugger);
}

llvm::StringRef ScriptInterpreterNone::GetPluginDescriptionStatic() {
  return "Null script interpreter";
}
