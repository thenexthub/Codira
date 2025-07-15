//===--- DeclNameLoc.cpp - Declaration Name Location Info -----------------===//
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
//  This file implements the DeclNameLoc class.
//
//===----------------------------------------------------------------------===//

#include "language/AST/DeclNameLoc.h"
#include "language/AST/ASTContext.h"
#include "language/Basic/Assertions.h"

using namespace language;

DeclNameLoc::DeclNameLoc(ASTContext &ctx, SourceLoc baseNameLoc,
                         SourceLoc lParenLoc,
                         ArrayRef<SourceLoc> argumentLabelLocs,
                         SourceLoc rParenLoc)
  : NumArgumentLabels(argumentLabelLocs.size()) {
  assert(NumArgumentLabels > 0 && "Use other constructor");

  // Copy the location information into permanent storage.
  auto storedLocs = ctx.Allocate<SourceLoc>(NumArgumentLabels + 3);
  storedLocs[BaseNameIndex] = baseNameLoc;
  storedLocs[LParenIndex] = lParenLoc;
  storedLocs[RParenIndex] = rParenLoc;
  std::memcpy(storedLocs.data() + FirstArgumentLabelIndex,
              argumentLabelLocs.data(),
              argumentLabelLocs.size() * sizeof(SourceLoc));

  LocationInfo = storedLocs.data();
}
