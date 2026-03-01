//===-- OptionValueArch.cpp -----------------------------------------------===//
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

#include "lldb/Interpreter/OptionValueArch.h"

#include "lldb/DataFormatters/FormatManager.h"
#include "lldb/Interpreter/CommandCompletions.h"
#include "lldb/Interpreter/CommandInterpreter.h"
#include "lldb/Interpreter/OptionValue.h"
#include "lldb/Utility/Args.h"
#include "lldb/Utility/State.h"

using namespace lldb;
using namespace lldb_private;

void OptionValueArch::DumpValue(const ExecutionContext *exe_ctx, Stream &strm,
                                uint32_t dump_mask) {
  if (dump_mask & eDumpOptionType)
    strm.Printf("(%s)", GetTypeAsCString());
  if (dump_mask & eDumpOptionValue) {
    if (dump_mask & eDumpOptionType)
      strm.PutCString(" = ");

    if (m_current_value.IsValid()) {
      const char *arch_name = m_current_value.GetArchitectureName();
      if (arch_name)
        strm.PutCString(arch_name);
    }

    if (dump_mask & eDumpOptionDefaultValue &&
        m_current_value != m_default_value && m_default_value.IsValid()) {
      DefaultValueFormat label(strm);
      strm.PutCString(m_default_value.GetArchitectureName());
    }
  }
}

llvm::json::Value
OptionValueArch::ToJSON(const ExecutionContext *exe_ctx) const {
  if (m_current_value.IsValid())
    return llvm::json::Value(m_current_value.GetArchitectureName());

  return {};
}

Status OptionValueArch::SetValueFromString(llvm::StringRef value,
                                           VarSetOperationType op) {
  Status error;
  switch (op) {
  case eVarSetOperationClear:
    Clear();
    NotifyValueChanged();
    break;

  case eVarSetOperationReplace:
  case eVarSetOperationAssign: {
    std::string value_str = value.trim().str();
    if (m_current_value.SetTriple(value_str.c_str())) {
      m_value_was_set = true;
      NotifyValueChanged();
    } else
      error = Status::FromErrorStringWithFormat("unsupported architecture '%s'",
                                                value_str.c_str());
    break;
  }
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

void OptionValueArch::AutoComplete(CommandInterpreter &interpreter,
                                   CompletionRequest &request) {
  lldb_private::CommandCompletions::InvokeCommonCompletionCallbacks(
      interpreter, lldb::eArchitectureCompletion, request, nullptr);
}
