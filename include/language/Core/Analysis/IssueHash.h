//===---------- IssueHash.h - Generate identification hashes ----*- C++ -*-===//
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
#ifndef LANGUAGE_CORE_ANALYSIS_ISSUEHASH_H
#define LANGUAGE_CORE_ANALYSIS_ISSUEHASH_H

#include "toolchain/ADT/SmallString.h"

namespace language::Core {
class Decl;
class FullSourceLoc;
class LangOptions;

/// Returns an opaque identifier for a diagnostic.
///
/// This opaque identifier is intended to be stable even when the source code
/// is changed. It allows to track diagnostics in the long term, for example,
/// find which diagnostics are "new", maintain a database of suppressed
/// diagnostics etc.
///
/// We may introduce more variants of issue hashes in the future
/// but older variants will still be available for compatibility.
///
/// This hash is based on the following information:
///   - Name of the checker that emitted the diagnostic.
///   - Warning message.
///   - Name of the enclosing declaration.
///   - Contents of the line of code with the issue, excluding whitespace.
///   - Column number (but not the line number! - which makes it stable).
toolchain::SmallString<32> getIssueHash(const FullSourceLoc &IssueLoc,
                                   toolchain::StringRef CheckerName,
                                   toolchain::StringRef WarningMessage,
                                   const Decl *IssueDecl,
                                   const LangOptions &LangOpts);

/// Get the unhashed string representation of the V1 issue hash.
/// When hashed, it becomes the actual issue hash. Useful for testing.
/// See GetIssueHashV1() for more information.
std::string getIssueString(const FullSourceLoc &IssueLoc,
                           toolchain::StringRef CheckerName,
                           toolchain::StringRef WarningMessage,
                           const Decl *IssueDecl, const LangOptions &LangOpts);
} // namespace language::Core

#endif
