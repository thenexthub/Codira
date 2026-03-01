//===-- OptionValueBoolean.cpp --------------------------------------------===//
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

#include "lldb/Interpreter/OptionValueBoolean.h"

#include "lldb/Host/PosixApi.h"
#include "lldb/Interpreter/OptionArgParser.h"
#include "lldb/Interpreter/OptionValue.h"
#include "lldb/Utility/Stream.h"
#include "lldb/Utility/StringList.h"
#include "llvm/ADT/STLExtras.h"

using namespace lldb;
using namespace lldb_private;

void OptionValueBoolean::DumpValue(const ExecutionContext *exe_ctx,
                                   Stream &strm, uint32_t dump_mask) {
  if (dump_mask & eDumpOptionType)
    strm.Printf("(%s)", GetTypeAsCString());
  //    if (dump_mask & eDumpOptionName)
  //        DumpQualifiedName (strm);
  if (dump_mask & eDumpOptionValue) {
    if (dump_mask & eDumpOptionType)
      strm.PutCString(" = ");
    strm.PutCString(m_current_value ? "true" : "false");
    if (dump_mask & eDumpOptionDefaultValue &&
        m_current_value != m_default_value) {
      DefaultValueFormat label(strm);
      strm.PutCString(m_default_value ? "true" : "false");
    }
  }
}

Status OptionValueBoolean::SetValueFromString(llvm::StringRef value_str,
                                              VarSetOperationType op) {
  Status error;
  switch (op) {
  case eVarSetOperationClear:
    Clear();
    NotifyValueChanged();
    break;

  case eVarSetOperationReplace:
  case eVarSetOperationAssign: {
    bool success = false;
    bool value = OptionArgParser::ToBoolean(value_str, false, &success);
    if (success) {
      m_value_was_set = true;
      m_current_value = value;
      NotifyValueChanged();
    } else {
      if (value_str.size() == 0)
        error = Status::FromErrorString("invalid boolean string value <empty>");
      else
        error = Status::FromErrorStringWithFormat(
            "invalid boolean string value: '%s'", value_str.str().c_str());
    }
  } break;

  case eVarSetOperationInsertBefore:
  case eVarSetOperationInsertAfter:
  case eVarSetOperationRemove:
  case eVarSetOperationAppend:
  case eVarSetOperationInvalid:
    error = OptionValue::SetValueFromString(value_str, op);
    break;
  }
  return error;
}

void OptionValueBoolean::AutoComplete(CommandInterpreter &interpreter,
                                      CompletionRequest &request) {
  llvm::StringRef autocomplete_entries[] = {"true", "false", "on", "off",
                                            "yes",  "no",    "1",  "0"};

  auto entries = llvm::ArrayRef(autocomplete_entries);

  // only suggest "true" or "false" by default
  if (request.GetCursorArgumentPrefix().empty())
    entries = entries.take_front(2);

  for (auto entry : entries)
    request.TryCompleteCurrentArg(entry);
}
