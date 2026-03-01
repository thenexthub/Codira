//===-- CommandObjectGUI.cpp ----------------------------------------------===//
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

#include "CommandObjectGUI.h"

#include "lldb/Core/IOHandlerCursesGUI.h"
#include "lldb/Host/Config.h"
#include "lldb/Interpreter/CommandInterpreter.h"
#include "lldb/Interpreter/CommandReturnObject.h"

using namespace lldb;
using namespace lldb_private;

// CommandObjectGUI

CommandObjectGUI::CommandObjectGUI(CommandInterpreter &interpreter)
    : CommandObjectParsed(interpreter, "gui",
                          "Switch into the curses based GUI mode.", "gui") {}

CommandObjectGUI::~CommandObjectGUI() = default;

void CommandObjectGUI::DoExecute(Args &args, CommandReturnObject &result) {
#if LLDB_ENABLE_CURSES
  Debugger &debugger = GetDebugger();

  FileSP input_sp = debugger.GetInputFileSP();
  FileSP output_sp = debugger.GetOutputFileSP();
  if (input_sp->GetStream() && output_sp->GetStream() &&
      input_sp->GetIsRealTerminal() && input_sp->GetIsInteractive()) {
    IOHandlerSP io_handler_sp(new IOHandlerCursesGUI(debugger));
    if (io_handler_sp)
      debugger.RunIOHandlerAsync(io_handler_sp);
    result.SetStatus(eReturnStatusSuccessFinishResult);
  } else {
    result.AppendError("the gui command requires an interactive terminal.");
  }
#else
  result.AppendError("lldb was not built with gui support");
#endif
}
