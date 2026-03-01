//===-- OptionValueChar.h ---------------------------------------*- C++ -*-===//
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

#ifndef LLDB_INTERPRETER_OPTIONVALUECHAR_H
#define LLDB_INTERPRETER_OPTIONVALUECHAR_H

#include "lldb/Interpreter/OptionValue.h"

namespace lldb_private {

class OptionValueChar : public Cloneable<OptionValueChar, OptionValue> {
public:
  OptionValueChar(char value)
      : m_current_value(value), m_default_value(value) {}

  OptionValueChar(char current_value, char default_value)
      : m_current_value(current_value), m_default_value(default_value) {}

  ~OptionValueChar() override = default;

  // Virtual subclass pure virtual overrides

  OptionValue::Type GetType() const override { return eTypeChar; }

  void DumpValue(const ExecutionContext *exe_ctx, Stream &strm,
                 uint32_t dump_mask) override;

  llvm::json::Value ToJSON(const ExecutionContext *exe_ctx) const override {
    return m_current_value;
  }

  Status
  SetValueFromString(llvm::StringRef value,
                     VarSetOperationType op = eVarSetOperationAssign) override;

  void Clear() override {
    m_current_value = m_default_value;
    m_value_was_set = false;
  }

  // Subclass specific functions

  const char &operator=(char c) {
    m_current_value = c;
    return m_current_value;
  }

  char GetCurrentValue() const { return m_current_value; }

  char GetDefaultValue() const { return m_default_value; }

  void SetCurrentValue(char value) { m_current_value = value; }

  void SetDefaultValue(char value) { m_default_value = value; }

protected:
  char m_current_value;
  char m_default_value;
};

} // namespace lldb_private

#endif // LLDB_INTERPRETER_OPTIONVALUECHAR_H
