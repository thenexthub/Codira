//===-- OptionValueLanguage.h -------------------------------------*- C++
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

#ifndef LLDB_INTERPRETER_OPTIONVALUELANGUAGE_H
#define LLDB_INTERPRETER_OPTIONVALUELANGUAGE_H

#include "lldb/Interpreter/OptionValue.h"
#include "lldb/lldb-enumerations.h"

namespace lldb_private {

class OptionValueLanguage : public Cloneable<OptionValueLanguage, OptionValue> {
public:
  OptionValueLanguage(lldb::LanguageType value)
      : m_current_value(value), m_default_value(value) {}

  OptionValueLanguage(lldb::LanguageType current_value,
                      lldb::LanguageType default_value)
      : m_current_value(current_value), m_default_value(default_value) {}

  ~OptionValueLanguage() override = default;

  // Virtual subclass pure virtual overrides

  OptionValue::Type GetType() const override { return eTypeLanguage; }

  void DumpValue(const ExecutionContext *exe_ctx, Stream &strm,
                 uint32_t dump_mask) override;

  llvm::json::Value ToJSON(const ExecutionContext *exe_ctx) const override;

  Status
  SetValueFromString(llvm::StringRef value,
                     VarSetOperationType op = eVarSetOperationAssign) override;

  void Clear() override {
    m_current_value = m_default_value;
    m_value_was_set = false;
  }

  // Subclass specific functions

  lldb::LanguageType GetCurrentValue() const { return m_current_value; }

  lldb::LanguageType GetDefaultValue() const { return m_default_value; }

  void SetCurrentValue(lldb::LanguageType value) { m_current_value = value; }

  void SetDefaultValue(lldb::LanguageType value) { m_default_value = value; }

protected:
  lldb::LanguageType m_current_value;
  lldb::LanguageType m_default_value;
};

} // namespace lldb_private

#endif // LLDB_INTERPRETER_OPTIONVALUELANGUAGE_H
