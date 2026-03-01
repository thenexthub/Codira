//===-- ScriptedInterfaceUsages.cpp --------------------------------------===//
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

#include "lldb/Interpreter/Interfaces/ScriptedInterfaceUsages.h"

using namespace lldb;
using namespace lldb_private;

void ScriptedInterfaceUsages::Dump(Stream &s, UsageKind kind) const {
  s.IndentMore();
  s.Indent();
  llvm::StringRef usage_kind =
      (kind == UsageKind::CommandInterpreter) ? "Command Interpreter" : "API";
  s << usage_kind << " Usages:";
  const std::vector<llvm::StringRef> &usages =
      (kind == UsageKind::CommandInterpreter) ? GetCommandInterpreterUsages()
                                              : GetSBAPIUsages();
  if (usages.empty())
    s << " None\n";
  else if (usages.size() == 1)
    s << " " << usages.front() << '\n';
  else {
    s << '\n';
    for (llvm::StringRef usage : usages) {
      s.IndentMore();
      s.Indent();
      s << usage << '\n';
      s.IndentLess();
    }
  }
  s.IndentLess();
}
