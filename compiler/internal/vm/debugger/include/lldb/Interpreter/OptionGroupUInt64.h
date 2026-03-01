//===-- OptionGroupUInt64.h -------------------------------------*- C++ -*-===//
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

#ifndef LLDB_INTERPRETER_OPTIONGROUPUINT64_H
#define LLDB_INTERPRETER_OPTIONGROUPUINT64_H

#include "lldb/Interpreter/OptionValueUInt64.h"
#include "lldb/Interpreter/Options.h"

namespace lldb_private {

// OptionGroupUInt64

class OptionGroupUInt64 : public OptionGroup {
public:
  OptionGroupUInt64(uint32_t usage_mask, bool required, const char *long_option,
                    int short_option, uint32_t completion_type,
                    lldb::CommandArgumentType argument_type,
                    const char *usage_text, uint64_t default_value);

  ~OptionGroupUInt64() override = default;

  llvm::ArrayRef<OptionDefinition> GetDefinitions() override {
    return llvm::ArrayRef<OptionDefinition>(&m_option_definition, 1);
  }

  Status SetOptionValue(uint32_t option_idx, llvm::StringRef option_value,
                        ExecutionContext *execution_context) override;

  void OptionParsingStarting(ExecutionContext *execution_context) override;

  OptionValueUInt64 &GetOptionValue() { return m_value; }

  const OptionValueUInt64 &GetOptionValue() const { return m_value; }

protected:
  OptionValueUInt64 m_value;
  OptionDefinition m_option_definition;
};

} // namespace lldb_private

#endif // LLDB_INTERPRETER_OPTIONGROUPUINT64_H
