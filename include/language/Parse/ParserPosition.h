//===--- ParserPosition.h - Parser Position ---------------------*- C++ -*-===//
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
// Parser position where Parser can jump to.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_PARSE_PARSERPOSITION_H
#define LANGUAGE_PARSE_PARSERPOSITION_H

#include "language/Basic/SourceLoc.h"
#include "language/Parse/LexerState.h"

namespace language {

class ParserPosition {
  LexerState LS;
  SourceLoc PreviousLoc;
  friend class Parser;

  ParserPosition(LexerState LS, SourceLoc PreviousLoc)
      : LS(LS), PreviousLoc(PreviousLoc) {}
public:
  ParserPosition() = default;

  bool isValid() const { return LS.isValid(); }
};

} // end namespace language

#endif
