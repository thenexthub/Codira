//===-- CommandObjectLanguage.cpp -----------------------------------------===//
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

#include "CommandObjectLanguage.h"



#include "lldb/Target/LanguageRuntime.h"

using namespace lldb;
using namespace lldb_private;

CommandObjectLanguage::CommandObjectLanguage(CommandInterpreter &interpreter)
    : CommandObjectMultiword(
          interpreter, "language", "Commands specific to a source language.",
          "language <language-name> <subcommand> [<subcommand-options>]") {
  // Let the LanguageRuntime populates this command with subcommands
  LanguageRuntime::InitializeCommands(this);
  SetHelpLong(
      R"(
Language specific subcommands may be used directly (without the `language
<language-name>` prefix), when stopped on a frame written in that language. For
example, from a C++ frame, users may run `demangle` directly, instead of
`language cplusplus demangle`.

Language specific subcommands are only available when the command name cannot be
misinterpreted. Take the `demangle` command for example, if a Python command
named `demangle-tree` were loaded, then the invocation `demangle` would run
`demangle-tree`, not `language cplusplus demangle`.
      )");
}

CommandObjectLanguage::~CommandObjectLanguage() = default;
