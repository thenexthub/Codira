//===--- SelectorLocationsKind.cpp - Kind of selector locations -*- C++ -*-===//
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
// Describes whether the identifier locations for a selector are "standard"
// or not.
//
//===----------------------------------------------------------------------===//

#include "language/Core/AST/SelectorLocationsKind.h"
#include "language/Core/AST/Expr.h"

using namespace language::Core;

static SourceLocation getStandardSelLoc(unsigned Index,
                                        Selector Sel,
                                        bool WithArgSpace,
                                        SourceLocation ArgLoc,
                                        SourceLocation EndLoc) {
  unsigned NumSelArgs = Sel.getNumArgs();
  if (NumSelArgs == 0) {
    assert(Index == 0);
    if (EndLoc.isInvalid())
      return SourceLocation();
    const IdentifierInfo *II = Sel.getIdentifierInfoForSlot(0);
    unsigned Len = II ? II->getLength() : 0;
    return EndLoc.getLocWithOffset(-Len);
  }

  assert(Index < NumSelArgs);
  if (ArgLoc.isInvalid())
    return SourceLocation();
  const IdentifierInfo *II = Sel.getIdentifierInfoForSlot(Index);
  unsigned Len = /* selector id */ (II ? II->getLength() : 0) + /* ':' */ 1;
  if (WithArgSpace)
    ++Len;
  return ArgLoc.getLocWithOffset(-Len);
}

namespace {

template <typename T>
SourceLocation getArgLoc(T* Arg);

template <>
SourceLocation getArgLoc<Expr>(Expr *Arg) {
  return Arg->getBeginLoc();
}

template <>
SourceLocation getArgLoc<ParmVarDecl>(ParmVarDecl *Arg) {
  SourceLocation Loc = Arg->getBeginLoc();
  if (Loc.isInvalid())
    return Loc;
  // -1 to point to left paren of the method parameter's type.
  return Loc.getLocWithOffset(-1);
}

template <typename T>
SourceLocation getArgLoc(unsigned Index, ArrayRef<T*> Args) {
  return Index < Args.size() ? getArgLoc(Args[Index]) : SourceLocation();
}

template <typename T>
SelectorLocationsKind hasStandardSelLocs(Selector Sel,
                                         ArrayRef<SourceLocation> SelLocs,
                                         ArrayRef<T *> Args,
                                         SourceLocation EndLoc) {
  // Are selector locations in standard position with no space between args ?
  unsigned i;
  for (i = 0; i != SelLocs.size(); ++i) {
    if (SelLocs[i] != getStandardSelectorLoc(i, Sel, /*WithArgSpace=*/false,
                                             Args, EndLoc))
      break;
  }
  if (i == SelLocs.size())
    return SelLoc_StandardNoSpace;

  // Are selector locations in standard position with space between args ?
  for (i = 0; i != SelLocs.size(); ++i) {
    if (SelLocs[i] != getStandardSelectorLoc(i, Sel, /*WithArgSpace=*/true,
                                             Args, EndLoc))
      return SelLoc_NonStandard;
  }

  return SelLoc_StandardWithSpace;
}

} // anonymous namespace

SelectorLocationsKind
language::Core::hasStandardSelectorLocs(Selector Sel,
                               ArrayRef<SourceLocation> SelLocs,
                               ArrayRef<Expr *> Args,
                               SourceLocation EndLoc) {
  return hasStandardSelLocs(Sel, SelLocs, Args, EndLoc);
}

SourceLocation language::Core::getStandardSelectorLoc(unsigned Index,
                                             Selector Sel,
                                             bool WithArgSpace,
                                             ArrayRef<Expr *> Args,
                                             SourceLocation EndLoc) {
  return getStandardSelLoc(Index, Sel, WithArgSpace,
                           getArgLoc(Index, Args), EndLoc);
}

SelectorLocationsKind
language::Core::hasStandardSelectorLocs(Selector Sel,
                               ArrayRef<SourceLocation> SelLocs,
                               ArrayRef<ParmVarDecl *> Args,
                               SourceLocation EndLoc) {
  return hasStandardSelLocs(Sel, SelLocs, Args, EndLoc);
}

SourceLocation language::Core::getStandardSelectorLoc(unsigned Index,
                                             Selector Sel,
                                             bool WithArgSpace,
                                             ArrayRef<ParmVarDecl *> Args,
                                             SourceLocation EndLoc) {
  return getStandardSelLoc(Index, Sel, WithArgSpace,
                           getArgLoc(Index, Args), EndLoc);
}
