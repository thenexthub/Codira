//===- ThreadSafetyLogical.cpp ---------------------------------*- C++ --*-===//
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
// This file defines a representation for logical expressions with SExpr leaves
// that are used as part of fact-checking capability expressions.
//===----------------------------------------------------------------------===//

#include "language/Core/Analysis/Analyses/ThreadSafetyLogical.h"

using namespace toolchain;
using namespace language::Core::threadSafety::lexpr;

// Implication.  We implement De Morgan's Laws by maintaining LNeg and RNeg
// to keep track of whether LHS and RHS are negated.
static bool implies(const LExpr *LHS, bool LNeg, const LExpr *RHS, bool RNeg) {
  // In comments below, we write => for implication.

  // Calculates the logical AND implication operator.
  const auto LeftAndOperator = [=](const BinOp *A) {
    return implies(A->left(), LNeg, RHS, RNeg) &&
           implies(A->right(), LNeg, RHS, RNeg);
  };
  const auto RightAndOperator = [=](const BinOp *A) {
    return implies(LHS, LNeg, A->left(), RNeg) &&
           implies(LHS, LNeg, A->right(), RNeg);
  };

  // Calculates the logical OR implication operator.
  const auto LeftOrOperator = [=](const BinOp *A) {
    return implies(A->left(), LNeg, RHS, RNeg) ||
           implies(A->right(), LNeg, RHS, RNeg);
  };
  const auto RightOrOperator = [=](const BinOp *A) {
    return implies(LHS, LNeg, A->left(), RNeg) ||
           implies(LHS, LNeg, A->right(), RNeg);
  };

  // Recurse on right.
  switch (RHS->kind()) {
  case LExpr::And:
    // When performing right recursion:
    //   C => A & B  [if]  C => A and C => B
    // When performing right recursion (negated):
    //   C => !(A & B)  [if]  C => !A | !B  [===]  C => !A or C => !B
    return RNeg ? RightOrOperator(cast<And>(RHS))
                : RightAndOperator(cast<And>(RHS));
  case LExpr::Or:
    // When performing right recursion:
    //   C => (A | B)  [if]  C => A or C => B
    // When performing right recursion (negated):
    //   C => !(A | B)  [if]  C => !A & !B  [===]  C => !A and C => !B
    return RNeg ? RightAndOperator(cast<Or>(RHS))
                : RightOrOperator(cast<Or>(RHS));
  case LExpr::Not:
    // Note that C => !A is very different from !(C => A). It would be incorrect
    // to return !implies(LHS, RHS).
    return implies(LHS, LNeg, cast<Not>(RHS)->exp(), !RNeg);
  case LExpr::Terminal:
    // After reaching the terminal, it's time to recurse on the left.
    break;
  }

  // RHS is now a terminal.  Recurse on Left.
  switch (LHS->kind()) {
  case LExpr::And:
    // When performing left recursion:
    //   A & B => C  [if]  A => C or B => C
    // When performing left recursion (negated):
    //   !(A & B) => C  [if]  !A | !B => C  [===]  !A => C and !B => C
    return LNeg ? LeftAndOperator(cast<And>(LHS))
                : LeftOrOperator(cast<And>(LHS));
  case LExpr::Or:
    // When performing left recursion:
    //   A | B => C  [if]  A => C and B => C
    // When performing left recursion (negated):
    //   !(A | B) => C  [if]  !A & !B => C  [===]  !A => C or !B => C
    return LNeg ? LeftOrOperator(cast<Or>(LHS))
                : LeftAndOperator(cast<Or>(LHS));
  case LExpr::Not:
    // Note that A => !C is very different from !(A => C). It would be incorrect
    // to return !implies(LHS, RHS).
    return implies(cast<Not>(LHS)->exp(), !LNeg, RHS, RNeg);
  case LExpr::Terminal:
    // After reaching the terminal, it's time to perform identity comparisons.
    break;
  }

  // A => A
  // !A => !A
  if (LNeg != RNeg)
    return false;

  // FIXME -- this should compare SExprs for equality, not pointer equality.
  return cast<Terminal>(LHS)->expr() == cast<Terminal>(RHS)->expr();
}

namespace language::Core {
namespace threadSafety {
namespace lexpr {

bool implies(const LExpr *LHS, const LExpr *RHS) {
  // Start out by assuming that LHS and RHS are not negated.
  return ::implies(LHS, false, RHS, false);
}
}
}
}
