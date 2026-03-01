//===-- OptionValueEnumeration.h --------------------------------*- C++ -*-===//
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

#ifndef LLDB_INTERPRETER_OPTIONVALUEENUMERATION_H
#define LLDB_INTERPRETER_OPTIONVALUEENUMERATION_H

#include "lldb/Core/UniqueCStringMap.h"
#include "lldb/Interpreter/OptionValue.h"
#include "lldb/Utility/ConstString.h"
#include "lldb/Utility/Status.h"
#include "lldb/Utility/Stream.h"
#include "lldb/Utility/StreamString.h"
#include "lldb/lldb-private-types.h"

namespace lldb_private {

class OptionValueEnumeration
    : public Cloneable<OptionValueEnumeration, OptionValue> {
public:
  typedef int64_t enum_type;
  struct EnumeratorInfo {
    enum_type value;
    const char *description;
  };
  typedef UniqueCStringMap<EnumeratorInfo> EnumerationMap;
  typedef EnumerationMap::Entry EnumerationMapEntry;

  OptionValueEnumeration(const OptionEnumValues &enumerators, enum_type value);

  ~OptionValueEnumeration() override = default;

  // Virtual subclass pure virtual overrides

  OptionValue::Type GetType() const override { return eTypeEnum; }

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

  void AutoComplete(CommandInterpreter &interpreter,
                    CompletionRequest &request) override;

  // Subclass specific functions

  enum_type operator=(enum_type value) {
    m_current_value = value;
    return m_current_value;
  }

  enum_type GetCurrentValue() const { return m_current_value; }

  enum_type GetDefaultValue() const { return m_default_value; }

  void SetCurrentValue(enum_type value) { m_current_value = value; }

  void SetDefaultValue(enum_type value) { m_default_value = value; }

protected:
  void SetEnumerations(const OptionEnumValues &enumerators);
  void DumpEnum(Stream &strm, enum_type value);

  enum_type m_current_value;
  enum_type m_default_value;
  EnumerationMap m_enumerations;
};

} // namespace lldb_private

#endif // LLDB_INTERPRETER_OPTIONVALUEENUMERATION_H
