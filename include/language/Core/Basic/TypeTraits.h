//===--- TypeTraits.h - C++ Type Traits Support Enumerations ----*- C++ -*-===//
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
///
/// \file
/// Defines enumerations for the type traits support.
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_BASIC_TYPETRAITS_H
#define LANGUAGE_CORE_BASIC_TYPETRAITS_H

#include "toolchain/Support/Compiler.h"

namespace language::Core {
/// Names for traits that operate specifically on types.
enum TypeTrait {
#define TYPE_TRAIT_1(Spelling, Name, Key) UTT_##Name,
#include "language/Core/Basic/TokenKinds.def"
  UTT_Last = -1 // UTT_Last == last UTT_XX in the enum.
#define TYPE_TRAIT_1(Spelling, Name, Key) +1
#include "language/Core/Basic/TokenKinds.def"
  ,
#define TYPE_TRAIT_2(Spelling, Name, Key) BTT_##Name,
#include "language/Core/Basic/TokenKinds.def"
  BTT_Last = UTT_Last // BTT_Last == last BTT_XX in the enum.
#define TYPE_TRAIT_2(Spelling, Name, Key) +1
#include "language/Core/Basic/TokenKinds.def"
  ,
#define TYPE_TRAIT_N(Spelling, Name, Key) TT_##Name,
#include "language/Core/Basic/TokenKinds.def"
  TT_Last = BTT_Last // TT_Last == last TT_XX in the enum.
#define TYPE_TRAIT_N(Spelling, Name, Key) +1
#include "language/Core/Basic/TokenKinds.def"
};

/// Names for the array type traits.
enum ArrayTypeTrait {
#define ARRAY_TYPE_TRAIT(Spelling, Name, Key) ATT_##Name,
#include "language/Core/Basic/TokenKinds.def"
  ATT_Last = -1 // ATT_Last == last ATT_XX in the enum.
#define ARRAY_TYPE_TRAIT(Spelling, Name, Key) +1
#include "language/Core/Basic/TokenKinds.def"
};

/// Names for the "expression or type" traits.
enum UnaryExprOrTypeTrait {
#define UNARY_EXPR_OR_TYPE_TRAIT(Spelling, Name, Key) UETT_##Name,
#define CXX11_UNARY_EXPR_OR_TYPE_TRAIT(Spelling, Name, Key) UETT_##Name,
#include "language/Core/Basic/TokenKinds.def"
  UETT_Last = -1 // UETT_Last == last UETT_XX in the enum.
#define UNARY_EXPR_OR_TYPE_TRAIT(Spelling, Name, Key) +1
#define CXX11_UNARY_EXPR_OR_TYPE_TRAIT(Spelling, Name, Key) +1
#include "language/Core/Basic/TokenKinds.def"
};

/// Return the internal name of type trait \p T. Never null.
const char *getTraitName(TypeTrait T) LLVM_READONLY;
const char *getTraitName(ArrayTypeTrait T) LLVM_READONLY;
const char *getTraitName(UnaryExprOrTypeTrait T) LLVM_READONLY;

/// Return the spelling of the type trait \p TT. Never null.
const char *getTraitSpelling(TypeTrait T) LLVM_READONLY;
const char *getTraitSpelling(ArrayTypeTrait T) LLVM_READONLY;
const char *getTraitSpelling(UnaryExprOrTypeTrait T) LLVM_READONLY;

/// Return the arity of the type trait \p T.
unsigned getTypeTraitArity(TypeTrait T) LLVM_READONLY;

} // namespace language::Core

#endif
