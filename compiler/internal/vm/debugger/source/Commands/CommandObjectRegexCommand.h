//===-- CommandObjectRegexCommand.h -----------------------------*- C++ -*-===//
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

#ifndef LLDB_INTERPRETER_COMMANDOBJECTREGEXCOMMAND_H
#define LLDB_INTERPRETER_COMMANDOBJECTREGEXCOMMAND_H

#include <list>

#include "lldb/Interpreter/CommandObject.h"
#include "lldb/Utility/CompletionRequest.h"
#include "lldb/Utility/RegularExpression.h"

namespace lldb_private {

// CommandObjectRegexCommand

class CommandObjectRegexCommand : public CommandObjectRaw {
public:
  CommandObjectRegexCommand(CommandInterpreter &interpreter,
                            llvm::StringRef name, llvm::StringRef help,
                            llvm::StringRef syntax,
                            uint32_t completion_type_mask, bool is_removable);

  ~CommandObjectRegexCommand() override;

  bool IsRemovable() const override { return m_is_removable; }

  bool AddRegexCommand(llvm::StringRef re_cstr, llvm::StringRef command_cstr);

  bool HasRegexEntries() const { return !m_entries.empty(); }

  void HandleCompletion(CompletionRequest &request) override;

protected:
  void DoExecute(llvm::StringRef command, CommandReturnObject &result) override;

  /// Substitute variables of the format %\d+ in the input string.
  static llvm::Expected<std::string> SubstituteVariables(
      llvm::StringRef input,
      const llvm::SmallVectorImpl<llvm::StringRef> &replacements);

  struct Entry {
    RegularExpression regex;
    std::string command;
  };

  typedef std::list<Entry> EntryCollection;
  const uint32_t m_completion_type_mask;
  EntryCollection m_entries;
  bool m_is_removable;

private:
  CommandObjectRegexCommand(const CommandObjectRegexCommand &) = delete;
  const CommandObjectRegexCommand &
  operator=(const CommandObjectRegexCommand &) = delete;
};

} // namespace lldb_private

#endif // LLDB_INTERPRETER_COMMANDOBJECTREGEXCOMMAND_H
