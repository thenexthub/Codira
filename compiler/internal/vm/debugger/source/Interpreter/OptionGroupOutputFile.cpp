//===-- OptionGroupOutputFile.cpp -----------------------------------------===//
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

#include "lldb/Interpreter/OptionGroupOutputFile.h"

#include "lldb/Host/OptionParser.h"

using namespace lldb;
using namespace lldb_private;

OptionGroupOutputFile::OptionGroupOutputFile() : m_append(false, false) {}

static const uint32_t SHORT_OPTION_APND = 0x61706e64; // 'apnd'

static constexpr OptionDefinition g_option_table[] = {
    {LLDB_OPT_SET_1, false, "outfile", 'o', OptionParser::eRequiredArgument,
     nullptr, {}, 0, eArgTypeFilename,
     "Specify a path for capturing command output."},
    {LLDB_OPT_SET_1, false, "append-outfile", SHORT_OPTION_APND,
     OptionParser::eNoArgument, nullptr, {}, 0, eArgTypeNone,
     "Append to the file specified with '--outfile <path>'."},
};

llvm::ArrayRef<OptionDefinition> OptionGroupOutputFile::GetDefinitions() {
  return llvm::ArrayRef(g_option_table);
}

Status
OptionGroupOutputFile::SetOptionValue(uint32_t option_idx,
                                      llvm::StringRef option_arg,
                                      ExecutionContext *execution_context) {
  Status error;
  const int short_option = g_option_table[option_idx].short_option;

  switch (short_option) {
  case 'o':
    error = m_file.SetValueFromString(option_arg);
    break;

  case SHORT_OPTION_APND:
    m_append.SetCurrentValue(true);
    break;

  default:
    llvm_unreachable("Unimplemented option");
  }

  return error;
}

void OptionGroupOutputFile::OptionParsingStarting(
    ExecutionContext *execution_context) {
  m_file.Clear();
  m_append.Clear();
}
