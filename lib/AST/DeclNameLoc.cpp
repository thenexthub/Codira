//===--- DeclNameLoc.cpp - Declaration Name Location Info -----------------===//
//
// This source file is part of the Swift.org open source project
//
// Copyright (c) 2014 - 2017 Apple Inc. and the Swift project authors
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See https://swift.org/LICENSE.txt for license information
// See https://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
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
