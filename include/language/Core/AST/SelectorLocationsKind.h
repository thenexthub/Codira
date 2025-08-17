//===--- SelectorLocationsKind.h - Kind of selector locations ---*- C++ -*-===//
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

#ifndef LANGUAGE_CORE_AST_SELECTORLOCATIONSKIND_H
#define LANGUAGE_CORE_AST_SELECTORLOCATIONSKIND_H

#include "language/Core/Basic/LLVM.h"

namespace language::Core {
  class Selector;
  class SourceLocation;
  class Expr;
  class ParmVarDecl;

/// Whether all locations of the selector identifiers are in a
/// "standard" position.
enum SelectorLocationsKind {
  /// Non-standard.
  SelLoc_NonStandard = 0,

  /// For nullary selectors, immediately before the end:
  ///    "[foo release]" / "-(void)release;"
  /// Or immediately before the arguments:
  ///    "[foo first:1 second:2]" / "-(id)first:(int)x second:(int)y;
  SelLoc_StandardNoSpace = 1,

  /// For nullary selectors, immediately before the end:
  ///    "[foo release]" / "-(void)release;"
  /// Or with a space between the arguments:
  ///    "[foo first: 1 second: 2]" / "-(id)first: (int)x second: (int)y;
  SelLoc_StandardWithSpace = 2
};

/// Returns true if all \p SelLocs are in a "standard" location.
SelectorLocationsKind hasStandardSelectorLocs(Selector Sel,
                                              ArrayRef<SourceLocation> SelLocs,
                                              ArrayRef<Expr *> Args,
                                              SourceLocation EndLoc);

/// Get the "standard" location of a selector identifier, e.g:
/// For nullary selectors, immediately before ']': "[foo release]"
///
/// \param WithArgSpace if true the standard location is with a space apart
/// before arguments: "[foo first: 1 second: 2]"
/// If false: "[foo first:1 second:2]"
SourceLocation getStandardSelectorLoc(unsigned Index,
                                      Selector Sel,
                                      bool WithArgSpace,
                                      ArrayRef<Expr *> Args,
                                      SourceLocation EndLoc);

/// Returns true if all \p SelLocs are in a "standard" location.
SelectorLocationsKind hasStandardSelectorLocs(Selector Sel,
                                              ArrayRef<SourceLocation> SelLocs,
                                              ArrayRef<ParmVarDecl *> Args,
                                              SourceLocation EndLoc);

/// Get the "standard" location of a selector identifier, e.g:
/// For nullary selectors, immediately before ']': "[foo release]"
///
/// \param WithArgSpace if true the standard location is with a space apart
/// before arguments: "-(id)first: (int)x second: (int)y;"
/// If false: "-(id)first:(int)x second:(int)y;"
SourceLocation getStandardSelectorLoc(unsigned Index,
                                      Selector Sel,
                                      bool WithArgSpace,
                                      ArrayRef<ParmVarDecl *> Args,
                                      SourceLocation EndLoc);

} // end namespace language::Core

#endif
