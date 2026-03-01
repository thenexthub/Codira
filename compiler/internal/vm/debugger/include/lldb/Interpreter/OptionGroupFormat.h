//===-- OptionGroupFormat.h -------------------------------------*- C++ -*-===//
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

#ifndef LLDB_INTERPRETER_OPTIONGROUPFORMAT_H
#define LLDB_INTERPRETER_OPTIONGROUPFORMAT_H

#include "lldb/Interpreter/OptionValueFormat.h"
#include "lldb/Interpreter/OptionValueSInt64.h"
#include "lldb/Interpreter/OptionValueUInt64.h"
#include "lldb/Interpreter/Options.h"

namespace lldb_private {

typedef std::vector<std::tuple<lldb::CommandArgumentType, const char *>>
    OptionGroupFormatUsageTextVector;

// OptionGroupFormat

class OptionGroupFormat : public OptionGroup {
public:
  static const uint32_t OPTION_GROUP_FORMAT = LLDB_OPT_SET_1;
  static const uint32_t OPTION_GROUP_GDB_FMT = LLDB_OPT_SET_2;
  static const uint32_t OPTION_GROUP_SIZE = LLDB_OPT_SET_3;
  static const uint32_t OPTION_GROUP_COUNT = LLDB_OPT_SET_4;

  OptionGroupFormat(
      lldb::Format default_format,
      uint64_t default_byte_size =
          UINT64_MAX, // Pass UINT64_MAX to disable the "--size" option
      uint64_t default_count =
          UINT64_MAX, // Pass UINT64_MAX to disable the "--count" option
      OptionGroupFormatUsageTextVector usage_text_vector = {}
      // Use to override default option usage text with the command specific one
  );

  ~OptionGroupFormat() override = default;

  llvm::ArrayRef<OptionDefinition> GetDefinitions() override;

  Status SetOptionValue(uint32_t option_idx, llvm::StringRef option_value,
                        ExecutionContext *execution_context) override;

  void OptionParsingStarting(ExecutionContext *execution_context) override;

  lldb::Format GetFormat() const { return m_format.GetCurrentValue(); }

  OptionValueFormat &GetFormatValue() { return m_format; }

  const OptionValueFormat &GetFormatValue() const { return m_format; }

  OptionValueUInt64 &GetByteSizeValue() { return m_byte_size; }

  const OptionValueUInt64 &GetByteSizeValue() const { return m_byte_size; }

  OptionValueUInt64 &GetCountValue() { return m_count; }

  const OptionValueUInt64 &GetCountValue() const { return m_count; }

  bool HasGDBFormat() const { return m_has_gdb_format; }

  bool AnyOptionWasSet() const {
    return m_format.OptionWasSet() || m_byte_size.OptionWasSet() ||
           m_count.OptionWasSet();
  }

protected:
  bool ParserGDBFormatLetter(ExecutionContext *execution_context,
                             char format_letter, lldb::Format &format,
                             uint32_t &byte_size);

  OptionValueFormat m_format;
  OptionValueUInt64 m_byte_size;
  OptionValueUInt64 m_count;
  char m_prev_gdb_format;
  char m_prev_gdb_size;
  bool m_has_gdb_format;
  OptionDefinition m_option_definitions[4];
};

} // namespace lldb_private

#endif // LLDB_INTERPRETER_OPTIONGROUPFORMAT_H
