//===-- OptionValueFileSpec.h -----------------------------------*- C++ -*-===//
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

#ifndef LLDB_INTERPRETER_OPTIONVALUEFILESPEC_H
#define LLDB_INTERPRETER_OPTIONVALUEFILESPEC_H

#include "lldb/Interpreter/CommandCompletions.h"
#include "lldb/Interpreter/OptionValue.h"

#include "lldb/Utility/FileSpec.h"
#include "llvm/Support/Chrono.h"

namespace lldb_private {

class OptionValueFileSpec : public Cloneable<OptionValueFileSpec, OptionValue> {
public:
  OptionValueFileSpec(bool resolve = true);

  OptionValueFileSpec(const FileSpec &value, bool resolve = true);

  OptionValueFileSpec(const FileSpec &current_value,
                      const FileSpec &default_value, bool resolve = true);

  ~OptionValueFileSpec() override = default;

  // Virtual subclass pure virtual overrides

  OptionValue::Type GetType() const override { return eTypeFileSpec; }

  void DumpValue(const ExecutionContext *exe_ctx, Stream &strm,
                 uint32_t dump_mask) override;

  llvm::json::Value ToJSON(const ExecutionContext *exe_ctx) const override {
    return m_current_value.GetPath();
  }

  Status
  SetValueFromString(llvm::StringRef value,
                     VarSetOperationType op = eVarSetOperationAssign) override;

  void Clear() override {
    m_current_value = m_default_value;
    m_value_was_set = false;
    m_data_sp.reset();
    m_data_mod_time = llvm::sys::TimePoint<>();
  }

  void AutoComplete(CommandInterpreter &interpreter,
                    CompletionRequest &request) override;

  // Subclass specific functions

  FileSpec &GetCurrentValue() { return m_current_value; }

  const FileSpec &GetCurrentValue() const { return m_current_value; }

  const FileSpec &GetDefaultValue() const { return m_default_value; }

  void SetCurrentValue(const FileSpec &value, bool set_value_was_set) {
    m_current_value = value;
    if (set_value_was_set)
      m_value_was_set = true;
    m_data_sp.reset();
  }

  void SetDefaultValue(const FileSpec &value) { m_default_value = value; }

  const lldb::DataBufferSP &GetFileContents();

  void SetCompletionMask(uint32_t mask) { m_completion_mask = mask; }

protected:
  FileSpec m_current_value;
  FileSpec m_default_value;
  lldb::DataBufferSP m_data_sp;
  llvm::sys::TimePoint<> m_data_mod_time;
  uint32_t m_completion_mask = lldb::eDiskFileCompletion;
  bool m_resolve;
};

} // namespace lldb_private

#endif // LLDB_INTERPRETER_OPTIONVALUEFILESPEC_H
