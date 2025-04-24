//===--- BasicSourceInfo.cpp - Simple source information ------------------===//
//
// This source file is part of the Swift.org open source project
//
// Copyright (c) 2014 - 2021 Apple Inc. and the Swift project authors
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See https://swift.org/LICENSE.txt for license information
// See https://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
//
//===----------------------------------------------------------------------===//

#include "language/AST/ASTContext.h"
#include "language/AST/SourceFile.h"
#include "language/Basic/BasicSourceInfo.h"
#include "language/Basic/Defer.h"
#include "language/Basic/SourceManager.h"

using namespace language;

BasicSourceFileInfo::BasicSourceFileInfo(const SourceFile *SF)
    : SFAndIsFromSF(SF, true) {
  FilePath = SF->getFilename();
}

bool BasicSourceFileInfo::isFromSourceFile() const {
  return SFAndIsFromSF.getInt();
}

void BasicSourceFileInfo::populateWithSourceFileIfNeeded() {
  const auto *SF = SFAndIsFromSF.getPointer();
  if (!SF)
    return;
  SWIFT_DEFER {
    SFAndIsFromSF.setPointer(nullptr);
  };

  SourceManager &SM = SF->getASTContext().SourceMgr;

  if (FilePath.empty())
    return;
  auto stat = SM.getFileSystem()->status(FilePath);
  if (!stat)
    return;

  LastModified = stat->getLastModificationTime();
  FileSize = stat->getSize();

  if (SF->hasInterfaceHash()) {
    InterfaceHashIncludingTypeMembers = SF->getInterfaceHashIncludingTypeMembers();
    InterfaceHashExcludingTypeMembers = SF->getInterfaceHash();
  } else {
    // FIXME: Parse the file with EnableInterfaceHash option.
    InterfaceHashIncludingTypeMembers = Fingerprint::ZERO();
    InterfaceHashExcludingTypeMembers = Fingerprint::ZERO();
  }

  return;
}
