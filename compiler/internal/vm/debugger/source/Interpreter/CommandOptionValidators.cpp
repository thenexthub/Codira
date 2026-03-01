//===-- CommandOptionValidators.cpp ---------------------------------------===//
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

#include "lldb/Interpreter/CommandOptionValidators.h"

#include "lldb/Interpreter/CommandInterpreter.h"
#include "lldb/Target/Platform.h"

using namespace lldb;
using namespace lldb_private;

bool PosixPlatformCommandOptionValidator::IsValid(
    Platform &platform, const ExecutionContext &target) const {
  llvm::Triple::OSType os =
      platform.GetSystemArchitecture().GetTriple().getOS();
  switch (os) {
  // Are there any other platforms that are not POSIX-compatible?
  case llvm::Triple::Win32:
    return false;
  default:
    return true;
  }
}

const char *PosixPlatformCommandOptionValidator::ShortConditionString() const {
  return "POSIX";
}

const char *PosixPlatformCommandOptionValidator::LongConditionString() const {
  return "Option only valid for POSIX-compliant hosts.";
}
