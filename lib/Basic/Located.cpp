//===--- Located.cpp - Source Location and Associated Value ----------*- C++ -*-===//
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

#include "toolchain/Support/raw_ostream.h"
#include "language/Basic/Located.h"

using namespace language;

template<typename T>
void Located<T>::dump() const {
  dump(toolchain::errs());
}

template<typename T>
void Located<T>::dump(raw_ostream &os) const {
  // FIXME: The following does not compile on newer clangs because operator<<
  // does not exist for SourceLoc. More so, the operator does not exist because
  // one needs a SourceManager reference and buffer ID to convert any given
  // SourceLoc into line and column information.
  //os << Loc << " " << Item;
}
