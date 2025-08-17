//===- SourceManagerInternals.h - SourceManager Internals -------*- C++ -*-===//
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
/// \file
/// Defines implementation details of the language::Core::SourceManager class.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_BASIC_SOURCEMANAGERINTERNALS_H
#define LANGUAGE_CORE_BASIC_SOURCEMANAGERINTERNALS_H

#include "language/Core/Basic/SourceLocation.h"
#include "language/Core/Basic/SourceManager.h"
#include "toolchain/ADT/StringMap.h"
#include "toolchain/ADT/StringRef.h"
#include "toolchain/Support/Allocator.h"
#include <cassert>
#include <map>
#include <vector>

namespace language::Core {

//===----------------------------------------------------------------------===//
// Line Table Implementation
//===----------------------------------------------------------------------===//

struct LineEntry {
  /// The offset in this file that the line entry occurs at.
  unsigned FileOffset;

  /// The presumed line number of this line entry: \#line 4.
  unsigned LineNo;

  /// The ID of the filename identified by this line entry:
  /// \#line 4 "foo.c".  This is -1 if not specified.
  int FilenameID;

  /// Set the 0 if no flags, 1 if a system header,
  SrcMgr::CharacteristicKind FileKind;

  /// The offset of the virtual include stack location,
  /// which is manipulated by GNU linemarker directives.
  ///
  /// If this is 0 then there is no virtual \#includer.
  unsigned IncludeOffset;

  static LineEntry get(unsigned Offs, unsigned Line, int Filename,
                       SrcMgr::CharacteristicKind FileKind,
                       unsigned IncludeOffset) {
    LineEntry E;
    E.FileOffset = Offs;
    E.LineNo = Line;
    E.FilenameID = Filename;
    E.FileKind = FileKind;
    E.IncludeOffset = IncludeOffset;
    return E;
  }
};

// needed for FindNearestLineEntry (upper_bound of LineEntry)
inline bool operator<(const LineEntry &lhs, const LineEntry &rhs) {
  // FIXME: should check the other field?
  return lhs.FileOffset < rhs.FileOffset;
}

inline bool operator<(const LineEntry &E, unsigned Offset) {
  return E.FileOffset < Offset;
}

inline bool operator<(unsigned Offset, const LineEntry &E) {
  return Offset < E.FileOffset;
}

/// Used to hold and unique data used to represent \#line information.
class LineTableInfo {
  /// Map used to assign unique IDs to filenames in \#line directives.
  ///
  /// This allows us to unique the filenames that
  /// frequently reoccur and reference them with indices.  FilenameIDs holds
  /// the mapping from string -> ID, and FilenamesByID holds the mapping of ID
  /// to string.
  toolchain::StringMap<unsigned, toolchain::BumpPtrAllocator> FilenameIDs;
  std::vector<toolchain::StringMapEntry<unsigned>*> FilenamesByID;

  /// Map from FileIDs to a list of line entries (sorted by the offset
  /// at which they occur in the file).
  std::map<FileID, std::vector<LineEntry>> LineEntries;

public:
  void clear() {
    FilenameIDs.clear();
    FilenamesByID.clear();
    LineEntries.clear();
  }

  unsigned getLineTableFilenameID(StringRef Str);

  StringRef getFilename(unsigned ID) const {
    assert(ID < FilenamesByID.size() && "Invalid FilenameID");
    return FilenamesByID[ID]->getKey();
  }

  unsigned getNumFilenames() const { return FilenamesByID.size(); }

  void AddLineNote(FileID FID, unsigned Offset,
                   unsigned LineNo, int FilenameID,
                   unsigned EntryExit, SrcMgr::CharacteristicKind FileKind);


  /// Find the line entry nearest to FID that is before it.
  ///
  /// If there is no line entry before \p Offset in \p FID, returns null.
  const LineEntry *FindNearestLineEntry(FileID FID, unsigned Offset);

  // Low-level access
  using iterator = std::map<FileID, std::vector<LineEntry>>::iterator;

  iterator begin() { return LineEntries.begin(); }
  iterator end() { return LineEntries.end(); }

  /// Add a new line entry that has already been encoded into
  /// the internal representation of the line table.
  void AddEntry(FileID FID, const std::vector<LineEntry> &Entries);
};

} // namespace language::Core

#endif // LANGUAGE_CORE_BASIC_SOURCEMANAGERINTERNALS_H
