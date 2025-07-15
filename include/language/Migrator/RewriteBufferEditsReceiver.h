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

#include "clang/Basic/SourceManager.h"
#include "clang/Basic/SourceLocation.h"
#include "clang/Edit/EditsReceiver.h"
#include "clang/Rewrite/Core/RewriteBuffer.h"
#include "toolchain/ADT/StringRef.h"
#include "toolchain/Support/raw_ostream.h"

using toolchain::StringRef;

namespace language {
namespace migrator {

/// An EditsReceiver that collects edits from an EditedSource and directly
/// applies it to a clang::RewriteBuffer.
class RewriteBufferEditsReceiver final : public clang::edit::EditsReceiver {
  const clang::SourceManager &ClangSourceManager;
  const clang::FileID InputFileID;
  const StringRef InputText;
  clang::RewriteBuffer RewriteBuf;
public:
  RewriteBufferEditsReceiver(const clang::SourceManager &ClangSourceManager,
                             const clang::FileID InputFileID,
                             const StringRef InputText)
    : ClangSourceManager(ClangSourceManager),
      InputFileID(InputFileID),
      InputText(InputText) {
    RewriteBuf.Initialize(InputText);
  }

  virtual void insert(clang::SourceLocation Loc, StringRef Text) override;
  virtual void replace(clang::CharSourceRange Range, StringRef Text) override;

  /// Print the result of all of the edits to the given output stream.
  void printResult(toolchain::raw_ostream &OS) const;
};

} // end namespace migrator
} // end namespace language

#endif // LANGUAGE_MIGRATOR_REWRITEBUFFEREDITSRECEIVER_H
