//===-- CommandObjectDWIMPrint.h --------------------------------*- C++ -*-===//
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

#ifndef LLDB_SOURCE_COMMANDS_COMMANDOBJECTDWIMPRINT_H
#define LLDB_SOURCE_COMMANDS_COMMANDOBJECTDWIMPRINT_H

#include "CommandObjectExpression.h"
#include "lldb/Interpreter/CommandObject.h"
#include "lldb/Interpreter/OptionGroupFormat.h"
#include "lldb/Interpreter/OptionGroupValueObjectDisplay.h"
#include "lldb/Interpreter/OptionValueFormat.h"

namespace lldb_private {

/// Implements `dwim-print`, a printing command that chooses the most direct,
/// efficient, and resilient means of printing a given expression.
///
/// DWIM is an acronym for Do What I Mean. From Wikipedia, DWIM is described as:
///
///   > attempt to anticipate what users intend to do, correcting trivial errors
///   > automatically rather than blindly executing users' explicit but
///   > potentially incorrect input
///
/// The `dwim-print` command serves as a single print command for users who
/// don't yet know, or perfer not to know, the various lldb commands that can be
/// used to print, and when to use them.
class CommandObjectDWIMPrint : public CommandObjectRaw {
public:
  CommandObjectDWIMPrint(CommandInterpreter &interpreter);

  ~CommandObjectDWIMPrint() override = default;

  Options *GetOptions() override;

  bool WantsCompletion() override { return true; }

private:
  void DoExecute(llvm::StringRef command, CommandReturnObject &result) override;

  OptionGroupOptions m_option_group;
  OptionGroupFormat m_format_options = lldb::eFormatDefault;
  OptionGroupValueObjectDisplay m_varobj_options;
  CommandObjectExpression::CommandOptions m_expr_options;
};

} // namespace lldb_private

#endif
