//===-- OptionValueBoolean.h ------------------------------------*- C++ -*-===//
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

#ifndef LLDB_INTERPRETER_OPTIONVALUEBOOLEAN_H
#define LLDB_INTERPRETER_OPTIONVALUEBOOLEAN_H

#include "lldb/Interpreter/OptionValue.h"

namespace lldb_private {

class OptionValueBoolean : public Cloneable<OptionValueBoolean, OptionValue> {
public:
  OptionValueBoolean(bool value)
      : m_current_value(value), m_default_value(value) {}
  OptionValueBoolean(bool current_value, bool default_value)
      : m_current_value(current_value), m_default_value(default_value) {}

  ~OptionValueBoolean() override = default;

  // Virtual subclass pure virtual overrides

  OptionValue::Type GetType() const override { return eTypeBoolean; }

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

  void AutoComplete(CommandInterpreter &interpreter,
                    CompletionRequest &request) override;

  // Subclass specific functions

  /// Convert to bool operator.
  ///
  /// This allows code to check a OptionValueBoolean in conditions.
  ///
  /// \code
  /// OptionValueBoolean bool_value(...);
  /// if (bool_value)
  /// { ...
  /// \endcode
  ///
  /// \return
  ///     /b True this object contains a valid namespace decl, \b
  ///     false otherwise.
  explicit operator bool() const { return m_current_value; }

  const bool &operator=(bool b) {
    m_current_value = b;
    return m_current_value;
  }

  bool GetCurrentValue() const { return m_current_value; }

  bool GetDefaultValue() const { return m_default_value; }

  void SetCurrentValue(bool value) { m_current_value = value; }

  void SetDefaultValue(bool value) { m_default_value = value; }

protected:
  bool m_current_value;
  bool m_default_value;
};

} // namespace lldb_private

#endif // LLDB_INTERPRETER_OPTIONVALUEBOOLEAN_H
