//===--- ExpressionTraits.cpp - Expression Traits Support -----------------===//
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
//  This file implements the expression traits support functions.
//
//===----------------------------------------------------------------------===//

#include "language/Core/Basic/ExpressionTraits.h"
#include <cassert>
using namespace language::Core;

static constexpr const char *ExpressionTraitNames[] = {
#define EXPRESSION_TRAIT(Spelling, Name, Key) #Name,
#include "language/Core/Basic/TokenKinds.def"
};

static constexpr const char *ExpressionTraitSpellings[] = {
#define EXPRESSION_TRAIT(Spelling, Name, Key) #Spelling,
#include "language/Core/Basic/TokenKinds.def"
};

const char *language::Core::getTraitName(ExpressionTrait T) {
  assert(T <= ET_Last && "invalid enum value!");
  return ExpressionTraitNames[T];
}

const char *language::Core::getTraitSpelling(ExpressionTrait T) {
  assert(T <= ET_Last && "invalid enum value!");
  return ExpressionTraitSpellings[T];
}
