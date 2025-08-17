//===--- Diagnostic.cpp - Framework for clang diagnostics tools ----------===//
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
//
//  Implements classes to support/store diagnostics refactoring.
//
//===----------------------------------------------------------------------===//

#include "language/Core/Tooling/Core/Diagnostic.h"
#include "language/Core/Basic/SourceLocation.h"
#include "language/Core/Basic/SourceManager.h"
#include "toolchain/ADT/STLExtras.h"

namespace language::Core {
namespace tooling {

DiagnosticMessage::DiagnosticMessage(toolchain::StringRef Message)
    : Message(Message), FileOffset(0) {}

DiagnosticMessage::DiagnosticMessage(toolchain::StringRef Message,
                                     const SourceManager &Sources,
                                     SourceLocation Loc)
    : Message(Message), FileOffset(0) {
  assert(Loc.isValid() && Loc.isFileID());
  FilePath = std::string(Sources.getFilename(Loc));

  // Don't store offset in the scratch space. It doesn't tell anything to the
  // user. Moreover, it depends on the history of macro expansions and thus
  // prevents deduplication of warnings in headers.
  if (!FilePath.empty())
    FileOffset = Sources.getFileOffset(Loc);
}

FileByteRange::FileByteRange(
    const SourceManager &Sources, CharSourceRange Range)
    : FileOffset(0), Length(0) {
  FilePath = std::string(Sources.getFilename(Range.getBegin()));
  if (!FilePath.empty()) {
    FileOffset = Sources.getFileOffset(Range.getBegin());
    Length = Sources.getFileOffset(Range.getEnd()) - FileOffset;
  }
}

Diagnostic::Diagnostic(toolchain::StringRef DiagnosticName,
                       Diagnostic::Level DiagLevel, StringRef BuildDirectory)
    : DiagnosticName(DiagnosticName), DiagLevel(DiagLevel),
      BuildDirectory(BuildDirectory) {}

Diagnostic::Diagnostic(toolchain::StringRef DiagnosticName,
                       const DiagnosticMessage &Message,
                       const SmallVector<DiagnosticMessage, 1> &Notes,
                       Level DiagLevel, toolchain::StringRef BuildDirectory)
    : DiagnosticName(DiagnosticName), Message(Message), Notes(Notes),
      DiagLevel(DiagLevel), BuildDirectory(BuildDirectory) {}

const toolchain::StringMap<Replacements> *selectFirstFix(const Diagnostic& D) {
   if (!D.Message.Fix.empty())
    return &D.Message.Fix;
  auto Iter = toolchain::find_if(D.Notes, [](const tooling::DiagnosticMessage &D) {
    return !D.Fix.empty();
  });
  if (Iter != D.Notes.end())
    return &Iter->Fix;
  return nullptr;
}

} // end namespace tooling
} // end namespace language::Core
