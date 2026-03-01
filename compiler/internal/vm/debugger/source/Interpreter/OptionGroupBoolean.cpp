//===-- OptionGroupBoolean.cpp --------------------------------------------===//
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

#include "lldb/Interpreter/OptionGroupBoolean.h"

#include "lldb/Host/OptionParser.h"

using namespace lldb;
using namespace lldb_private;

OptionGroupBoolean::OptionGroupBoolean(uint32_t usage_mask, bool required,
                                       const char *long_option,
                                       int short_option, const char *usage_text,
                                       bool default_value,
                                       bool no_argument_toggle_default)
    : m_value(default_value, default_value) {
  m_option_definition.usage_mask = usage_mask;
  m_option_definition.required = required;
  m_option_definition.long_option = long_option;
  m_option_definition.short_option = short_option;
  m_option_definition.validator = nullptr;
  m_option_definition.option_has_arg = no_argument_toggle_default
                                           ? OptionParser::eNoArgument
                                           : OptionParser::eRequiredArgument;
  m_option_definition.enum_values = {};
  m_option_definition.completion_type = 0;
  m_option_definition.argument_type = eArgTypeBoolean;
  m_option_definition.usage_text = usage_text;
}

Status OptionGroupBoolean::SetOptionValue(uint32_t option_idx,
                                          llvm::StringRef option_value,
                                          ExecutionContext *execution_context) {
  Status error;
  if (m_option_definition.option_has_arg == OptionParser::eNoArgument) {
    // Not argument, toggle the default value and mark the option as having
    // been set
    m_value.SetCurrentValue(!m_value.GetDefaultValue());
    m_value.SetOptionWasSet();
  } else {
    error = m_value.SetValueFromString(option_value);
  }
  return error;
}

void OptionGroupBoolean::OptionParsingStarting(
    ExecutionContext *execution_context) {
  m_value.Clear();
}
