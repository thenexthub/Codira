//===-- ScriptInterpreterNone.h ---------------------------------*- C++ -*-===//
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

#ifndef LLDB_SOURCE_PLUGINS_SCRIPTINTERPRETER_NONE_SCRIPTINTERPRETERNONE_H
#define LLDB_SOURCE_PLUGINS_SCRIPTINTERPRETER_NONE_SCRIPTINTERPRETERNONE_H

#include "lldb/Interpreter/ScriptInterpreter.h"

namespace lldb_private {

class ScriptInterpreterNone : public ScriptInterpreter {
public:
  ScriptInterpreterNone(Debugger &debugger);

  ~ScriptInterpreterNone() override;

  bool ExecuteOneLine(
      llvm::StringRef command, CommandReturnObject *result,
      const ExecuteScriptOptions &options = ExecuteScriptOptions()) override;

  void ExecuteInterpreterLoop() override;

  // Static Functions
  static void Initialize();

  static void Terminate();

  static lldb::ScriptInterpreterSP CreateInstance(Debugger &debugger);

  static llvm::StringRef GetPluginNameStatic() { return "script-none"; }

  static llvm::StringRef GetPluginDescriptionStatic();

  // PluginInterface protocol
  llvm::StringRef GetPluginName() override { return GetPluginNameStatic(); }
};

} // namespace lldb_private

#endif // LLDB_SOURCE_PLUGINS_SCRIPTINTERPRETER_NONE_SCRIPTINTERPRETERNONE_H
