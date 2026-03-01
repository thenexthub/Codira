//===-- Utility.cpp -------------------------------------------------------===//
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

#include "Utility.h"

#include "lldb/Core/Module.h"
#include "lldb/Target/Target.h"

namespace lldb_private {

std::tuple<lldb::ModuleSP, HistoryPCType>
GetPreferredAsanModule(const Target &target) {
  // Currently only Darwin provides ASan runtime support as part of the OS
  // (libsanitizers).
  if (!target.GetArchitecture().GetTriple().isOSDarwin())
    return {nullptr, HistoryPCType::Calls};

  lldb::ModuleSP module;
  llvm::Regex pattern(R"(libclang_rt\.asan_.*_dynamic\.dylib)");
  target.GetImages().ForEach([&](const lldb::ModuleSP &m) {
    if (pattern.match(m->GetFileSpec().GetFilename().GetStringRef())) {
      module = m;
      return IterationAction::Stop;
    }

    return IterationAction::Continue;
  });

  // `Calls` - The ASan compiler-rt runtime already massages the return
  //   addresses into call addresses, so we don't want LLDB's unwinder to try to
  //   locate the previous instruction again as this might lead to us reporting
  //   a different line.
  // `ReturnsNoZerothFrame` - Darwin, but not ASan compiler-rt implies
  //   libsanitizers which collects return addresses.  It also discards a few
  //   non-user frames at the top of the stack.
  auto pc_type =
      (module ? HistoryPCType::Calls : HistoryPCType::ReturnsNoZerothFrame);
  return {module, pc_type};
}

} // namespace lldb_private
