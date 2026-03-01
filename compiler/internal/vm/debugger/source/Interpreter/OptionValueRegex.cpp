//===-- OptionValueRegex.cpp ----------------------------------------------===//
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

#include "lldb/Interpreter/OptionValueRegex.h"

#include "lldb/Interpreter/OptionValue.h"
#include "lldb/Utility/Stream.h"

using namespace lldb;
using namespace lldb_private;

void OptionValueRegex::DumpValue(const ExecutionContext *exe_ctx, Stream &strm,
                                 uint32_t dump_mask) {
  if (dump_mask & eDumpOptionType)
    strm.Printf("(%s)", GetTypeAsCString());
  if (dump_mask & eDumpOptionValue) {
    if (dump_mask & eDumpOptionType)
      strm.PutCString(" = ");
    if (m_regex.IsValid()) {
      llvm::StringRef regex_text = m_regex.GetText();
      strm.Printf("%s", regex_text.str().c_str());
    }
    if (dump_mask & eDumpOptionDefaultValue &&
        m_regex.GetText() != m_default_regex_str &&
        !m_default_regex_str.empty()) {
      DefaultValueFormat label(strm);
      strm.PutCString(m_default_regex_str);
    }
  }
}

Status OptionValueRegex::SetValueFromString(llvm::StringRef value,
                                            VarSetOperationType op) {
  Status error;
  switch (op) {
  case eVarSetOperationInvalid:
  case eVarSetOperationInsertBefore:
  case eVarSetOperationInsertAfter:
  case eVarSetOperationRemove:
  case eVarSetOperationAppend:
    error = OptionValue::SetValueFromString(value, op);
    break;

  case eVarSetOperationClear:
    Clear();
    NotifyValueChanged();
    break;

  case eVarSetOperationReplace:
  case eVarSetOperationAssign:
    m_regex = RegularExpression(value);
    if (m_regex.IsValid()) {
      m_value_was_set = true;
      NotifyValueChanged();
    } else if (llvm::Error err = m_regex.GetError()) {
      return Status::FromError(std::move(err));
    } else {
      return Status::FromErrorString("regex error");
    }
    break;
  }
  return error;
}
