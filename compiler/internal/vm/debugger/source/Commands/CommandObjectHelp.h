//===-- CommandObjectHelp.h -------------------------------------*- C++ -*-===//
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

#ifndef LLDB_SOURCE_COMMANDS_COMMANDOBJECTHELP_H
#define LLDB_SOURCE_COMMANDS_COMMANDOBJECTHELP_H

#include "lldb/Host/OptionParser.h"
#include "lldb/Interpreter/CommandObject.h"
#include "lldb/Interpreter/Options.h"

namespace lldb_private {

// CommandObjectHelp

class CommandObjectHelp : public CommandObjectParsed {
public:
  CommandObjectHelp(CommandInterpreter &interpreter);

  ~CommandObjectHelp() override;

  void HandleCompletion(CompletionRequest &request) override;

  static void GenerateAdditionalHelpAvenuesMessage(
      Stream *s, llvm::StringRef command, llvm::StringRef prefix,
      llvm::StringRef subcommand, bool include_upropos = true,
      bool include_type_lookup = true);

  class CommandOptions : public Options {
  public:
    CommandOptions() = default;

    ~CommandOptions() override = default;

    Status SetOptionValue(uint32_t option_idx, llvm::StringRef option_arg,
                          ExecutionContext *execution_context) override {
      Status error;
      const int short_option = m_getopt_table[option_idx].val;

      switch (short_option) {
      case 'a':
        m_show_aliases = false;
        break;
      case 'u':
        m_show_user_defined = false;
        break;
      case 'h':
        m_show_hidden = true;
        break;
      default:
        llvm_unreachable("Unimplemented option");
      }

      return error;
    }

    void OptionParsingStarting(ExecutionContext *execution_context) override {
      m_show_aliases = true;
      m_show_user_defined = true;
      m_show_hidden = false;
    }

    llvm::ArrayRef<OptionDefinition> GetDefinitions() override;

    // Instance variables to hold the values for command options.

    bool m_show_aliases;
    bool m_show_user_defined;
    bool m_show_hidden;
  };

  Options *GetOptions() override { return &m_options; }

protected:
  void DoExecute(Args &command, CommandReturnObject &result) override;

private:
  CommandOptions m_options;
};

} // namespace lldb_private

#endif // LLDB_SOURCE_COMMANDS_COMMANDOBJECTHELP_H
