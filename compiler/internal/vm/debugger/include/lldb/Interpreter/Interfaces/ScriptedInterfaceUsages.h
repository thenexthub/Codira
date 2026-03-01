//===-- ScriptedInterfaceUsages.h ---------------------------- -*- C++ -*-===//
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

#ifndef LLDB_INTERPRETER_SCRIPTEDINTERFACEUSAGES_H
#define LLDB_INTERPRETER_SCRIPTEDINTERFACEUSAGES_H

#include "lldb/lldb-types.h"

#include "lldb/Utility/Stream.h"
#include "llvm/ADT/StringRef.h"

namespace lldb_private {
class ScriptedInterfaceUsages {
public:
  ScriptedInterfaceUsages() = default;
  ScriptedInterfaceUsages(const std::vector<llvm::StringRef> ci_usages,
                          const std::vector<llvm::StringRef> sbapi_usages)
      : m_command_interpreter_usages(ci_usages), m_sbapi_usages(sbapi_usages) {}

  const std::vector<llvm::StringRef> &GetCommandInterpreterUsages() const {
    return m_command_interpreter_usages;
  }

  const std::vector<llvm::StringRef> &GetSBAPIUsages() const {
    return m_sbapi_usages;
  }

  enum class UsageKind { CommandInterpreter, API };

  void Dump(Stream &s, UsageKind kind) const;

private:
  std::vector<llvm::StringRef> m_command_interpreter_usages;
  std::vector<llvm::StringRef> m_sbapi_usages;
};
} // namespace lldb_private

#endif // LLDB_INTERPRETER_SCRIPTEDINTERFACEUSAGES_H
