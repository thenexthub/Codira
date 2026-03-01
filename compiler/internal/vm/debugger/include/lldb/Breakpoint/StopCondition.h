//===----------------------------------------------------------------------===//
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

#ifndef LLDB_BREAKPOINT_STOPCONDITION_H
#define LLDB_BREAKPOINT_STOPCONDITION_H

#include "lldb/lldb-private.h"
#include "llvm/ADT/StringRef.h"

namespace lldb_private {

class StopCondition {
public:
  StopCondition() = default;
  StopCondition(std::string text,
                lldb::LanguageType language = lldb::eLanguageTypeUnknown)
      : m_language(language) {
    SetText(std::move(text));
  }

  explicit operator bool() const { return !m_text.empty(); }

  llvm::StringRef GetText() const { return m_text; }

  void SetText(std::string text) {
    static std::hash<std::string> hasher;
    m_text = std::move(text);
    m_hash = hasher(text);
  }

  size_t GetHash() const { return m_hash; }

  lldb::LanguageType GetLanguage() const { return m_language; }

  void SetLanguage(lldb::LanguageType language) { m_language = language; }

private:
  /// The condition to test.
  std::string m_text;

  /// Its hash, so that locations know when the condition is updated.
  size_t m_hash = 0;

  /// The language for this condition.
  lldb::LanguageType m_language = lldb::eLanguageTypeUnknown;
};

} // namespace lldb_private

#endif // LLDB_BREAKPOINT_STOPCONDITION_H
