//===--- TypeTraits.cpp - Type Traits Support -----------------------------===//
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
//  This file implements the type traits support functions.
//
//===----------------------------------------------------------------------===//

#include "language/Core/Basic/TypeTraits.h"
#include <cassert>
#include <cstring>
using namespace language::Core;

static constexpr const char *TypeTraitNames[] = {
#define TYPE_TRAIT_1(Spelling, Name, Key) #Name,
#include "language/Core/Basic/TokenKinds.def"
#define TYPE_TRAIT_2(Spelling, Name, Key) #Name,
#include "language/Core/Basic/TokenKinds.def"
#define TYPE_TRAIT_N(Spelling, Name, Key) #Name,
#include "language/Core/Basic/TokenKinds.def"
};

static constexpr const char *TypeTraitSpellings[] = {
#define TYPE_TRAIT_1(Spelling, Name, Key) #Spelling,
#include "language/Core/Basic/TokenKinds.def"
#define TYPE_TRAIT_2(Spelling, Name, Key) #Spelling,
#include "language/Core/Basic/TokenKinds.def"
#define TYPE_TRAIT_N(Spelling, Name, Key) #Spelling,
#include "language/Core/Basic/TokenKinds.def"
};

static constexpr const char *ArrayTypeTraitNames[] = {
#define ARRAY_TYPE_TRAIT(Spelling, Name, Key) #Name,
#include "language/Core/Basic/TokenKinds.def"
};

static constexpr const char *ArrayTypeTraitSpellings[] = {
#define ARRAY_TYPE_TRAIT(Spelling, Name, Key) #Spelling,
#include "language/Core/Basic/TokenKinds.def"
};

static constexpr const char *UnaryExprOrTypeTraitNames[] = {
#define UNARY_EXPR_OR_TYPE_TRAIT(Spelling, Name, Key) #Name,
#define CXX11_UNARY_EXPR_OR_TYPE_TRAIT(Spelling, Name, Key) #Name,
#include "language/Core/Basic/TokenKinds.def"
};

static constexpr const char *UnaryExprOrTypeTraitSpellings[] = {
#define UNARY_EXPR_OR_TYPE_TRAIT(Spelling, Name, Key) #Spelling,
#define CXX11_UNARY_EXPR_OR_TYPE_TRAIT(Spelling, Name, Key) #Spelling,
#include "language/Core/Basic/TokenKinds.def"
};

static constexpr const unsigned TypeTraitArities[] = {
#define TYPE_TRAIT_1(Spelling, Name, Key) 1,
#include "language/Core/Basic/TokenKinds.def"
#define TYPE_TRAIT_2(Spelling, Name, Key) 2,
#include "language/Core/Basic/TokenKinds.def"
#define TYPE_TRAIT_N(Spelling, Name, Key) 0,
#include "language/Core/Basic/TokenKinds.def"
};

const char *language::Core::getTraitName(TypeTrait T) {
  assert(T <= TT_Last && "invalid enum value!");
  return TypeTraitNames[T];
}

const char *language::Core::getTraitName(ArrayTypeTrait T) {
  assert(T <= ATT_Last && "invalid enum value!");
  return ArrayTypeTraitNames[T];
}

const char *language::Core::getTraitName(UnaryExprOrTypeTrait T) {
  assert(T <= UETT_Last && "invalid enum value!");
  return UnaryExprOrTypeTraitNames[T];
}

const char *language::Core::getTraitSpelling(TypeTrait T) {
  assert(T <= TT_Last && "invalid enum value!");
  if (T == BTT_IsDeducible) {
    // The __is_deducible is an internal-only type trait. To hide it from
    // external users, we define it with an empty spelling name, preventing the
    // clang parser from recognizing its token kind.
    // However, other components such as the AST dump still require the real
    // type trait name. Therefore, we return the real name when needed.
    assert(std::strlen(TypeTraitSpellings[T]) == 0);
    return "__is_deducible";
  }
  return TypeTraitSpellings[T];
}

const char *language::Core::getTraitSpelling(ArrayTypeTrait T) {
  assert(T <= ATT_Last && "invalid enum value!");
  return ArrayTypeTraitSpellings[T];
}

const char *language::Core::getTraitSpelling(UnaryExprOrTypeTrait T) {
  assert(T <= UETT_Last && "invalid enum value!");
  return UnaryExprOrTypeTraitSpellings[T];
}

unsigned language::Core::getTypeTraitArity(TypeTrait T) {
  assert(T <= TT_Last && "invalid enum value!");
  return TypeTraitArities[T];
}
