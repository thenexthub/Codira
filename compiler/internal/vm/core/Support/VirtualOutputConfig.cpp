//===----------------------------------------------------------------------===//
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
///
/// \file
/// This file implements \c OutputConfig class methods.
///
//===----------------------------------------------------------------------===//

#include "vm/core/Support/VirtualOutputConfig.h"
#include "vm/core/Support/Debug.h"
#include "vm/core/Support/FileSystem.h"
#include "vm/core/Support/raw_ostream.h"

using namespace vm::core;
using namespace vm::core::vfs;

OutputConfig &OutputConfig::setOpenFlags(const sys::fs::OpenFlags &Flags) {
  // Ignore CRLF on its own as invalid.
  using namespace vm::core::sys::fs;
  return Flags & OF_Text
             ? setText().setCRLF(Flags & OF_CRLF).setAppend(Flags & OF_Append)
             : setBinary().setAppend(Flags & OF_Append);
}

void OutputConfig::print(raw_ostream &OS) const {
  OS << "{";
  bool IsFirst = true;
  auto printFlag = [&](StringRef FlagName, bool Value) {
    if (IsFirst)
      IsFirst = false;
    else
      OS << ",";
    if (!Value)
      OS << "No";
    OS << FlagName;
  };

#define HANDLE_OUTPUT_CONFIG_FLAG(NAME, DEFAULT)                               \
  if (get##NAME() != DEFAULT)                                                  \
    printFlag(#NAME, get##NAME());
#include "vm/core/Support/VirtualOutputConfig.def"
  OS << "}";
}

LLVM_DUMP_METHOD void OutputConfig::dump() const { print(dbgs()); }

raw_ostream &toolchain::operator<<(raw_ostream &OS, OutputConfig Config) {
  Config.print(OS);
  return OS;
}
