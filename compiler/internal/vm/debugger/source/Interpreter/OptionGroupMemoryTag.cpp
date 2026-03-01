//===-- OptionGroupMemoryTag.cpp -----------------------------------------===//
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

#include "lldb/Interpreter/OptionGroupMemoryTag.h"

#include "lldb/Host/OptionParser.h"

using namespace lldb;
using namespace lldb_private;

static const uint32_t SHORT_OPTION_SHOW_TAGS = 0x54414753; // 'tags'

OptionGroupMemoryTag::OptionGroupMemoryTag(bool note_binary /*=false*/)
    : m_show_tags(false, false), m_option_definition{
                                     LLDB_OPT_SET_1,
                                     false,
                                     "show-tags",
                                     SHORT_OPTION_SHOW_TAGS,
                                     OptionParser::eNoArgument,
                                     nullptr,
                                     {},
                                     0,
                                     eArgTypeNone,
                                     note_binary
                                         ? "Include memory tags in output "
                                           "(does not apply to binary output)."
                                         : "Include memory tags in output."} {}

llvm::ArrayRef<OptionDefinition> OptionGroupMemoryTag::GetDefinitions() {
  return llvm::ArrayRef(m_option_definition);
}

Status
OptionGroupMemoryTag::SetOptionValue(uint32_t option_idx,
                                     llvm::StringRef option_arg,
                                     ExecutionContext *execution_context) {
  assert(option_idx == 0 && "Only one option in memory tag group!");

  switch (m_option_definition.short_option) {
  case SHORT_OPTION_SHOW_TAGS:
    m_show_tags.SetCurrentValue(true);
    m_show_tags.SetOptionWasSet();
    break;

  default:
    llvm_unreachable("Unimplemented option");
  }

  return {};
}

void OptionGroupMemoryTag::OptionParsingStarting(
    ExecutionContext *execution_context) {
  m_show_tags.Clear();
}
