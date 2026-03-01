//===-- OptionValuePathMappings.h -------------------------------*- C++ -*-===//
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

#ifndef LLDB_INTERPRETER_OPTIONVALUEPATHMAPPINGS_H
#define LLDB_INTERPRETER_OPTIONVALUEPATHMAPPINGS_H

#include "lldb/Interpreter/OptionValue.h"
#include "lldb/Target/PathMappingList.h"

namespace lldb_private {

class OptionValuePathMappings
    : public Cloneable<OptionValuePathMappings, OptionValue> {
public:
  OptionValuePathMappings(bool notify_changes)
      : m_notify_changes(notify_changes) {}

  ~OptionValuePathMappings() override = default;

  // Virtual subclass pure virtual overrides

  OptionValue::Type GetType() const override { return eTypePathMap; }

  void DumpValue(const ExecutionContext *exe_ctx, Stream &strm,
                 uint32_t dump_mask) override;

  llvm::json::Value ToJSON(const ExecutionContext *exe_ctx) const override;

  Status
  SetValueFromString(llvm::StringRef value,
                     VarSetOperationType op = eVarSetOperationAssign) override;

  void Clear() override {
    m_path_mappings.Clear(m_notify_changes);
    m_value_was_set = false;
  }

  bool IsAggregateValue() const override { return true; }

  // Subclass specific functions

  PathMappingList &GetCurrentValue() { return m_path_mappings; }

  const PathMappingList &GetCurrentValue() const { return m_path_mappings; }

protected:
  PathMappingList m_path_mappings;
  bool m_notify_changes;
};

} // namespace lldb_private

#endif // LLDB_INTERPRETER_OPTIONVALUEPATHMAPPINGS_H
