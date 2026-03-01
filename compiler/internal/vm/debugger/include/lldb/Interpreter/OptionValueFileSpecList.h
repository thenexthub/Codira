//===-- OptionValueFileSpecList.h -------------------------------*- C++ -*-===//
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

#ifndef LLDB_INTERPRETER_OPTIONVALUEFILESPECLIST_H
#define LLDB_INTERPRETER_OPTIONVALUEFILESPECLIST_H

#include <mutex>

#include "lldb/Interpreter/OptionValue.h"
#include "lldb/Utility/FileSpecList.h"

namespace lldb_private {

class OptionValueFileSpecList
    : public Cloneable<OptionValueFileSpecList, OptionValue> {
public:
  OptionValueFileSpecList() = default;

  OptionValueFileSpecList(const OptionValueFileSpecList &other)
      : Cloneable(other), m_current_value(other.GetCurrentValue()) {}

  ~OptionValueFileSpecList() override = default;

  // Virtual subclass pure virtual overrides

  OptionValue::Type GetType() const override { return eTypeFileSpecList; }

  void DumpValue(const ExecutionContext *exe_ctx, Stream &strm,
                 uint32_t dump_mask) override;

  llvm::json::Value ToJSON(const ExecutionContext *exe_ctx) const override;

  Status
  SetValueFromString(llvm::StringRef value,
                     VarSetOperationType op = eVarSetOperationAssign) override;

  void Clear() override {
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    m_current_value.Clear();
    m_value_was_set = false;
  }

  bool IsAggregateValue() const override { return true; }

  // Subclass specific functions

  FileSpecList GetCurrentValue() const {
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    return m_current_value;
  }

  void SetCurrentValue(const FileSpecList &value) {
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    m_current_value = value;
  }

  void AppendCurrentValue(const FileSpec &value) {
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    m_current_value.Append(value);
  }

protected:
  lldb::OptionValueSP Clone() const override;

  mutable std::recursive_mutex m_mutex;
  FileSpecList m_current_value;
};

} // namespace lldb_private

#endif // LLDB_INTERPRETER_OPTIONVALUEFILESPECLIST_H
