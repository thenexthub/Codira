//===--- BasicSourceInfo.h - Simple source information ----------*- C++ -*-===//
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

#ifndef LANGUAGE_BASIC_BASIC_SOURCE_INFO_H
#define LANGUAGE_BASIC_BASIC_SOURCE_INFO_H

#include "language/Basic/Fingerprint.h"
#include "language/Basic/Toolchain.h"
#include "language/Basic/SourceLoc.h"
#include "toolchain/ADT/PointerIntPair.h"
#include "toolchain/Support/Chrono.h"

namespace language {

class SourceFile;

struct ExternalSourceLocs {
  struct LocationDirective {
    uint32_t Offset = 0;
    int32_t LineOffset = 0;
    uint32_t Length = 0;
    StringRef Name;

    bool isValid() const { return Length > 0; }
  };

  struct RawLoc {
    uint32_t Offset = 0;
    uint32_t Line = 0;
    uint32_t Column = 0;
    LocationDirective Directive;
  };

  struct RawLocs {
    StringRef SourceFilePath;
    SmallVector<std::pair<RawLoc, uint32_t>, 4> DocRanges;
    RawLoc Loc;
    RawLoc StartLoc;
    RawLoc EndLoc;
  };

  unsigned BufferID = 0;
  SourceLoc Loc;
  ArrayRef<CharSourceRange> DocRanges;
};

class BasicSourceFileInfo {
  /// If this is non-null, fields other than 'FilePath' hasn't been populated.
  /// The 'getInt()' part indicates this instance is constructed with a
  /// SourceFile.
  toolchain::PointerIntPair<const SourceFile *, 1, bool> SFAndIsFromSF;

  StringRef FilePath;
  Fingerprint InterfaceHashIncludingTypeMembers = Fingerprint::ZERO();
  /// Does *not* include the type-body hashes of the top level types.
  /// Just the `SourceFile` hashes.
  /// Used for incremental imports.
  Fingerprint InterfaceHashExcludingTypeMembers = Fingerprint::ZERO();
  toolchain::sys::TimePoint<> LastModified = {};
  uint64_t FileSize = 0;

  // Populate the from 'SF' member if exist. 'SF' will be cleared.
  void populateWithSourceFileIfNeeded();

public:
  BasicSourceFileInfo(StringRef FilePath,
                      Fingerprint InterfaceHashIncludingTypeMembers,
                      Fingerprint InterfaceHashExcludingTypeMembers,
                      toolchain::sys::TimePoint<> LastModified, uint64_t FileSize)
      : FilePath(FilePath),
        InterfaceHashIncludingTypeMembers(InterfaceHashIncludingTypeMembers),
        InterfaceHashExcludingTypeMembers(InterfaceHashExcludingTypeMembers),
        LastModified(LastModified), FileSize(FileSize) {}

  ///  Construct with a `SourceFile`. `getInterfaceHashIncludingTypeMembers()`,
  ///  `getInterfaceHashExcludingTypeMembers()`, `getLastModified()` and
  /// `getFileSize()` are lazily populated when accessed.
  BasicSourceFileInfo(const SourceFile *SF);

  bool isFromSourceFile() const;

  StringRef getFilePath() const { return FilePath; }

  Fingerprint getInterfaceHashIncludingTypeMembers() const {
    const_cast<BasicSourceFileInfo *>(this)->populateWithSourceFileIfNeeded();
    return InterfaceHashIncludingTypeMembers;
  }

  Fingerprint getInterfaceHashExcludingTypeMembers() const {
    const_cast<BasicSourceFileInfo *>(this)->populateWithSourceFileIfNeeded();
    return InterfaceHashExcludingTypeMembers;
  }

  toolchain::sys::TimePoint<> getLastModified() const {
    const_cast<BasicSourceFileInfo *>(this)->populateWithSourceFileIfNeeded();
    return LastModified;
  }

  uint64_t getFileSize() const {
    const_cast<BasicSourceFileInfo *>(this)->populateWithSourceFileIfNeeded();
    return FileSize;
  }
};

} // namespace language

#endif // LANGUAGE_BASIC_BASIC_SOURCE_INFO_H

