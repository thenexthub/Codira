//===--- RewriteBufferEditsReceiver.cpp -----------------------------------===//
//
// Copyright (c) NeXTHub Corporation. All rights reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// This code is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// version 2 for more details (a copy is included in the LICENSE file that
// accompanied this code).
//
// Author(-s): Tunjay Akbarli
//

//===----------------------------------------------------------------------===//

#include "language/Migrator/RewriteBufferEditsReceiver.h"

using namespace language;
using namespace language::migrator;

void RewriteBufferEditsReceiver::insert(clang::SourceLocation ClangLoc,
                                                StringRef NewText) {
  auto Offset = ClangSourceManager.getFileOffset(ClangLoc);
  RewriteBuf.InsertText(Offset, NewText);
}

void RewriteBufferEditsReceiver::replace(clang::CharSourceRange ClangRange,
                                         StringRef ReplacementText) {
  auto StartOffset = ClangSourceManager.getFileOffset(ClangRange.getBegin());
  auto EndOffset = ClangSourceManager.getFileOffset(ClangRange.getEnd());
  auto Length = EndOffset - StartOffset;
  RewriteBuf.ReplaceText(StartOffset, Length, ReplacementText);
}

void RewriteBufferEditsReceiver::printResult(toolchain::raw_ostream &OS) const {
  RewriteBuf.write(OS);
}
