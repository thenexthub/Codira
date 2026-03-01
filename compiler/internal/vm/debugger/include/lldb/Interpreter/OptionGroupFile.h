//===-- OptionGroupFile.h ---------------------------------------*- C++ -*-===//
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

#ifndef LLDB_INTERPRETER_OPTIONGROUPFILE_H
#define LLDB_INTERPRETER_OPTIONGROUPFILE_H

#include "lldb/Interpreter/OptionValueFileSpec.h"
#include "lldb/Interpreter/OptionValueFileSpecList.h"
#include "lldb/Interpreter/Options.h"

namespace lldb_private {

// OptionGroupFile

class OptionGroupFile : public OptionGroup {
public:
  OptionGroupFile(uint32_t usage_mask, bool required, const char *long_option,
                  int short_option, uint32_t completion_type,
                  lldb::CommandArgumentType argument_type,
                  const char *usage_text);

  ~OptionGroupFile() override = default;

  llvm::ArrayRef<OptionDefinition> GetDefinitions() override {
    return llvm::ArrayRef<OptionDefinition>(&m_option_definition, 1);
  }

  Status SetOptionValue(uint32_t option_idx, llvm::StringRef option_value,
                        ExecutionContext *execution_context) override;

  void OptionParsingStarting(ExecutionContext *execution_context) override;

  OptionValueFileSpec &GetOptionValue() { return m_file; }

  const OptionValueFileSpec &GetOptionValue() const { return m_file; }

protected:
  OptionValueFileSpec m_file;
  OptionDefinition m_option_definition;
};

// OptionGroupFileList

class OptionGroupFileList : public OptionGroup {
public:
  OptionGroupFileList(uint32_t usage_mask, bool required,
                      const char *long_option, int short_option,
                      uint32_t completion_type,
                      lldb::CommandArgumentType argument_type,
                      const char *usage_text);

  ~OptionGroupFileList() override;

  llvm::ArrayRef<OptionDefinition> GetDefinitions() override {
    return llvm::ArrayRef<OptionDefinition>(&m_option_definition, 1);
  }

  Status SetOptionValue(uint32_t option_idx, llvm::StringRef option_value,
                        ExecutionContext *execution_context) override;

  void OptionParsingStarting(ExecutionContext *execution_context) override;

  OptionValueFileSpecList &GetOptionValue() { return m_file_list; }

  const OptionValueFileSpecList &GetOptionValue() const { return m_file_list; }

protected:
  OptionValueFileSpecList m_file_list;
  OptionDefinition m_option_definition;
};

} // namespace lldb_private

#endif // LLDB_INTERPRETER_OPTIONGROUPFILE_H
