//===--- RewriteBufferEditsReceiver.h ---------------------------*- C++ -*-===//
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

#ifndef LANGUAGE_MIGRATOR_REWRITEBUFFEREDITSRECEIVER_H
#define LANGUAGE_MIGRATOR_REWRITEBUFFEREDITSRECEIVER_H

#include "language/Core/Basic/SourceManager.h"
#include "language/Core/Basic/SourceLocation.h"
#include "language/Core/Edit/EditsReceiver.h"
#include "language/Core/Rewrite/Core/RewriteBuffer.h"
#include "toolchain/ADT/StringRef.h"
#include "toolchain/Support/raw_ostream.h"

using toolchain::StringRef;

namespace language {
namespace migrator {

/// An EditsReceiver that collects edits from an EditedSource and directly
/// applies it to a language::Core::RewriteBuffer.
class RewriteBufferEditsReceiver final : public language::Core::edit::EditsReceiver {
  const language::Core::SourceManager &ClangSourceManager;
  const language::Core::FileID InputFileID;
  const StringRef InputText;
  language::Core::RewriteBuffer RewriteBuf;
public:
  RewriteBufferEditsReceiver(const language::Core::SourceManager &ClangSourceManager,
                             const language::Core::FileID InputFileID,
                             const StringRef InputText)
    : ClangSourceManager(ClangSourceManager),
      InputFileID(InputFileID),
      InputText(InputText) {
    RewriteBuf.Initialize(InputText);
  }

  virtual void insert(language::Core::SourceLocation Loc, StringRef Text) override;
  virtual void replace(language::Core::CharSourceRange Range, StringRef Text) override;

  /// Print the result of all of the edits to the given output stream.
  void printResult(toolchain::raw_ostream &OS) const;
};

} // end namespace migrator
} // end namespace language

#endif // LANGUAGE_MIGRATOR_REWRITEBUFFEREDITSRECEIVER_H
