//===-- OptionValueString.cpp ---------------------------------------------===//
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

#include "lldb/Interpreter/OptionValueString.h"

#include "lldb/Host/OptionParser.h"
#include "lldb/Interpreter/OptionValue.h"
#include "lldb/Utility/Args.h"
#include "lldb/Utility/Stream.h"

using namespace lldb;
using namespace lldb_private;

static void DumpString(Stream &strm, const std::string &str, bool escape,
                       bool raw) {
  if (escape) {
    std::string escaped_str;
    Args::ExpandEscapedCharacters(str.c_str(), escaped_str);
    DumpString(strm, escaped_str, false, raw);
    return;
  }

  if (raw)
    strm.PutCString(str);
  else
    strm.QuotedCString(str.c_str());
}

void OptionValueString::DumpValue(const ExecutionContext *exe_ctx, Stream &strm,
                                  uint32_t dump_mask) {
  if (dump_mask & eDumpOptionType)
    strm.Printf("(%s)", GetTypeAsCString());
  if (dump_mask & eDumpOptionValue) {
    if (dump_mask & eDumpOptionType)
      strm.PutCString(" = ");
    const bool escape = m_options.Test(eOptionEncodeCharacterEscapeSequences);
    const bool raw = dump_mask & eDumpOptionRaw;
    if (!m_current_value.empty() || m_value_was_set)
      DumpString(strm, m_current_value, escape, raw);

    if (dump_mask & eDumpOptionDefaultValue &&
        m_current_value != m_default_value && !m_default_value.empty()) {
      DefaultValueFormat label(strm);
      DumpString(strm, m_default_value, escape, raw);
    }
  }
}

Status OptionValueString::SetValueFromString(llvm::StringRef value,
                                             VarSetOperationType op) {
  Status error;

  std::string value_str = value.str();
  value = value.trim();
  if (value.size() > 0) {
    switch (value.front()) {
    case '"':
    case '\'': {
      if (value.size() <= 1 || value.back() != value.front()) {
        error = Status::FromErrorString("mismatched quotes");
        return error;
      }
      value = value.drop_front().drop_back();
    } break;
    }
    value_str = value.str();
  }

  switch (op) {
  case eVarSetOperationInvalid:
  case eVarSetOperationInsertBefore:
  case eVarSetOperationInsertAfter:
  case eVarSetOperationRemove:
    if (m_validator) {
      error = m_validator(value_str.c_str(), m_validator_baton);
      if (error.Fail())
        return error;
    }
    error = OptionValue::SetValueFromString(value, op);
    break;

  case eVarSetOperationAppend: {
    std::string new_value(m_current_value);
    if (value.size() > 0) {
      if (m_options.Test(eOptionEncodeCharacterEscapeSequences)) {
        std::string str;
        Args::EncodeEscapeSequences(value_str.c_str(), str);
        new_value.append(str);
      } else
        new_value.append(std::string(value));
    }
    if (m_validator) {
      error = m_validator(new_value.c_str(), m_validator_baton);
      if (error.Fail())
        return error;
    }
    m_current_value.assign(new_value);
    NotifyValueChanged();
  } break;

  case eVarSetOperationClear:
    Clear();
    NotifyValueChanged();
    break;

  case eVarSetOperationReplace:
  case eVarSetOperationAssign:
    if (m_validator) {
      error = m_validator(value_str.c_str(), m_validator_baton);
      if (error.Fail())
        return error;
    }
    m_value_was_set = true;
    if (m_options.Test(eOptionEncodeCharacterEscapeSequences)) {
      Args::EncodeEscapeSequences(value_str.c_str(), m_current_value);
    } else {
      SetCurrentValue(value_str);
    }
    NotifyValueChanged();
    break;
  }
  return error;
}

Status OptionValueString::SetCurrentValue(llvm::StringRef value) {
  if (m_validator) {
    Status error(m_validator(value.str().c_str(), m_validator_baton));
    if (error.Fail())
      return error;
  }
  m_current_value.assign(std::string(value));
  return Status();
}

Status OptionValueString::AppendToCurrentValue(const char *value) {
  if (value && value[0]) {
    if (m_validator) {
      std::string new_value(m_current_value);
      new_value.append(value);
      Status error(m_validator(value, m_validator_baton));
      if (error.Fail())
        return error;
      m_current_value.assign(new_value);
    } else
      m_current_value.append(value);
  }
  return Status();
}
