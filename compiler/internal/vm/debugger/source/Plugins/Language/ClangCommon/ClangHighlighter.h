//===-- ClangHighlighter.h --------------------------------------*- C++ -*-===//
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

#ifndef LLDB_SOURCE_PLUGINS_LANGUAGE_CLANGCOMMON_CLANGHIGHLIGHTER_H
#define LLDB_SOURCE_PLUGINS_LANGUAGE_CLANGCOMMON_CLANGHIGHLIGHTER_H

#include "lldb/Utility/Stream.h"
#include "llvm/ADT/StringSet.h"

#include "lldb/Core/Highlighter.h"
#include <optional>

namespace lldb_private {

class ClangHighlighter : public Highlighter {
  llvm::StringSet<> keywords;

public:
  ClangHighlighter();
  llvm::StringRef GetName() const override { return "clang"; }

  void Highlight(const HighlightStyle &options, llvm::StringRef line,
                 std::optional<size_t> cursor_pos,
                 llvm::StringRef previous_lines, Stream &s) const override;

  /// Returns true if the given string represents a keywords in any Clang
  /// supported language.
  bool isKeyword(llvm::StringRef token) const;
};

} // namespace lldb_private

#endif // LLDB_SOURCE_PLUGINS_LANGUAGE_CLANGCOMMON_CLANGHIGHLIGHTER_H
