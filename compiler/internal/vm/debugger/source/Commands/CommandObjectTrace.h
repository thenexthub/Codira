//===-- CommandObjectTrace.h ------------------------------------*- C++ -*-===//
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

#ifndef LLDB_SOURCE_COMMANDS_COMMANDOBJECTTRACE_H
#define LLDB_SOURCE_COMMANDS_COMMANDOBJECTTRACE_H

#include "CommandObjectThreadUtil.h"

namespace lldb_private {

class CommandObjectTrace : public CommandObjectMultiword {
public:
  CommandObjectTrace(CommandInterpreter &interpreter);

  ~CommandObjectTrace() override;
};

/// This class works by delegating the logic to the actual trace plug-in that
/// can support the current process.
class CommandObjectTraceProxy : public CommandObjectProxy {
public:
  CommandObjectTraceProxy(bool live_debug_session_only,
                          CommandInterpreter &interpreter, const char *name,
                          const char *help = nullptr,
                          const char *syntax = nullptr, uint32_t flags = 0)
      : CommandObjectProxy(interpreter, name, help, syntax, flags),
        m_live_debug_session_only(live_debug_session_only) {}

protected:
  virtual lldb::CommandObjectSP GetDelegateCommand(Trace &trace) = 0;

  llvm::Expected<lldb::CommandObjectSP> DoGetProxyCommandObject();

  CommandObject *GetProxyCommandObject() override;

private:
  llvm::StringRef GetUnsupportedError() override { return m_delegate_error; }

  bool m_live_debug_session_only;
  lldb::CommandObjectSP m_delegate_sp;
  std::string m_delegate_error;
};

} // namespace lldb_private

#endif // LLDB_SOURCE_COMMANDS_COMMANDOBJECTTRACE_H
