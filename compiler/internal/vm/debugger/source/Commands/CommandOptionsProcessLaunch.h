//===-- CommandOptionsProcessLaunch.h ---------------------------*- C++ -*-===//
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

#ifndef LLDB_SOURCE_COMMANDS_COMMANDOPTIONSPROCESSLAUNCH_H
#define LLDB_SOURCE_COMMANDS_COMMANDOPTIONSPROCESSLAUNCH_H

#include "lldb/Host/ProcessLaunchInfo.h"
#include "lldb/Interpreter/Options.h"

namespace lldb_private {

// CommandOptionsProcessLaunch

class CommandOptionsProcessLaunch : public lldb_private::OptionGroup {
public:
  CommandOptionsProcessLaunch() {
    // Keep default values of all options in one place: OptionParsingStarting
    // ()
    OptionParsingStarting(nullptr);
  }

  ~CommandOptionsProcessLaunch() override = default;

  lldb_private::Status
  SetOptionValue(uint32_t option_idx, llvm::StringRef option_arg,
                 lldb_private::ExecutionContext *execution_context) override;

  void OptionParsingStarting(
      lldb_private::ExecutionContext *execution_context) override {
    launch_info.Clear();
    disable_aslr = lldb_private::eLazyBoolCalculate;
  }

  llvm::ArrayRef<lldb_private::OptionDefinition> GetDefinitions() override;

  // Instance variables to hold the values for command options.

  lldb_private::ProcessLaunchInfo launch_info;
  lldb_private::LazyBool disable_aslr;
}; // CommandOptionsProcessLaunch

} // namespace lldb_private

#endif // LLDB_SOURCE_COMMANDS_COMMANDOPTIONSPROCESSLAUNCH_H
