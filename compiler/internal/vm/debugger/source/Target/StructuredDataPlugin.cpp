//===-- StructuredDataPlugin.cpp ------------------------------------------===//
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

#include "lldb/Target/StructuredDataPlugin.h"

#include "lldb/Core/Debugger.h"
#include "lldb/Interpreter/CommandInterpreter.h"
#include "lldb/Interpreter/CommandObjectMultiword.h"

using namespace lldb;
using namespace lldb_private;

namespace {
class CommandStructuredData : public CommandObjectMultiword {
public:
  CommandStructuredData(CommandInterpreter &interpreter)
      : CommandObjectMultiword(interpreter, "structured-data",
                               "Parent for per-plugin structured data commands",
                               "plugin structured-data <plugin>") {}

  ~CommandStructuredData() override = default;
};
}

StructuredDataPlugin::StructuredDataPlugin(const ProcessWP &process_wp)
    : PluginInterface(), m_process_wp(process_wp) {}

StructuredDataPlugin::~StructuredDataPlugin() = default;

bool StructuredDataPlugin::GetEnabled(llvm::StringRef type_name) const {
  // By default, plugins are always enabled.  Plugin authors should override
  // this if there is an enabled/disabled state for their plugin.
  return true;
}

ProcessSP StructuredDataPlugin::GetProcess() const {
  return m_process_wp.lock();
}

void StructuredDataPlugin::InitializeBasePluginForDebugger(Debugger &debugger) {
  // Create our multiword command anchor if it doesn't already exist.
  auto &interpreter = debugger.GetCommandInterpreter();
  if (!interpreter.GetCommandObject("plugin structured-data")) {
    // Find the parent command.
    auto parent_command =
        debugger.GetCommandInterpreter().GetCommandObject("plugin");
    if (!parent_command)
      return;

    // Create the structured-data command object.
    auto command_name = "structured-data";
    auto command_sp = CommandObjectSP(new CommandStructuredData(interpreter));

    // Hook it up under the top-level plugin command.
    parent_command->LoadSubCommand(command_name, command_sp);
  }
}

void StructuredDataPlugin::ModulesDidLoad(Process &process,
                                          ModuleList &module_list) {
  // Default implementation does nothing.
}
