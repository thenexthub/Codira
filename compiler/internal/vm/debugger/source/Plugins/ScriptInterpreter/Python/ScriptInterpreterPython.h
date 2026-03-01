//===-- ScriptInterpreterPython.h -------------------------------*- C++ -*-===//
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

#ifndef LLDB_PLUGINS_SCRIPTINTERPRETER_PYTHON_SCRIPTINTERPRETERPYTHON_H
#define LLDB_PLUGINS_SCRIPTINTERPRETER_PYTHON_SCRIPTINTERPRETERPYTHON_H

#include "lldb/Host/Config.h"

#if LLDB_ENABLE_PYTHON

#include "lldb/Breakpoint/BreakpointOptions.h"
#include "lldb/Core/IOHandler.h"
#include "lldb/Core/StructuredDataImpl.h"
#include "lldb/Interpreter/ScriptInterpreter.h"
#include "lldb/lldb-private.h"

#include <memory>
#include <string>
#include <vector>

namespace lldb_private {
/// Abstract interface for the Python script interpreter.
class ScriptInterpreterPython : public ScriptInterpreter,
                                public IOHandlerDelegateMultiline {
public:
  class CommandDataPython : public BreakpointOptions::CommandData {
  public:
    CommandDataPython() : BreakpointOptions::CommandData() {
      interpreter = lldb::eScriptLanguagePython;
    }
    CommandDataPython(StructuredData::ObjectSP extra_args_sp)
        : BreakpointOptions::CommandData(),
          m_extra_args(std::move(extra_args_sp)) {
      interpreter = lldb::eScriptLanguagePython;
    }
    StructuredDataImpl m_extra_args;
  };

  ScriptInterpreterPython(Debugger &debugger)
      : ScriptInterpreter(debugger, lldb::eScriptLanguagePython),
        IOHandlerDelegateMultiline("DONE") {}

  StructuredData::DictionarySP GetInterpreterInfo() override;
  static void Initialize();
  static void Terminate();
  static llvm::StringRef GetPluginNameStatic() { return "script-python"; }
  static llvm::StringRef GetPluginDescriptionStatic();
  static FileSpec GetPythonDir();
  static void SharedLibraryDirectoryHelper(FileSpec &this_file);

protected:
  static void ComputePythonDirForApple(llvm::SmallVectorImpl<char> &path);
  static void ComputePythonDir(llvm::SmallVectorImpl<char> &path);
};
} // namespace lldb_private

#endif // LLDB_ENABLE_PYTHON
#endif // LLDB_PLUGINS_SCRIPTINTERPRETER_PYTHON_SCRIPTINTERPRETERPYTHON_H
