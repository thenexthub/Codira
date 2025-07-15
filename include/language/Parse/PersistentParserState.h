//===--- PersistentParserState.h - Parser State -----------------*- C++ -*-===//
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
// Parser state persistent across multiple parses.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_PARSE_PERSISTENTPARSERSTATE_H
#define LANGUAGE_PARSE_PERSISTENTPARSERSTATE_H

#include "language/Basic/SourceLoc.h"

namespace language {

class SourceFile;
class DeclContext;
class IterableDeclContext;

enum class IDEInspectionDelayedDeclKind {
  TopLevelCodeDecl,
  Decl,
  FunctionBody,
};

class IDEInspectionDelayedDeclState {
public:
  IDEInspectionDelayedDeclKind Kind;
  DeclContext *ParentContext;
  unsigned StartOffset;
  unsigned EndOffset;
  unsigned PrevOffset;

  IDEInspectionDelayedDeclState(IDEInspectionDelayedDeclKind Kind,
                                DeclContext *ParentContext,
                                unsigned StartOffset, unsigned EndOffset,
                                unsigned PrevOffset)
      : Kind(Kind), ParentContext(ParentContext), StartOffset(StartOffset),
        EndOffset(EndOffset), PrevOffset(PrevOffset) {}
};

/// Parser state persistent across multiple parses.
class PersistentParserState {
  std::unique_ptr<IDEInspectionDelayedDeclState> IDEInspectionDelayedDeclStat;

public:
  PersistentParserState();
  PersistentParserState(ASTContext &ctx) : PersistentParserState() { }
  ~PersistentParserState();

  void setIDEInspectionDelayedDeclState(SourceManager &SM, unsigned BufferID,
                                        IDEInspectionDelayedDeclKind Kind,
                                        DeclContext *ParentContext,
                                        SourceRange BodyRange,
                                        SourceLoc PreviousLoc);
  void restoreIDEInspectionDelayedDeclState(
      const IDEInspectionDelayedDeclState &other);

  bool hasIDEInspectionDelayedDeclState() const {
    return IDEInspectionDelayedDeclStat.get() != nullptr;
  }

  IDEInspectionDelayedDeclState &getIDEInspectionDelayedDeclState() {
    return *IDEInspectionDelayedDeclStat.get();
  }
  const IDEInspectionDelayedDeclState &
  getIDEInspectionDelayedDeclState() const {
    return *IDEInspectionDelayedDeclStat.get();
  }

  std::unique_ptr<IDEInspectionDelayedDeclState>
  takeIDEInspectionDelayedDeclState() {
    assert(hasIDEInspectionDelayedDeclState());
    return std::move(IDEInspectionDelayedDeclStat);
  }
};

} // end namespace language

#endif
