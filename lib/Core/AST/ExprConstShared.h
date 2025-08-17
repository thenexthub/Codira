//===--- ExprConstShared.h - Shared consetxpr functionality ----*- C++ -*-===//
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
// Shared functionality between the new constant expression
// interpreter (AST/ByteCode/) and the current one (ExprConstant.cpp).
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_LIB_AST_EXPRCONSTSHARED_H
#define LANGUAGE_CORE_LIB_AST_EXPRCONSTSHARED_H

#include "language/Core/Basic/TypeTraits.h"

namespace toolchain {
class APFloat;
}
namespace language::Core {
class QualType;
class LangOptions;
class ASTContext;
class CharUnits;
class Expr;
} // namespace language::Core
using namespace language::Core;
/// Values returned by __builtin_classify_type, chosen to match the values
/// produced by GCC's builtin.
enum class GCCTypeClass {
  None = -1,
  Void = 0,
  Integer = 1,
  // GCC reserves 2 for character types, but instead classifies them as
  // integers.
  Enum = 3,
  Bool = 4,
  Pointer = 5,
  // GCC reserves 6 for references, but appears to never use it (because
  // expressions never have reference type, presumably).
  PointerToDataMember = 7,
  RealFloat = 8,
  Complex = 9,
  // GCC reserves 10 for functions, but does not use it since GCC version 6 due
  // to decay to pointer. (Prior to version 6 it was only used in C++ mode).
  // GCC claims to reserve 11 for pointers to member functions, but *actually*
  // uses 12 for that purpose, same as for a class or struct. Maybe it
  // internally implements a pointer to member as a struct?  Who knows.
  PointerToMemberFunction = 12, // Not a bug, see above.
  ClassOrStruct = 12,
  Union = 13,
  // GCC reserves 14 for arrays, but does not use it since GCC version 6 due to
  // decay to pointer. (Prior to version 6 it was only used in C++ mode).
  // GCC reserves 15 for strings, but actually uses 5 (pointer) for string
  // literals.
  // Lang = 16,
  // OpaqueType = 17,
  BitInt = 18,
  Vector = 19
};

GCCTypeClass EvaluateBuiltinClassifyType(QualType T,
                                         const LangOptions &LangOpts);

void HandleComplexComplexMul(toolchain::APFloat A, toolchain::APFloat B, toolchain::APFloat C,
                             toolchain::APFloat D, toolchain::APFloat &ResR,
                             toolchain::APFloat &ResI);
void HandleComplexComplexDiv(toolchain::APFloat A, toolchain::APFloat B, toolchain::APFloat C,
                             toolchain::APFloat D, toolchain::APFloat &ResR,
                             toolchain::APFloat &ResI);

CharUnits GetAlignOfExpr(const ASTContext &Ctx, const Expr *E,
                         UnaryExprOrTypeTrait ExprKind);

#endif
