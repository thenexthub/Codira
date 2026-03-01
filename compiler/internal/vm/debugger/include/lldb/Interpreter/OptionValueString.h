//===-- OptionValueString.h -------------------------------------*- C++ -*-===//
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

#ifndef LLDB_INTERPRETER_OPTIONVALUESTRING_H
#define LLDB_INTERPRETER_OPTIONVALUESTRING_H

#include <string>

#include "lldb/Utility/Flags.h"

#include "lldb/Interpreter/OptionValue.h"

namespace lldb_private {

class OptionValueString : public Cloneable<OptionValueString, OptionValue> {
public:
  typedef Status (*ValidatorCallback)(const char *string, void *baton);

  enum Options { eOptionEncodeCharacterEscapeSequences = (1u << 0) };

  OptionValueString() = default;

  OptionValueString(ValidatorCallback validator, void *baton = nullptr)
      : m_validator(validator), m_validator_baton(baton) {}

  OptionValueString(const char *value) {
    if (value && value[0]) {
      m_current_value.assign(value);
      m_default_value.assign(value);
    }
  }

  OptionValueString(const char *current_value, const char *default_value) {
    if (current_value && current_value[0])
      m_current_value.assign(current_value);
    if (default_value && default_value[0])
      m_default_value.assign(default_value);
  }

  OptionValueString(const char *value, ValidatorCallback validator,
                    void *baton = nullptr)
      : m_validator(validator), m_validator_baton(baton) {
    if (value && value[0]) {
      m_current_value.assign(value);
      m_default_value.assign(value);
    }
  }

  OptionValueString(const char *current_value, const char *default_value,
                    ValidatorCallback validator, void *baton = nullptr)
      : m_validator(validator), m_validator_baton(baton) {
    if (current_value && current_value[0])
      m_current_value.assign(current_value);
    if (default_value && default_value[0])
      m_default_value.assign(default_value);
  }

  ~OptionValueString() override = default;

  // Virtual subclass pure virtual overrides

  OptionValue::Type GetType() const override { return eTypeString; }

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

  Flags &GetOptions() { return m_options; }

  const Flags &GetOptions() const { return m_options; }

  const char *operator=(const char *value) {
    SetCurrentValue(value);
    return m_current_value.c_str();
  }

  const char *GetCurrentValue() const { return m_current_value.c_str(); }
  llvm::StringRef GetCurrentValueAsRef() const { return m_current_value; }

  const char *GetDefaultValue() const { return m_default_value.c_str(); }
  llvm::StringRef GetDefaultValueAsRef() const { return m_default_value; }

  Status SetCurrentValue(llvm::StringRef value);

  Status AppendToCurrentValue(const char *value);

  void SetDefaultValue(const char *value) {
    if (value && value[0])
      m_default_value.assign(value);
    else
      m_default_value.clear();
  }

  bool IsCurrentValueEmpty() const { return m_current_value.empty(); }

  bool IsDefaultValueEmpty() const { return m_default_value.empty(); }

  void SetValidator(ValidatorCallback validator, void *baton = nullptr) {
    m_validator = validator;
    m_validator_baton = baton;
  }

protected:
  std::string m_current_value;
  std::string m_default_value;
  Flags m_options;
  ValidatorCallback m_validator = nullptr;
  void *m_validator_baton = nullptr;
};

} // namespace lldb_private

#endif // LLDB_INTERPRETER_OPTIONVALUESTRING_H
