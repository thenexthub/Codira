//===-- OptionValueRegex.h --------------------------------------*- C++ -*-===//
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

#ifndef LLDB_INTERPRETER_OPTIONVALUEREGEX_H
#define LLDB_INTERPRETER_OPTIONVALUEREGEX_H

#include "lldb/Interpreter/OptionValue.h"
#include "lldb/Utility/RegularExpression.h"

namespace lldb_private {

class OptionValueRegex : public Cloneable<OptionValueRegex, OptionValue> {
public:
  OptionValueRegex(const char *value = nullptr)
      : m_regex(value), m_default_regex_str(value) {}

  ~OptionValueRegex() override = default;

  // Virtual subclass pure virtual overrides

  OptionValue::Type GetType() const override { return eTypeRegex; }

  void DumpValue(const ExecutionContext *exe_ctx, Stream &strm,
                 uint32_t dump_mask) override;

  llvm::json::Value ToJSON(const ExecutionContext *exe_ctx) const override {
    return m_regex.GetText();
  }

  Status
  SetValueFromString(llvm::StringRef value,
                     VarSetOperationType op = eVarSetOperationAssign) override;

  void Clear() override {
    m_regex = RegularExpression(m_default_regex_str);
    m_value_was_set = false;
  }

  // Subclass specific functions
  const RegularExpression *GetCurrentValue() const {
    return (m_regex.IsValid() ? &m_regex : nullptr);
  }

  void SetCurrentValue(const char *value) {
    if (value && value[0])
      m_regex = RegularExpression(llvm::StringRef(value));
    else
      m_regex = RegularExpression();
  }

  bool IsValid() const { return m_regex.IsValid(); }

protected:
  RegularExpression m_regex;
  std::string m_default_regex_str;
};

} // namespace lldb_private

#endif // LLDB_INTERPRETER_OPTIONVALUEREGEX_H
