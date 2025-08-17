//= ObjCNoReturn.h - Handling of Cocoa APIs known not to return --*- C++ -*---//
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
// This file implements special handling of recognizing ObjC API hooks that
// do not return but aren't marked as such in API headers.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_ANALYSIS_DOMAINSPECIFIC_OBJCNORETURN_H
#define LANGUAGE_CORE_ANALYSIS_DOMAINSPECIFIC_OBJCNORETURN_H

#include "language/Core/Basic/IdentifierTable.h"

namespace language::Core {

class ASTContext;
class ObjCMessageExpr;

class ObjCNoReturn {
  /// Cached "raise" selector.
  Selector RaiseSel;

  /// Cached identifier for "NSException".
  IdentifierInfo *NSExceptionII;

  enum { NUM_RAISE_SELECTORS = 2 };

  /// Cached set of selectors in NSException that are 'noreturn'.
  Selector NSExceptionInstanceRaiseSelectors[NUM_RAISE_SELECTORS];

public:
  ObjCNoReturn(ASTContext &C);

  /// Return true if the given message expression is known to never
  /// return.
  bool isImplicitNoReturn(const ObjCMessageExpr *ME);
};
}

#endif
