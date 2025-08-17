//===--- HeaderAnalysis.h -----------------------------------------*-C++-*-===//
//
// Copyright (c) 2025, NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
// 
// Author: Tunjay Akbarli
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
// 
//     http://www.apache.org/licenses/LICENSE-2.0
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

#ifndef LANGUAGE_CORE_TOOLING_INCLUSIONS_HEADER_ANALYSIS_H
#define LANGUAGE_CORE_TOOLING_INCLUSIONS_HEADER_ANALYSIS_H

#include "language/Core/Basic/FileEntry.h"
#include "toolchain/ADT/StringRef.h"
#include <optional>

namespace language::Core {
class SourceManager;
class HeaderSearch;

namespace tooling {

/// Returns true if the given physical file is a self-contained header.
///
/// A header is considered self-contained if
//   - it has a proper header guard or has been #imported or contains #import(s)
//   - *and* it doesn't have a dont-include-me pattern.
///
/// This function can be expensive as it may scan the source code to find out
/// dont-include-me pattern heuristically.
bool isSelfContainedHeader(FileEntryRef FE, const SourceManager &SM,
                           const HeaderSearch &HeaderInfo);

/// This scans the given source code to see if it contains #import(s).
bool codeContainsImports(toolchain::StringRef Code);

/// If Text begins an Include-What-You-Use directive, returns it.
/// Given "// IWYU pragma: keep", returns "keep".
/// Input is a null-terminated char* as provided by SM.getCharacterData().
/// (This should not be StringRef as we do *not* want to scan for its length).
/// For multi-line comments, we return only the first line.
std::optional<toolchain::StringRef> parseIWYUPragma(const char *Text);

} // namespace tooling
} // namespace language::Core

#endif // LANGUAGE_CORE_TOOLING_INCLUSIONS_HEADER_ANALYSIS_H
