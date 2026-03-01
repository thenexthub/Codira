//===-- OptionValueSInt64.cpp ---------------------------------------------===//
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

#include "lldb/Interpreter/OptionValueSInt64.h"

#include "lldb/Interpreter/OptionValue.h"
#include "lldb/Utility/Stream.h"

using namespace lldb;
using namespace lldb_private;

void OptionValueSInt64::DumpValue(const ExecutionContext *exe_ctx, Stream &strm,
                                  uint32_t dump_mask) {
  // printf ("%p: DumpValue (exe_ctx=%p, strm, mask) m_current_value = %"
  // PRIi64
  // "\n", this, exe_ctx, m_current_value);
  if (dump_mask & eDumpOptionType)
    strm.Printf("(%s)", GetTypeAsCString());
  //    if (dump_mask & eDumpOptionName)
  //        DumpQualifiedName (strm);
  if (dump_mask & eDumpOptionValue) {
    if (dump_mask & eDumpOptionType)
      strm.PutCString(" = ");
    strm.Printf("%" PRIi64, m_current_value);
    if (dump_mask & eDumpOptionDefaultValue &&
        m_current_value != m_default_value) {
      DefaultValueFormat label(strm);
      strm.Printf("%" PRIi64, m_default_value);
    }
  }
}

Status OptionValueSInt64::SetValueFromString(llvm::StringRef value_ref,
                                             VarSetOperationType op) {
  Status error;
  switch (op) {
  case eVarSetOperationClear:
    Clear();
    NotifyValueChanged();
    break;

  case eVarSetOperationReplace:
  case eVarSetOperationAssign: {
    llvm::StringRef value_trimmed = value_ref.trim();
    int64_t value;
    if (llvm::to_integer(value_trimmed, value)) {
      if (value >= m_min_value && value <= m_max_value) {
        m_value_was_set = true;
        m_current_value = value;
        NotifyValueChanged();
      } else
        error = Status::FromErrorStringWithFormat(
            "%" PRIi64 " is out of range, valid values must be between %" PRIi64
            " and %" PRIi64 ".",
            value, m_min_value, m_max_value);
    } else {
      error = Status::FromErrorStringWithFormat(
          "invalid int64_t string value: '%s'", value_ref.str().c_str());
    }
  } break;

  case eVarSetOperationInsertBefore:
  case eVarSetOperationInsertAfter:
  case eVarSetOperationRemove:
  case eVarSetOperationAppend:
  case eVarSetOperationInvalid:
    error = OptionValue::SetValueFromString(value_ref, op);
    break;
  }
  return error;
}
