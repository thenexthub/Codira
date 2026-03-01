//===-- OptionGroupArchitecture.cpp ---------------------------------------===//
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

#include "lldb/Interpreter/OptionGroupArchitecture.h"
#include "lldb/Host/OptionParser.h"
#include "lldb/Target/Platform.h"

using namespace lldb;
using namespace lldb_private;

static constexpr OptionDefinition g_option_table[] = {
    {LLDB_OPT_SET_1, false, "arch", 'a', OptionParser::eRequiredArgument,
     nullptr, {}, 0, eArgTypeArchitecture,
     "Specify the architecture for the target."},
};

llvm::ArrayRef<OptionDefinition> OptionGroupArchitecture::GetDefinitions() {
  return llvm::ArrayRef(g_option_table);
}

bool OptionGroupArchitecture::GetArchitecture(Platform *platform,
                                              ArchSpec &arch) {
  arch = Platform::GetAugmentedArchSpec(platform, m_arch_str);
  return arch.IsValid();
}

Status
OptionGroupArchitecture::SetOptionValue(uint32_t option_idx,
                                        llvm::StringRef option_arg,
                                        ExecutionContext *execution_context) {
  Status error;
  const int short_option = g_option_table[option_idx].short_option;

  switch (short_option) {
  case 'a':
    m_arch_str.assign(std::string(option_arg));
    break;

  default:
    llvm_unreachable("Unimplemented option");
  }

  return error;
}

void OptionGroupArchitecture::OptionParsingStarting(
    ExecutionContext *execution_context) {
  m_arch_str.clear();
}
