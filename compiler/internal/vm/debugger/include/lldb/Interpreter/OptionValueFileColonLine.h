//===-- OptionValueFileColonLine.h ------------------------------*- C++ -*-===//
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

#ifndef LLDB_INTERPRETER_OPTIONVALUEFILECOLONLINE_H
#define LLDB_INTERPRETER_OPTIONVALUEFILECOLONLINE_H

#include "lldb/Interpreter/CommandCompletions.h"
#include "lldb/Interpreter/OptionValue.h"
#include "lldb/Utility/FileSpec.h"
#include "llvm/Support/Chrono.h"

namespace lldb_private {

class OptionValueFileColonLine :
    public Cloneable<OptionValueFileColonLine, OptionValue> {
public:
  OptionValueFileColonLine();
  OptionValueFileColonLine(const llvm::StringRef input);

  ~OptionValueFileColonLine() override = default;

  OptionValue::Type GetType() const override { return eTypeFileLineColumn; }

  void DumpValue(const ExecutionContext *exe_ctx, Stream &strm,
                 uint32_t dump_mask) override;

  llvm::json::Value ToJSON(const ExecutionContext *exe_ctx) const override;

  Status
  SetValueFromString(llvm::StringRef value,
                     VarSetOperationType op = eVarSetOperationAssign) override;

  void Clear() override {
    m_file_spec.Clear();
    m_line_number = LLDB_INVALID_LINE_NUMBER;
    m_column_number = LLDB_INVALID_COLUMN_NUMBER;
  }

  void SetFile(const FileSpec &file_spec) { m_file_spec = file_spec; }
  void SetLine(uint32_t line) { m_line_number = line; }
  void SetColumn(uint32_t column) { m_column_number = column; }

  void AutoComplete(CommandInterpreter &interpreter,
                    CompletionRequest &request) override;

  FileSpec &GetFileSpec() { return m_file_spec; }
  uint32_t GetLineNumber() { return m_line_number; }
  uint32_t GetColumnNumber() { return m_column_number; }

  void SetCompletionMask(uint32_t mask) { m_completion_mask = mask; }

protected:
  FileSpec m_file_spec;
  uint32_t m_line_number = LLDB_INVALID_LINE_NUMBER;
  uint32_t m_column_number = LLDB_INVALID_COLUMN_NUMBER;
  uint32_t m_completion_mask = lldb::eSourceFileCompletion;
};

} // namespace lldb_private

#endif // LLDB_INTERPRETER_OPTIONVALUEFILECOLONLINE_H
