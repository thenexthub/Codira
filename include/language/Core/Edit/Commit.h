//===- Commit.h - A unit of edits -------------------------------*- C++ -*-===//
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

#ifndef LANGUAGE_CORE_EDIT_COMMIT_H
#define LANGUAGE_CORE_EDIT_COMMIT_H

#include "language/Core/Basic/LLVM.h"
#include "language/Core/Basic/SourceLocation.h"
#include "language/Core/Edit/FileOffset.h"
#include "toolchain/ADT/SmallVector.h"
#include "toolchain/ADT/StringRef.h"
#include "toolchain/Support/Allocator.h"

namespace language::Core {

class LangOptions;
class PPConditionalDirectiveRecord;
class SourceManager;

namespace edit {

class EditedSource;

class Commit {
public:
  enum EditKind {
    Act_Insert,
    Act_InsertFromRange,
    Act_Remove
  };

  struct Edit {
    EditKind Kind;
    StringRef Text;
    SourceLocation OrigLoc;
    FileOffset Offset;
    FileOffset InsertFromRangeOffs;
    unsigned Length;
    bool BeforePrev;

    SourceLocation getFileLocation(SourceManager &SM) const;
    CharSourceRange getFileRange(SourceManager &SM) const;
    CharSourceRange getInsertFromRange(SourceManager &SM) const;
  };

private:
  const SourceManager &SourceMgr;
  const LangOptions &LangOpts;
  const PPConditionalDirectiveRecord *PPRec;
  EditedSource *Editor = nullptr;

  bool IsCommitable = true;
  SmallVector<Edit, 8> CachedEdits;

  toolchain::BumpPtrAllocator StrAlloc;

public:
  explicit Commit(EditedSource &Editor);
  Commit(const SourceManager &SM, const LangOptions &LangOpts,
         const PPConditionalDirectiveRecord *PPRec = nullptr)
      : SourceMgr(SM), LangOpts(LangOpts), PPRec(PPRec) {}

  bool isCommitable() const { return IsCommitable; }

  bool insert(SourceLocation loc, StringRef text, bool afterToken = false,
              bool beforePreviousInsertions = false);

  bool insertAfterToken(SourceLocation loc, StringRef text,
                        bool beforePreviousInsertions = false) {
    return insert(loc, text, /*afterToken=*/true, beforePreviousInsertions);
  }

  bool insertBefore(SourceLocation loc, StringRef text) {
    return insert(loc, text, /*afterToken=*/false,
                  /*beforePreviousInsertions=*/true);
  }

  bool insertFromRange(SourceLocation loc, CharSourceRange range,
                       bool afterToken = false,
                       bool beforePreviousInsertions = false);
  bool insertWrap(StringRef before, CharSourceRange range, StringRef after);

  bool remove(CharSourceRange range);

  bool replace(CharSourceRange range, StringRef text);
  bool replaceWithInner(CharSourceRange range, CharSourceRange innerRange);
  bool replaceText(SourceLocation loc, StringRef text,
                   StringRef replacementText);

  bool insertFromRange(SourceLocation loc, SourceRange TokenRange,
                       bool afterToken = false,
                       bool beforePreviousInsertions = false) {
    return insertFromRange(loc, CharSourceRange::getTokenRange(TokenRange),
                           afterToken, beforePreviousInsertions);
  }

  bool insertWrap(StringRef before, SourceRange TokenRange, StringRef after) {
    return insertWrap(before, CharSourceRange::getTokenRange(TokenRange), after);
  }

  bool remove(SourceRange TokenRange) {
    return remove(CharSourceRange::getTokenRange(TokenRange));
  }

  bool replace(SourceRange TokenRange, StringRef text) {
    return replace(CharSourceRange::getTokenRange(TokenRange), text);
  }

  bool replaceWithInner(SourceRange TokenRange, SourceRange TokenInnerRange) {
    return replaceWithInner(CharSourceRange::getTokenRange(TokenRange),
                            CharSourceRange::getTokenRange(TokenInnerRange));
  }

  using edit_iterator = SmallVectorImpl<Edit>::const_iterator;

  edit_iterator edit_begin() const { return CachedEdits.begin(); }
  edit_iterator edit_end() const { return CachedEdits.end(); }

private:
  void addInsert(SourceLocation OrigLoc,
                FileOffset Offs, StringRef text, bool beforePreviousInsertions);
  void addInsertFromRange(SourceLocation OrigLoc, FileOffset Offs,
                          FileOffset RangeOffs, unsigned RangeLen,
                          bool beforePreviousInsertions);
  void addRemove(SourceLocation OrigLoc, FileOffset Offs, unsigned Len);

  bool canInsert(SourceLocation loc, FileOffset &Offset);
  bool canInsertAfterToken(SourceLocation loc, FileOffset &Offset,
                           SourceLocation &AfterLoc);
  bool canInsertInOffset(SourceLocation OrigLoc, FileOffset Offs);
  bool canRemoveRange(CharSourceRange range, FileOffset &Offs, unsigned &Len);
  bool canReplaceText(SourceLocation loc, StringRef text,
                      FileOffset &Offs, unsigned &Len);

  void commitInsert(FileOffset offset, StringRef text,
                    bool beforePreviousInsertions);
  void commitRemove(FileOffset offset, unsigned length);

  bool isAtStartOfMacroExpansion(SourceLocation loc,
                                 SourceLocation *MacroBegin = nullptr) const;
  bool isAtEndOfMacroExpansion(SourceLocation loc,
                               SourceLocation *MacroEnd = nullptr) const;
};

} // namespace edit

} // namespace language::Core

#endif // LANGUAGE_CORE_EDIT_COMMIT_H
