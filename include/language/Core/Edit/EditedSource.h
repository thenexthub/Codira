//===- EditedSource.h - Collection of source edits --------------*- C++ -*-===//
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

#ifndef LANGUAGE_CORE_EDIT_EDITEDSOURCE_H
#define LANGUAGE_CORE_EDIT_EDITEDSOURCE_H

#include "language/Core/Basic/IdentifierTable.h"
#include "language/Core/Basic/LLVM.h"
#include "language/Core/Basic/SourceLocation.h"
#include "language/Core/Edit/FileOffset.h"
#include "toolchain/ADT/DenseMap.h"
#include "toolchain/ADT/SmallVector.h"
#include "toolchain/ADT/StringRef.h"
#include "toolchain/Support/Allocator.h"
#include <map>
#include <tuple>
#include <utility>

namespace language::Core {

class LangOptions;
class PPConditionalDirectiveRecord;
class SourceManager;

namespace edit {

class Commit;
class EditsReceiver;

class EditedSource {
  const SourceManager &SourceMgr;
  const LangOptions &LangOpts;
  const PPConditionalDirectiveRecord *PPRec;

  struct FileEdit {
    StringRef Text;
    unsigned RemoveLen = 0;

    FileEdit() = default;
  };

  using FileEditsTy = std::map<FileOffset, FileEdit>;

  FileEditsTy FileEdits;

  struct MacroArgUse {
    IdentifierInfo *Identifier;
    SourceLocation ImmediateExpansionLoc;

    // Location of argument use inside the top-level macro
    SourceLocation UseLoc;

    bool operator==(const MacroArgUse &Other) const {
      return std::tie(Identifier, ImmediateExpansionLoc, UseLoc) ==
             std::tie(Other.Identifier, Other.ImmediateExpansionLoc,
                      Other.UseLoc);
    }
  };

  toolchain::DenseMap<SourceLocation, SmallVector<MacroArgUse, 2>> ExpansionToArgMap;
  SmallVector<std::pair<SourceLocation, MacroArgUse>, 2>
    CurrCommitMacroArgExps;

  IdentifierTable IdentTable;
  toolchain::BumpPtrAllocator StrAlloc;

public:
  EditedSource(const SourceManager &SM, const LangOptions &LangOpts,
               const PPConditionalDirectiveRecord *PPRec = nullptr)
      : SourceMgr(SM), LangOpts(LangOpts), PPRec(PPRec), IdentTable(LangOpts) {}

  const SourceManager &getSourceManager() const { return SourceMgr; }
  const LangOptions &getLangOpts() const { return LangOpts; }

  const PPConditionalDirectiveRecord *getPPCondDirectiveRecord() const {
    return PPRec;
  }

  bool canInsertInOffset(SourceLocation OrigLoc, FileOffset Offs);

  bool commit(const Commit &commit);

  void applyRewrites(EditsReceiver &receiver, bool adjustRemovals = true);
  void clearRewrites();

  StringRef copyString(StringRef str) { return str.copy(StrAlloc); }
  StringRef copyString(const Twine &twine);

private:
  bool commitInsert(SourceLocation OrigLoc, FileOffset Offs, StringRef text,
                    bool beforePreviousInsertions);
  bool commitInsertFromRange(SourceLocation OrigLoc, FileOffset Offs,
                             FileOffset InsertFromRangeOffs, unsigned Len,
                             bool beforePreviousInsertions);
  void commitRemove(SourceLocation OrigLoc, FileOffset BeginOffs, unsigned Len);

  StringRef getSourceText(FileOffset BeginOffs, FileOffset EndOffs,
                          bool &Invalid);
  FileEditsTy::iterator getActionForOffset(FileOffset Offs);
  void deconstructMacroArgLoc(SourceLocation Loc,
                              SourceLocation &ExpansionLoc,
                              MacroArgUse &ArgUse);

  void startingCommit();
  void finishedCommit();
};

} // namespace edit

} // namespace language::Core

#endif // LANGUAGE_CORE_EDIT_EDITEDSOURCE_H
