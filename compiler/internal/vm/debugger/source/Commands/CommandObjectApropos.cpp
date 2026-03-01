//===-- CommandObjectApropos.cpp ------------------------------------------===//
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

#include "CommandObjectApropos.h"
#include "lldb/Interpreter/CommandInterpreter.h"
#include "lldb/Interpreter/CommandReturnObject.h"
#include "lldb/Interpreter/Property.h"
#include "lldb/Utility/Args.h"

using namespace lldb;
using namespace lldb_private;

// CommandObjectApropos

CommandObjectApropos::CommandObjectApropos(CommandInterpreter &interpreter)
    : CommandObjectParsed(
          interpreter, "apropos",
          "List debugger commands related to a word or subject.", nullptr) {
  AddSimpleArgumentList(eArgTypeSearchWord);
}

CommandObjectApropos::~CommandObjectApropos() = default;

void CommandObjectApropos::DoExecute(Args &args, CommandReturnObject &result) {
  const size_t argc = args.GetArgumentCount();

  if (argc == 1) {
    auto search_word = args[0].ref();
    if (!search_word.empty()) {
      // The bulk of the work must be done inside the Command Interpreter,
      // since the command dictionary is private.
      StringList commands_found;
      StringList commands_help;

      m_interpreter.FindCommandsForApropos(
          search_word, commands_found, commands_help, true, true, true, true);

      if (commands_found.GetSize() == 0) {
        result.AppendMessageWithFormat("No commands found pertaining to '%s'. "
                                       "Try 'help' to see a complete list of "
                                       "debugger commands.\n",
                                       args[0].c_str());
      } else {
        if (commands_found.GetSize() > 0) {
          result.AppendMessageWithFormat(
              "The following commands may relate to '%s':\n", args[0].c_str());
          const size_t max_len = commands_found.GetMaxStringLength();

          for (size_t i = 0; i < commands_found.GetSize(); ++i)
            m_interpreter.OutputFormattedHelpText(
                result.GetOutputStream(), commands_found.GetStringAtIndex(i),
                "--", commands_help.GetStringAtIndex(i), max_len);
        }
      }

      std::vector<const Property *> properties;
      const size_t num_properties =
          GetDebugger().Apropos(search_word, properties);
      if (num_properties) {
        const bool dump_qualified_name = true;
        result.AppendMessageWithFormatv(
            "\nThe following settings variables may relate to '{0}': \n\n",
            args[0].ref());
        for (size_t i = 0; i < num_properties; ++i)
          properties[i]->DumpDescription(
              m_interpreter, result.GetOutputStream(), 0, dump_qualified_name);
      }

      result.SetStatus(eReturnStatusSuccessFinishNoResult);
    } else {
      result.AppendError("'' is not a valid search word.\n");
    }
  } else {
    result.AppendError("'apropos' must be called with exactly one argument.\n");
  }
}
