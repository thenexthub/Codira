//===-- CommandPlugins.h --------------------------------------------------===//
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

#ifndef LLDB_TOOLS_LLDB_DAP_COMMANDPLUGINS_H
#define LLDB_TOOLS_LLDB_DAP_COMMANDPLUGINS_H

#include "DAP.h"
#include "lldb/API/SBCommandInterpreter.h"

namespace lldb_dap {

struct StartDebuggingCommand : public lldb::SBCommandPluginInterface {
  DAP &dap;
  explicit StartDebuggingCommand(DAP &d) : dap(d) {};
  bool DoExecute(lldb::SBDebugger debugger, char **command,
                 lldb::SBCommandReturnObject &result) override;
};

struct ReplModeCommand : public lldb::SBCommandPluginInterface {
  DAP &dap;
  explicit ReplModeCommand(DAP &d) : dap(d) {};
  bool DoExecute(lldb::SBDebugger debugger, char **command,
                 lldb::SBCommandReturnObject &result) override;
};

struct SendEventCommand : public lldb::SBCommandPluginInterface {
  DAP &dap;
  explicit SendEventCommand(DAP &d) : dap(d) {};
  bool DoExecute(lldb::SBDebugger debugger, char **command,
                 lldb::SBCommandReturnObject &result) override;
};

} // namespace lldb_dap

#endif
