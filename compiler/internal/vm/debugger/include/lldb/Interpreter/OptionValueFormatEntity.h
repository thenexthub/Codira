//===-- OptionValueFormatEntity.h --------------------------------*- C++-*-===//
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

#ifndef LLDB_INTERPRETER_OPTIONVALUEFORMATENTITY_H
#define LLDB_INTERPRETER_OPTIONVALUEFORMATENTITY_H

#include "lldb/Core/FormatEntity.h"
#include "lldb/Interpreter/OptionValue.h"

namespace lldb_private {

class OptionValueFormatEntity
    : public Cloneable<OptionValueFormatEntity, OptionValue> {
public:
  OptionValueFormatEntity(const char *default_format);

  ~OptionValueFormatEntity() override = default;

  OptionValue::Type GetType() const override { return eTypeFormatEntity; }

  void DumpValue(const ExecutionContext *exe_ctx, Stream &strm,
                 uint32_t dump_mask) override;

  llvm::json::Value ToJSON(const ExecutionContext *exe_ctx) const override;

  Status
  SetValueFromString(llvm::StringRef value,
                     VarSetOperationType op = eVarSetOperationAssign) override;

  void Clear() override;

  void AutoComplete(CommandInterpreter &interpreter,
                    CompletionRequest &request) override;

  const FormatEntity::Entry &GetCurrentValue() const { return m_current_entry; }

  void SetCurrentValue(const FormatEntity::Entry &value) {
    m_current_entry = value;
  }

  const FormatEntity::Entry &GetDefaultValue() const { return m_default_entry; }

protected:
  std::string m_current_format;
  std::string m_default_format;
  FormatEntity::Entry m_current_entry;
  FormatEntity::Entry m_default_entry;
};

} // namespace lldb_private

#endif // LLDB_INTERPRETER_OPTIONVALUEFORMATENTITY_H
