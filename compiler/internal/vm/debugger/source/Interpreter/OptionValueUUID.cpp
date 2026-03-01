//===-- OptionValueUUID.cpp -----------------------------------------------===//
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

#include "lldb/Interpreter/OptionValueUUID.h"

#include "lldb/Core/Module.h"
#include "lldb/Interpreter/CommandInterpreter.h"
#include "lldb/Utility/Stream.h"
#include "lldb/Utility/StringList.h"

using namespace lldb;
using namespace lldb_private;

void OptionValueUUID::DumpValue(const ExecutionContext *exe_ctx, Stream &strm,
                                uint32_t dump_mask) {
  if (dump_mask & eDumpOptionType)
    strm.Printf("(%s)", GetTypeAsCString());
  if (dump_mask & eDumpOptionValue) {
    if (dump_mask & eDumpOptionType)
      strm.PutCString(" = ");
    m_uuid.Dump(strm);
  }
}

Status OptionValueUUID::SetValueFromString(llvm::StringRef value,
                                           VarSetOperationType op) {
  Status error;
  switch (op) {
  case eVarSetOperationClear:
    Clear();
    NotifyValueChanged();
    break;

  case eVarSetOperationReplace:
  case eVarSetOperationAssign: {
    if (!m_uuid.SetFromStringRef(value))
      error = Status::FromErrorStringWithFormat(
          "invalid uuid string value '%s'", value.str().c_str());
    else {
      m_value_was_set = true;
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

void OptionValueUUID::AutoComplete(CommandInterpreter &interpreter,
                                   CompletionRequest &request) {
  ExecutionContext exe_ctx(interpreter.GetExecutionContext());
  Target *target = exe_ctx.GetTargetPtr();
  if (!target)
    return;
  auto prefix = request.GetCursorArgumentPrefix();
  llvm::SmallVector<uint8_t, 20> uuid_bytes;
  if (!UUID::DecodeUUIDBytesFromString(prefix, uuid_bytes).empty())
    return;
  const size_t num_modules = target->GetImages().GetSize();
  for (size_t i = 0; i < num_modules; ++i) {
    ModuleSP module_sp(target->GetImages().GetModuleAtIndex(i));
    if (!module_sp)
      continue;
    const UUID &module_uuid = module_sp->GetUUID();
    if (!module_uuid.IsValid())
      continue;
    request.TryCompleteCurrentArg(module_uuid.GetAsString());
  }
}
