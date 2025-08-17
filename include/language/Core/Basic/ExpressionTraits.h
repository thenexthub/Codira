//===- ExpressionTraits.h - C++ Expression Traits Support Enums -*- C++ -*-===//
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
/// Defines enumerations for expression traits intrinsics.
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_BASIC_EXPRESSIONTRAITS_H
#define LANGUAGE_CORE_BASIC_EXPRESSIONTRAITS_H

#include "toolchain/Support/Compiler.h"

namespace language::Core {

enum ExpressionTrait {
#define EXPRESSION_TRAIT(Spelling, Name, Key) ET_##Name,
#include "language/Core/Basic/TokenKinds.def"
  ET_Last = -1 // ET_Last == last ET_XX in the enum.
#define EXPRESSION_TRAIT(Spelling, Name, Key) +1
#include "language/Core/Basic/TokenKinds.def"
};

/// Return the internal name of type trait \p T. Never null.
const char *getTraitName(ExpressionTrait T) LLVM_READONLY;

/// Return the spelling of the type trait \p TT. Never null.
const char *getTraitSpelling(ExpressionTrait T) LLVM_READONLY;

} // namespace language::Core

#endif
