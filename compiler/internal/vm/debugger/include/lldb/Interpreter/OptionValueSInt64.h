//===-- OptionValueSInt64.h --------------------------------------*- C++
//-*-===//
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

#ifndef LLDB_INTERPRETER_OPTIONVALUESINT64_H
#define LLDB_INTERPRETER_OPTIONVALUESINT64_H

#include "lldb/Interpreter/OptionValue.h"

namespace lldb_private {

class OptionValueSInt64 : public Cloneable<OptionValueSInt64, OptionValue> {
public:
  OptionValueSInt64() = default;

  OptionValueSInt64(int64_t value)
      : m_current_value(value), m_default_value(value) {}

  OptionValueSInt64(int64_t current_value, int64_t default_value)
      : m_current_value(current_value), m_default_value(default_value) {}

  OptionValueSInt64(const OptionValueSInt64 &rhs) = default;

  ~OptionValueSInt64() override = default;

  // Virtual subclass pure virtual overrides

  OptionValue::Type GetType() const override { return eTypeSInt64; }

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

  const int64_t &operator=(int64_t value) {
    m_current_value = value;
    return m_current_value;
  }

  int64_t GetCurrentValue() const { return m_current_value; }

  int64_t GetDefaultValue() const { return m_default_value; }

  bool SetCurrentValue(int64_t value) {
    if (value >= m_min_value && value <= m_max_value) {
      m_current_value = value;
      return true;
    }
    return false;
  }

  bool SetDefaultValue(int64_t value) {
    assert(value >= m_min_value && value <= m_max_value &&
           "disallowed default value");
    m_default_value = value;
    return true;
  }

  void SetMinimumValue(int64_t v) { m_min_value = v; }

  int64_t GetMinimumValue() const { return m_min_value; }

  void SetMaximumValue(int64_t v) { m_max_value = v; }

  int64_t GetMaximumValue() const { return m_max_value; }

protected:
  int64_t m_current_value = 0;
  int64_t m_default_value = 0;
  int64_t m_min_value = std::numeric_limits<int64_t>::min();
  int64_t m_max_value = std::numeric_limits<int64_t>::max();
};

} // namespace lldb_private

#endif // LLDB_INTERPRETER_OPTIONVALUESINT64_H
