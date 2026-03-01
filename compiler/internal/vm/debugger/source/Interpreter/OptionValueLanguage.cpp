//===-- OptionValueLanguage.cpp -------------------------------------------===//
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

#include "lldb/Interpreter/OptionValueLanguage.h"

#include "lldb/DataFormatters/FormatManager.h"
#include "lldb/Interpreter/OptionValue.h"
#include "lldb/Symbol/TypeSystem.h"
#include "lldb/Target/Language.h"
#include "lldb/Utility/Args.h"
#include "lldb/Utility/Stream.h"

using namespace lldb;
using namespace lldb_private;

void OptionValueLanguage::DumpValue(const ExecutionContext *exe_ctx,
                                    Stream &strm, uint32_t dump_mask) {
  if (dump_mask & eDumpOptionType)
    strm.Printf("(%s)", GetTypeAsCString());
  if (dump_mask & eDumpOptionValue) {
    if (dump_mask & eDumpOptionType)
      strm.PutCString(" = ");
    if (m_current_value != eLanguageTypeUnknown)
      strm.PutCString(Language::GetNameForLanguageType(m_current_value));
    if (dump_mask & eDumpOptionDefaultValue &&
        m_current_value != m_default_value &&
        m_default_value != eLanguageTypeUnknown) {
      DefaultValueFormat label(strm);
      strm.PutCString(Language::GetNameForLanguageType(m_default_value));
    }
  }
}

llvm::json::Value
OptionValueLanguage::ToJSON(const ExecutionContext *exe_ctx) const {
  return Language::GetNameForLanguageType(m_current_value);
}

Status OptionValueLanguage::SetValueFromString(llvm::StringRef value,
                                               VarSetOperationType op) {
  Status error;
  switch (op) {
  case eVarSetOperationClear:
    Clear();
    break;

  case eVarSetOperationReplace:
  case eVarSetOperationAssign: {
    LanguageSet languages_for_types = Language::GetLanguagesSupportingTypeSystems();
    LanguageType new_type = Language::GetLanguageTypeFromString(value.trim());
    if (new_type && languages_for_types[new_type]) {
      m_value_was_set = true;
      m_current_value = new_type;
    } else {
      StreamString error_strm;
      error_strm.Printf("invalid language type '%s', ", value.str().c_str());
      error_strm.Printf("valid values are:\n");
      for (int bit : languages_for_types.bitvector.set_bits()) {
        auto language = (LanguageType)bit;
        error_strm.Printf("    %s\n",
                          Language::GetNameForLanguageType(language));
      }
      error = Status(error_strm.GetString().str());
    }
  } break;

  case eVarSetOperationInsertBefore:
  case eVarSetOperationInsertAfter:
  case eVarSetOperationRemove:
  case eVarSetOperationAppend:
  case eVarSetOperationInvalid:
    error = OptionValue::SetValueFromString(value, op);
    break;
  }
  return error;
}
