//===-- CommandObjectMultiword.h --------------------------------*- C++ -*-===//
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

#ifndef LLDB_INTERPRETER_COMMANDOBJECTMULTIWORD_H
#define LLDB_INTERPRETER_COMMANDOBJECTMULTIWORD_H

#include "lldb/Interpreter/CommandObject.h"
#include "lldb/Utility/CompletionRequest.h"
#include <optional>

namespace lldb_private {

// CommandObjectMultiword

class CommandObjectMultiword : public CommandObject {
  // These two want to iterate over the subcommand dictionary.
  friend class CommandInterpreter;
  friend class CommandObjectSyntax;

public:
  CommandObjectMultiword(CommandInterpreter &interpreter, const char *name,
                         const char *help = nullptr,
                         const char *syntax = nullptr, uint32_t flags = 0);

  ~CommandObjectMultiword() override;

  bool IsMultiwordObject() override { return true; }

  CommandObjectMultiword *GetAsMultiwordCommand() override { return this; }

  bool LoadSubCommand(llvm::StringRef cmd_name,
                      const lldb::CommandObjectSP &command_obj) override;

  llvm::Error LoadUserSubcommand(llvm::StringRef cmd_name,
                                 const lldb::CommandObjectSP &command_obj,
                                 bool can_replace) override;

  llvm::Error RemoveUserSubcommand(llvm::StringRef cmd_name, bool multiword_okay);

  void GenerateHelpText(Stream &output_stream) override;

  lldb::CommandObjectSP GetSubcommandSP(llvm::StringRef sub_cmd,
                                        StringList *matches = nullptr) override;

  lldb::CommandObjectSP GetSubcommandSPExact(llvm::StringRef sub_cmd) override;

  CommandObject *GetSubcommandObject(llvm::StringRef sub_cmd,
                                     StringList *matches = nullptr) override;

  bool WantsRawCommandString() override { return false; }

  void HandleCompletion(CompletionRequest &request) override;

  std::optional<std::string> GetRepeatCommand(Args &current_command_args,
                                              uint32_t index) override;

  void Execute(const char *args_string, CommandReturnObject &result) override;

  bool IsRemovable() const override { return m_can_be_removed; }

  void SetRemovable(bool removable) { m_can_be_removed = removable; }

protected:
  CommandObject::CommandMap &GetSubcommandDictionary() {
    return m_subcommand_dict;
  }

  std::string GetSubcommandsHintText();

  CommandObject::CommandMap m_subcommand_dict;
  bool m_can_be_removed;
};

class CommandObjectProxy : public CommandObject {
public:
  CommandObjectProxy(CommandInterpreter &interpreter, const char *name,
                     const char *help = nullptr, const char *syntax = nullptr,
                     uint32_t flags = 0);

  ~CommandObjectProxy() override;

  // Subclasses must provide a command object that will be transparently used
  // for this object.
  virtual CommandObject *GetProxyCommandObject() = 0;

  llvm::StringRef GetSyntax() override;

  llvm::StringRef GetHelp() override;

  llvm::StringRef GetHelpLong() override;

  bool IsRemovable() const override;

  bool IsMultiwordObject() override;

  CommandObjectMultiword *GetAsMultiwordCommand() override;

  void GenerateHelpText(Stream &result) override;

  lldb::CommandObjectSP GetSubcommandSP(llvm::StringRef sub_cmd,
                                        StringList *matches = nullptr) override;

  CommandObject *GetSubcommandObject(llvm::StringRef sub_cmd,
                                     StringList *matches = nullptr) override;

  bool LoadSubCommand(llvm::StringRef cmd_name,
                      const lldb::CommandObjectSP &command_obj) override;

  bool WantsRawCommandString() override;

  bool WantsCompletion() override;

  Options *GetOptions() override;

  void HandleCompletion(CompletionRequest &request) override;

  void
  HandleArgumentCompletion(CompletionRequest &request,
                           OptionElementVector &opt_element_vector) override;

  std::optional<std::string> GetRepeatCommand(Args &current_command_args,
                                              uint32_t index) override;

  /// \return
  ///     An error message to be displayed when the command is executed (i.e.
  ///     Execute is called) and \a GetProxyCommandObject returned null.
  virtual llvm::StringRef GetUnsupportedError();

  void Execute(const char *args_string, CommandReturnObject &result) override;

protected:
  // These two want to iterate over the subcommand dictionary.
  friend class CommandInterpreter;
  friend class CommandObjectSyntax;
};

} // namespace lldb_private

#endif // LLDB_INTERPRETER_COMMANDOBJECTMULTIWORD_H
