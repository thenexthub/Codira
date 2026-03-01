//===-- CommandHistory.h ----------------------------------------*- C++ -*-===//
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

#ifndef LLDB_INTERPRETER_COMMANDHISTORY_H
#define LLDB_INTERPRETER_COMMANDHISTORY_H

#include <mutex>
#include <optional>
#include <string>
#include <vector>

#include "lldb/Utility/Stream.h"
#include "lldb/lldb-private.h"

namespace lldb_private {

class CommandHistory {
public:
  CommandHistory() = default;

  ~CommandHistory() = default;

  size_t GetSize() const;

  bool IsEmpty() const;

  std::optional<llvm::StringRef> FindString(llvm::StringRef input_str) const;

  llvm::StringRef GetStringAtIndex(size_t idx) const;

  llvm::StringRef operator[](size_t idx) const;

  llvm::StringRef GetRecentmostString() const;

  void AppendString(llvm::StringRef str, bool reject_if_dupe = true);

  void Clear();

  void Dump(Stream &stream, size_t start_idx = 0,
            size_t stop_idx = SIZE_MAX) const;

  static const char g_repeat_char = '!';

private:
  CommandHistory(const CommandHistory &) = delete;
  const CommandHistory &operator=(const CommandHistory &) = delete;

  typedef std::vector<std::string> History;
  mutable std::recursive_mutex m_mutex;
  History m_history;
};

} // namespace lldb_private

#endif // LLDB_INTERPRETER_COMMANDHISTORY_H
