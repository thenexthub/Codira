//===-- ScriptedBreakpointPythonInterface.h -----------------------*- C++
//-*-===//
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

#ifndef LLDB_PLUGINS_SCRIPTINTERPRETER_PYTHON_INTERFACES_SCRIPTEDBREAKPOINTPYTHONINTERFACE_H
#define LLDB_PLUGINS_SCRIPTINTERPRETER_PYTHON_INTERFACES_SCRIPTEDBREAKPOINTPYTHONINTERFACE_H

#include "lldb/Host/Config.h"
#include "lldb/Interpreter/Interfaces/ScriptedBreakpointInterface.h"

#if LLDB_ENABLE_PYTHON

#include "ScriptedPythonInterface.h"

namespace lldb_private {
class ScriptedBreakpointPythonInterface : public ScriptedBreakpointInterface,
                                          public ScriptedPythonInterface,
                                          public PluginInterface {
public:
  ScriptedBreakpointPythonInterface(ScriptInterpreterPythonImpl &interpreter);

  llvm::Expected<StructuredData::GenericSP>
  CreatePluginObject(llvm::StringRef class_name, lldb::BreakpointSP break_sp,
                     const StructuredDataImpl &args_sp) override;

  llvm::SmallVector<AbstractMethodRequirement>
  GetAbstractMethodRequirements() const override {
    return llvm::SmallVector<AbstractMethodRequirement>({{"__callback__", 2}});
  }

  bool ResolverCallback(SymbolContext sym_ctx) override;
  lldb::SearchDepth GetDepth() override;
  std::optional<std::string> GetShortHelp() override;
  lldb::BreakpointLocationSP
  WasHit(lldb::StackFrameSP frame_sp,
         lldb::BreakpointLocationSP bp_loc_sp) override;
  virtual std::optional<std::string>
  GetLocationDescription(lldb::BreakpointLocationSP bp_loc_sp,
                         lldb::DescriptionLevel level) override;

  static void Initialize();

  static void Terminate();

  static llvm::StringRef GetPluginNameStatic() {
    return "ScriptedBreakpointPythonInterface";
  }

  llvm::StringRef GetPluginName() override { return GetPluginNameStatic(); }
};
} // namespace lldb_private

#endif // LLDB_ENABLE_PYTHON
#endif // LLDB_PLUGINS_SCRIPTINTERPRETER_PYTHON_INTERFACES_SCRIPTEDBREAKPOINTPYTHONINTERFACE_H
