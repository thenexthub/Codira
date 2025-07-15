//===--- EditorAdapter.h ----------------------------------------*- C++ -*-===//
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
//
// This class wraps a clang::edit::Commit, taking Codira source locations and
// ranges, transforming them to Clang source locations and ranges, and pushes
// them into the textual editing infrastructure. This is a temporary measure
// while lib/Syntax bringup is happening.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_MIGRATOR_EDITORADAPTER_H
#define LANGUAGE_MIGRATOR_EDITORADAPTER_H

#include "language/Migrator/Replacement.h"
#include "clang/Basic/LangOptions.h"
#include "clang/Basic/SourceLocation.h"
#include "clang/Edit/Commit.h"
#include "toolchain/ADT/DenseMap.h"
#include "toolchain/ADT/SmallSet.h"

namespace language {

class SourceLoc;
class SourceRange;
class CharSourceRange;
class SourceManager;

namespace migrator {

class EditorAdapter {
  language::SourceManager &CodiraSrcMgr;
  clang::SourceManager &ClangSrcMgr;

  /// This holds a mapping of identical buffers, one that exist in the
  /// CodiraSrcMgr and one that exists in the ClangSrcMgr.
  ///
  /// This is marked mutable because it is lazily populated internally
  /// in the `getClangFileIDForCodiraBufferID` method below.
  mutable toolchain::SmallDenseMap<unsigned, clang::FileID> CodiraToClangBufferMap;

  /// Tracks a history of edits outside of the clang::edit::Commit collector
  /// below. That doesn't handle duplicate or redundant changes.
  mutable toolchain::SmallSet<Replacement, 32> Replacements;

  bool CacheEnabled;

  /// A running transactional collection of basic edit operations.
  /// Clang uses this transaction concept to cancel a batch of edits due to
  /// incompatibilities, such as those due to macro expansions, but we don't
  /// have macros in Codira. However, as a temporary adapter API, we use this
  /// to keep things simple.
  clang::edit::Commit Edits;

  /// Translate a Codira SourceLoc using the CodiraSrcMgr to a
  /// clang::SourceLocation using the ClangSrcMgr.
  clang::SourceLocation translateSourceLoc(SourceLoc CodiraLoc) const;

  /// Translate a Codira SourceRange using the CodiraSrcMgr to a
  /// clang::SourceRange using the ClangSrcMgr.
  clang::SourceRange
  translateSourceRange(SourceRange CodiraSourceRange) const;

  /// Translate a Codira CharSourceRange using the CodiraSrcMgr to a
  /// clang::CharSourceRange using the ClangSrcMgr.
  clang::CharSourceRange
  translateCharSourceRange(CharSourceRange CodiraSourceSourceRange) const;

  /// Returns the buffer ID and absolute offset for a Codira SourceLoc.
  std::pair<unsigned, unsigned> getLocInfo(language::SourceLoc Loc) const;

  /// Returns true if the replacement has already been booked. Otherwise,
  /// returns false and adds it to the replacement set.
  bool cacheReplacement(CharSourceRange Range, StringRef Text) const;

public:
  EditorAdapter(language::SourceManager &CodiraSrcMgr,
                clang::SourceManager &ClangSrcMgr)
    : CodiraSrcMgr(CodiraSrcMgr), ClangSrcMgr(ClangSrcMgr), CacheEnabled(true),
      Edits(clang::edit::Commit(ClangSrcMgr, clang::LangOptions())) {}

  /// Lookup the BufferID in the CodiraToClangBufferMap. If it doesn't exist,
  /// copy the corresponding buffer into the ClangSrcMgr.
  clang::FileID getClangFileIDForCodiraBufferID(unsigned BufferID) const;

  bool insert(SourceLoc Loc, StringRef Text, bool AfterToken = false,
              bool BeforePreviousInsertions = false);

  bool insertAfterToken(SourceLoc Loc, StringRef Text,
                        bool BeforePreviousInsertions = false) {
    return insert(Loc, Text, /*AfterToken=*/true, BeforePreviousInsertions);
  }

  bool insertBefore(SourceLoc Loc, StringRef Text) {
    return insert(Loc, Text, /*AfterToken=*/false,
                  /*BeforePreviousInsertions=*/true);
  }

  bool insertFromRange(SourceLoc Loc, CharSourceRange Range,
                       bool AfterToken = false,
                       bool BeforePreviousInsertions = false);
  bool insertWrap(StringRef before, CharSourceRange Range, StringRef after);

  bool remove(CharSourceRange Range);

  bool replace(CharSourceRange Range, StringRef Text);
  bool replaceWithInner(CharSourceRange Range, CharSourceRange innerRange);
  bool replaceText(SourceLoc Loc, StringRef Text,
                   StringRef replacementText);

  bool insertFromRange(SourceLoc Loc, SourceRange TokenRange,
                       bool AfterToken = false,
                       bool BeforePreviousInsertions = false);
  bool insertWrap(StringRef before, SourceRange TokenRange, StringRef after);
  bool remove(SourceLoc TokenLoc);
  bool remove(SourceRange TokenRange);
  bool replace(SourceRange TokenRange, StringRef Text);
  bool replaceToken(SourceLoc TokenLoc, StringRef Text);
  bool replaceWithInner(SourceRange TokenRange, SourceRange TokenInnerRange);

  /// Return the batched edits encountered so far.
  const clang::edit::Commit &getEdits() const {
    return Edits;
  }
  void enableCache() { CacheEnabled = true; }
  void disableCache() { CacheEnabled = false; }
};

} // end namespace migrator
} // end namespace language

#endif // LANGUAGE_MIGRATOR_EDITORADAPTER_H
