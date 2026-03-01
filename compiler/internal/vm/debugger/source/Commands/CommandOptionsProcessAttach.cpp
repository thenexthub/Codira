//===-- CommandOptionsProcessAttach.cpp -----------------------------------===//
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

#include "CommandOptionsProcessAttach.h"

#include "lldb/Host/FileSystem.h"
#include "lldb/Host/HostInfo.h"
#include "lldb/Host/OptionParser.h"
#include "lldb/Interpreter/CommandCompletions.h"
#include "lldb/Interpreter/CommandObject.h"
#include "lldb/Interpreter/CommandOptionArgumentTable.h"
#include "lldb/Interpreter/OptionArgParser.h"
#include "lldb/Target/ExecutionContext.h"
#include "lldb/Target/Platform.h"
#include "lldb/Target/Target.h"

#include "llvm/ADT/ArrayRef.h"

using namespace llvm;
using namespace lldb;
using namespace lldb_private;

#define LLDB_OPTIONS_process_attach
#include "CommandOptions.inc"

Status CommandOptionsProcessAttach::SetOptionValue(
    uint32_t option_idx, llvm::StringRef option_arg,
    ExecutionContext *execution_context) {
  Status error;
  const int short_option = g_process_attach_options[option_idx].short_option;
  switch (short_option) {
  case 'c':
    attach_info.SetContinueOnceAttached(true);
    break;

  case 'p': {
    lldb::pid_t pid;
    if (option_arg.getAsInteger(0, pid)) {
      return Status::FromErrorStringWithFormatv("invalid process ID '{0}'",
                                                option_arg);
    } else {
      attach_info.SetProcessID(pid);
    }
  } break;

  case 'P':
    attach_info.SetProcessPluginName(option_arg);
    break;

  case 'n':
    attach_info.GetExecutableFile().SetFile(option_arg,
                                            FileSpec::Style::native);
    break;

  case 'w':
    attach_info.SetWaitForLaunch(true);
    break;

  case 'i':
    attach_info.SetIgnoreExisting(false);
    break;

  default:
    llvm_unreachable("Unimplemented option");
  }
  return error;
}

llvm::ArrayRef<OptionDefinition> CommandOptionsProcessAttach::GetDefinitions() {
  return llvm::ArrayRef(g_process_attach_options);
}
