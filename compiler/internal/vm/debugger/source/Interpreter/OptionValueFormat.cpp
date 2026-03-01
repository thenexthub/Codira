//===-- OptionValueFormat.cpp ---------------------------------------------===//
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

#include "lldb/Interpreter/OptionValueFormat.h"

#include "lldb/DataFormatters/FormatManager.h"
#include "lldb/Interpreter/OptionArgParser.h"
#include "lldb/Interpreter/OptionValue.h"
#include "lldb/Utility/Stream.h"

using namespace lldb;
using namespace lldb_private;

void OptionValueFormat::DumpValue(const ExecutionContext *exe_ctx, Stream &strm,
                                  uint32_t dump_mask) {
  if (dump_mask & eDumpOptionType)
    strm.Printf("(%s)", GetTypeAsCString());
  if (dump_mask & eDumpOptionValue) {
    if (dump_mask & eDumpOptionType)
      strm.PutCString(" = ");
    strm.PutCString(FormatManager::GetFormatAsCString(m_current_value));
    if (dump_mask & eDumpOptionDefaultValue &&
        m_current_value != m_default_value) {
      DefaultValueFormat label(strm);
      strm.PutCString(FormatManager::GetFormatAsCString(m_default_value));
    }
  }
}

llvm::json::Value
OptionValueFormat::ToJSON(const ExecutionContext *exe_ctx) const {
  return FormatManager::GetFormatAsCString(m_current_value);
}

Status OptionValueFormat::SetValueFromString(llvm::StringRef value,
                                             VarSetOperationType op) {
  Status error;
  switch (op) {
  case eVarSetOperationClear:
    Clear();
    NotifyValueChanged();
    break;

  case eVarSetOperationReplace:
  case eVarSetOperationAssign: {
    Format new_format;
    error = OptionArgParser::ToFormat(value.str().c_str(), new_format, nullptr);
    if (error.Success()) {
      m_value_was_set = true;
      m_current_value = new_format;
      NotifyValueChanged();
    }
  } break;

  case eVarSetOperationInsertBefore:
  case eVarSetOperationInsertAfter:
  case eVarSetOperationRemove:
  case eVarSetOperationAppend:
  case eVarSetOperationInvalid:
    error = OptionValue::SetValueFromString(value, op);
    break;
  }
  return error;
}
