//===-- OptionValueArray.h --------------------------------------*- C++ -*-===//
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

#ifndef LLDB_INTERPRETER_OPTIONVALUEARRAY_H
#define LLDB_INTERPRETER_OPTIONVALUEARRAY_H

#include <vector>

#include "lldb/Interpreter/OptionValue.h"

namespace lldb_private {

class OptionValueArray : public Cloneable<OptionValueArray, OptionValue> {
public:
  OptionValueArray(uint32_t type_mask = UINT32_MAX, bool raw_value_dump = false)
      : m_type_mask(type_mask), m_raw_value_dump(raw_value_dump) {}

  ~OptionValueArray() override = default;

  // Virtual subclass pure virtual overrides

  OptionValue::Type GetType() const override { return eTypeArray; }

  void DumpValue(const ExecutionContext *exe_ctx, Stream &strm,
                 uint32_t dump_mask) override;

  llvm::json::Value ToJSON(const ExecutionContext *exe_ctx) const override;

  Status
  SetValueFromString(llvm::StringRef value,
                     VarSetOperationType op = eVarSetOperationAssign) override;

  void Clear() override {
    m_values.clear();
    m_value_was_set = false;
  }

  lldb::OptionValueSP
  DeepCopy(const lldb::OptionValueSP &new_parent) const override;

  bool IsAggregateValue() const override { return true; }

  lldb::OptionValueSP GetSubValue(const ExecutionContext *exe_ctx,
                                  llvm::StringRef name,
                                  Status &error) const override;

  // Subclass specific functions

  size_t GetSize() const { return m_values.size(); }

  lldb::OptionValueSP operator[](size_t idx) const {
    lldb::OptionValueSP value_sp;
    if (idx < m_values.size())
      value_sp = m_values[idx];
    return value_sp;
  }

  lldb::OptionValueSP GetValueAtIndex(size_t idx) const {
    lldb::OptionValueSP value_sp;
    if (idx < m_values.size())
      value_sp = m_values[idx];
    return value_sp;
  }

  bool AppendValue(const lldb::OptionValueSP &value_sp) {
    // Make sure the value_sp object is allowed to contain values of the type
    // passed in...
    if (value_sp && (m_type_mask & value_sp->GetTypeAsMask())) {
      m_values.push_back(value_sp);
      return true;
    }
    return false;
  }

  bool InsertValue(size_t idx, const lldb::OptionValueSP &value_sp) {
    // Make sure the value_sp object is allowed to contain values of the type
    // passed in...
    if (value_sp && (m_type_mask & value_sp->GetTypeAsMask())) {
      if (idx < m_values.size())
        m_values.insert(m_values.begin() + idx, value_sp);
      else
        m_values.push_back(value_sp);
      return true;
    }
    return false;
  }

  bool ReplaceValue(size_t idx, const lldb::OptionValueSP &value_sp) {
    // Make sure the value_sp object is allowed to contain values of the type
    // passed in...
    if (value_sp && (m_type_mask & value_sp->GetTypeAsMask())) {
      if (idx < m_values.size()) {
        m_values[idx] = value_sp;
        return true;
      }
    }
    return false;
  }

  bool DeleteValue(size_t idx) {
    if (idx < m_values.size()) {
      m_values.erase(m_values.begin() + idx);
      return true;
    }
    return false;
  }

  size_t GetArgs(Args &args) const;

  Status SetArgs(const Args &args, VarSetOperationType op);

protected:
  typedef std::vector<lldb::OptionValueSP> collection;

  uint32_t m_type_mask;
  collection m_values;
  bool m_raw_value_dump;
};

} // namespace lldb_private

#endif // LLDB_INTERPRETER_OPTIONVALUEARRAY_H
