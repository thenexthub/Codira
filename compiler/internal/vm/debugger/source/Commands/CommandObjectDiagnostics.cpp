//===-- CommandObjectDiagnostics.cpp --------------------------------------===//
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

#include "CommandObjectDiagnostics.h"
#include "lldb/Host/OptionParser.h"
#include "lldb/Interpreter/CommandOptionArgumentTable.h"
#include "lldb/Interpreter/CommandReturnObject.h"
#include "lldb/Interpreter/OptionArgParser.h"
#include "lldb/Interpreter/OptionValueEnumeration.h"
#include "lldb/Interpreter/OptionValueUInt64.h"
#include "lldb/Interpreter/Options.h"
#include "lldb/Utility/Diagnostics.h"

using namespace lldb;
using namespace lldb_private;

#define LLDB_OPTIONS_diagnostics_dump
#include "CommandOptions.inc"

class CommandObjectDiagnosticsDump : public CommandObjectParsed {
public:
  // Constructors and Destructors
  CommandObjectDiagnosticsDump(CommandInterpreter &interpreter)
      : CommandObjectParsed(interpreter, "diagnostics dump",
                            "Dump diagnostics to disk", nullptr) {}

  ~CommandObjectDiagnosticsDump() override = default;

  class CommandOptions : public Options {
  public:
    CommandOptions() = default;

    ~CommandOptions() override = default;

    Status SetOptionValue(uint32_t option_idx, llvm::StringRef option_arg,
                          ExecutionContext *execution_context) override {
      Status error;
      const int short_option = m_getopt_table[option_idx].val;

      switch (short_option) {
      case 'd':
        directory.SetDirectory(option_arg);
        break;
      default:
        llvm_unreachable("Unimplemented option");
      }
      return error;
    }

    void OptionParsingStarting(ExecutionContext *execution_context) override {
      directory.Clear();
    }

    llvm::ArrayRef<OptionDefinition> GetDefinitions() override {
      return llvm::ArrayRef(g_diagnostics_dump_options);
    }

    FileSpec directory;
  };

  Options *GetOptions() override { return &m_options; }

protected:
  llvm::Expected<FileSpec> GetDirectory() {
    if (m_options.directory) {
      auto ec =
          llvm::sys::fs::create_directories(m_options.directory.GetPath());
      if (ec)
        return llvm::errorCodeToError(ec);
      return m_options.directory;
    }
    return Diagnostics::CreateUniqueDirectory();
  }

  void DoExecute(Args &args, CommandReturnObject &result) override {
    llvm::Expected<FileSpec> directory = GetDirectory();

    if (!directory) {
      result.AppendError(llvm::toString(directory.takeError()));
      return;
    }

    llvm::Error error = Diagnostics::Instance().Create(*directory);
    if (error) {
      result.AppendErrorWithFormat("failed to write diagnostics to %s",
                                   directory->GetPath().c_str());
      result.AppendError(llvm::toString(std::move(error)));
      return;
    }

    result.GetOutputStream() << "diagnostics written to " << *directory << '\n';

    result.SetStatus(eReturnStatusSuccessFinishResult);
  }

  CommandOptions m_options;
};

CommandObjectDiagnostics::CommandObjectDiagnostics(
    CommandInterpreter &interpreter)
    : CommandObjectMultiword(interpreter, "diagnostics",
                             "Commands controlling LLDB diagnostics.",
                             "diagnostics <subcommand> [<command-options>]") {
  LoadSubCommand(
      "dump", CommandObjectSP(new CommandObjectDiagnosticsDump(interpreter)));
}

CommandObjectDiagnostics::~CommandObjectDiagnostics() = default;
