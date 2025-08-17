//===--- SemaFixItUtils.h - Sema FixIts -------------------------*- C++ -*-===//
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
//  This file defines helper classes for generation of Sema FixItHints.
//
//===----------------------------------------------------------------------===//
#ifndef LANGUAGE_CORE_SEMA_SEMAFIXITUTILS_H
#define LANGUAGE_CORE_SEMA_SEMAFIXITUTILS_H

#include "language/Core/AST/Expr.h"

namespace language::Core {

enum OverloadFixItKind {
  OFIK_Undefined = 0,
  OFIK_Dereference,
  OFIK_TakeAddress,
  OFIK_RemoveDereference,
  OFIK_RemoveTakeAddress
};

class Sema;

/// The class facilities generation and storage of conversion FixIts. Hints for
/// new conversions are added using TryToFixConversion method. The default type
/// conversion checker can be reset.
struct ConversionFixItGenerator {
  /// Performs a simple check to see if From type can be converted to To type.
  static bool compareTypesSimple(CanQualType From,
                                 CanQualType To,
                                 Sema &S,
                                 SourceLocation Loc,
                                 ExprValueKind FromVK);

  /// The list of Hints generated so far.
  std::vector<FixItHint> Hints;

  /// The number of Conversions fixed. This can be different from the size
  /// of the Hints vector since we allow multiple FixIts per conversion.
  unsigned NumConversionsFixed;

  /// The type of fix applied. If multiple conversions are fixed, corresponds
  /// to the kid of the very first conversion.
  OverloadFixItKind Kind;

  typedef bool (*TypeComparisonFuncTy) (const CanQualType FromTy,
                                        const CanQualType ToTy,
                                        Sema &S,
                                        SourceLocation Loc,
                                        ExprValueKind FromVK);
  /// The type comparison function used to decide if expression FromExpr of
  /// type FromTy can be converted to ToTy. For example, one could check if
  /// an implicit conversion exists. Returns true if comparison exists.
  TypeComparisonFuncTy CompareTypes;

  ConversionFixItGenerator(TypeComparisonFuncTy Foo): NumConversionsFixed(0),
                                                      Kind(OFIK_Undefined),
                                                      CompareTypes(Foo) {}

  ConversionFixItGenerator(): NumConversionsFixed(0),
                              Kind(OFIK_Undefined),
                              CompareTypes(compareTypesSimple) {}

  /// Resets the default conversion checker method.
  void setConversionChecker(TypeComparisonFuncTy Foo) {
    CompareTypes = Foo;
  }

  /// If possible, generates and stores a fix for the given conversion.
  bool tryToFixConversion(const Expr *FromExpr,
                          const QualType FromQTy, const QualType ToQTy,
                          Sema &S);

  void clear() {
    Hints.clear();
    NumConversionsFixed = 0;
  }

  bool isNull() {
    return (NumConversionsFixed == 0);
  }
};

} // endof namespace language::Core
#endif
