//===-- OptionArgParser.h ---------------------------------------*- C++ -*-===//
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

#ifndef LLDB_INTERPRETER_OPTIONARGPARSER_H
#define LLDB_INTERPRETER_OPTIONARGPARSER_H

#include "lldb/lldb-private-types.h"

#include <optional>

namespace lldb_private {

struct OptionArgParser {
  /// Try to parse an address. If it succeeds return the address with the
  /// non-address bits removed.
  static lldb::addr_t ToAddress(const ExecutionContext *exe_ctx,
                                llvm::StringRef s, lldb::addr_t fail_value,
                                Status *error_ptr);

  /// As for ToAddress but do not remove non-address bits from the result.
  static lldb::addr_t ToRawAddress(const ExecutionContext *exe_ctx,
                                   llvm::StringRef s, lldb::addr_t fail_value,
                                   Status *error_ptr);

  static bool ToBoolean(llvm::StringRef s, bool fail_value, bool *success_ptr);

  static llvm::Expected<bool> ToBoolean(llvm::StringRef option_name,
                                        llvm::StringRef option_arg);

  static char ToChar(llvm::StringRef s, char fail_value, bool *success_ptr);

  static int64_t ToOptionEnum(llvm::StringRef s,
                              const OptionEnumValues &enum_values,
                              int32_t fail_value, Status &error);

  static lldb::ScriptLanguage ToScriptLanguage(llvm::StringRef s,
                                               lldb::ScriptLanguage fail_value,
                                               bool *success_ptr);

  // TODO: Use StringRef
  static Status ToFormat(const char *s, lldb::Format &format,
                         size_t *byte_size_ptr); // If non-NULL, then a
                                                 // byte size can precede
                                                 // the format character

private:
  static std::optional<lldb::addr_t>
  DoToAddress(const ExecutionContext *exe_ctx, llvm::StringRef s,
              Status *error);
};

} // namespace lldb_private

#endif // LLDB_INTERPRETER_OPTIONARGPARSER_H
