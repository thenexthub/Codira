//===-- OptionValueUUID.h --------------------------------------*- C++ -*-===//
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

#ifndef LLDB_INTERPRETER_OPTIONVALUEUUID_H
#define LLDB_INTERPRETER_OPTIONVALUEUUID_H

#include "lldb/Utility/UUID.h"
#include "lldb/Interpreter/OptionValue.h"

namespace lldb_private {

class OptionValueUUID : public Cloneable<OptionValueUUID, OptionValue> {
public:
  OptionValueUUID() = default;

  OptionValueUUID(const UUID &uuid) : m_uuid(uuid) {}

  ~OptionValueUUID() override = default;

  // Virtual subclass pure virtual overrides

  OptionValue::Type GetType() const override { return eTypeUUID; }

  void DumpValue(const ExecutionContext *exe_ctx, Stream &strm,
                 uint32_t dump_mask) override;

  llvm::json::Value ToJSON(const ExecutionContext *exe_ctx) const override {
    return m_uuid.GetAsString();
  }

  Status
  SetValueFromString(llvm::StringRef value,
                     VarSetOperationType op = eVarSetOperationAssign) override;

  void Clear() override {
    m_uuid.Clear();
    m_value_was_set = false;
  }

  // Subclass specific functions

  UUID &GetCurrentValue() { return m_uuid; }

  const UUID &GetCurrentValue() const { return m_uuid; }

  void SetCurrentValue(const UUID &value) { m_uuid = value; }

  void AutoComplete(CommandInterpreter &interpreter,
                    CompletionRequest &request) override;

protected:
  UUID m_uuid;
};

} // namespace lldb_private

#endif // LLDB_INTERPRETER_OPTIONVALUEUUID_H
