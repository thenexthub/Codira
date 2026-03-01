//===-- CommandObjectVersion.h ----------------------------------*- C++ -*-===//
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

#ifndef LLDB_SOURCE_COMMANDS_COMMANDOBJECTVERSION_H
#define LLDB_SOURCE_COMMANDS_COMMANDOBJECTVERSION_H

#include "lldb/Host/OptionParser.h"
#include "lldb/Interpreter/CommandObject.h"
#include "lldb/Interpreter/Options.h"

namespace lldb_private {

class CommandObjectVersion : public CommandObjectParsed {
public:
  CommandObjectVersion(CommandInterpreter &interpreter);

  ~CommandObjectVersion() override;

  class CommandOptions : public Options {
  public:
    CommandOptions() = default;

    ~CommandOptions() override = default;

    Status SetOptionValue(uint32_t option_idx, llvm::StringRef option_arg,
                          ExecutionContext *execution_context) override {
      Status error;
      const int short_option = m_getopt_table[option_idx].val;

      switch (short_option) {
      case 'v':
        verbose = true;
        break;
      default:
        llvm_unreachable("Unimplemented option");
      }

      return error;
    }

    void OptionParsingStarting(ExecutionContext *execution_context) override {
      verbose = false;
    }

    llvm::ArrayRef<OptionDefinition> GetDefinitions() override;

    bool verbose;
  };

  Options *GetOptions() override { return &m_options; }

protected:
  void DoExecute(Args &args, CommandReturnObject &result) override;

private:
  CommandOptions m_options;
};

} // namespace lldb_private

#endif // LLDB_SOURCE_COMMANDS_COMMANDOBJECTVERSION_H
