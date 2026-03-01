//===-- OptionValueChar.cpp -----------------------------------------------===//
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

#include "lldb/Interpreter/OptionValueChar.h"

#include "lldb/Interpreter/OptionArgParser.h"
#include "lldb/Interpreter/OptionValue.h"
#include "lldb/Utility/Stream.h"
#include "lldb/Utility/StringList.h"
#include "llvm/ADT/STLExtras.h"

using namespace lldb;
using namespace lldb_private;

static void DumpChar(Stream &strm, char value) {
  if (value != '\0')
    strm.PutChar(value);
  else
    strm.PutCString("(null)");
}

void OptionValueChar::DumpValue(const ExecutionContext *exe_ctx, Stream &strm,
                                uint32_t dump_mask) {
  if (dump_mask & eDumpOptionType)
    strm.Printf("(%s)", GetTypeAsCString());

  if (dump_mask & eDumpOptionValue) {
    if (dump_mask & eDumpOptionType)
      strm.PutCString(" = ");
    DumpChar(strm, m_current_value);
    if (dump_mask & eDumpOptionDefaultValue &&
        m_current_value != m_default_value) {
      DefaultValueFormat label(strm);
      DumpChar(strm, m_default_value);
    }
  }
}

Status OptionValueChar::SetValueFromString(llvm::StringRef value,
                                           VarSetOperationType op) {
  Status error;
  switch (op) {
  case eVarSetOperationClear:
    Clear();
    break;

  case eVarSetOperationReplace:
  case eVarSetOperationAssign: {
    bool success = false;
    char char_value = OptionArgParser::ToChar(value, '\0', &success);
    if (success) {
      m_current_value = char_value;
      m_value_was_set = true;
    } else
      return Status::FromErrorStringWithFormatv(
          "'{0}' cannot be longer than 1 character", value);
  } break;

  default:
    error = OptionValue::SetValueFromString(value, op);
    break;
  }
  return error;
}
