//===-- OptionValueDictionary.h ---------------------------------*- C++ -*-===//
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

#ifndef LLDB_INTERPRETER_OPTIONVALUEDICTIONARY_H
#define LLDB_INTERPRETER_OPTIONVALUEDICTIONARY_H

#include "lldb/Interpreter/OptionValue.h"
#include "lldb/lldb-private-types.h"

#include "llvm/ADT/StringMap.h"

namespace lldb_private {

class OptionValueDictionary
    : public Cloneable<OptionValueDictionary, OptionValue> {
public:
  OptionValueDictionary(uint32_t type_mask = UINT32_MAX,
                        OptionEnumValues enum_values = OptionEnumValues(),
                        bool raw_value_dump = true)
      : m_type_mask(type_mask), m_enum_values(enum_values),
        m_raw_value_dump(raw_value_dump) {}

  ~OptionValueDictionary() override = default;

  // Virtual subclass pure virtual overrides

  OptionValue::Type GetType() const override { return eTypeDictionary; }

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

  bool IsHomogenous() const {
    return ConvertTypeMaskToType(m_type_mask) != eTypeInvalid;
  }

  // Subclass specific functions

  size_t GetNumValues() const { return m_values.size(); }

  lldb::OptionValueSP GetValueForKey(llvm::StringRef key) const;

  lldb::OptionValueSP GetSubValue(const ExecutionContext *exe_ctx,
                                  llvm::StringRef name,
                                  Status &error) const override;

  Status SetSubValue(const ExecutionContext *exe_ctx, VarSetOperationType op,
                     llvm::StringRef name, llvm::StringRef value) override;

  bool SetValueForKey(llvm::StringRef key, const lldb::OptionValueSP &value_sp,
                      bool can_replace = true);

  bool DeleteValueForKey(llvm::StringRef key);

  size_t GetArgs(Args &args) const;

  Status SetArgs(const Args &args, VarSetOperationType op);

protected:
  uint32_t m_type_mask;
  OptionEnumValues m_enum_values;
  llvm::StringMap<lldb::OptionValueSP> m_values;
  bool m_raw_value_dump;
};

} // namespace lldb_private

#endif // LLDB_INTERPRETER_OPTIONVALUEDICTIONARY_H
