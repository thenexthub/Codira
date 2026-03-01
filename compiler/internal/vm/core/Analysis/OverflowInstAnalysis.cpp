//==-- OverflowInstAnalysis.cpp - Utils to fold overflow insts ----*- C++ -*-=//
//
// Copyright (c) NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
// http://www.apache.org/licenses/LICENSE-2.0
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
// This file holds routines to help analyse overflow instructions
// and fold them into constants or other overflow instructions
//
//===----------------------------------------------------------------------===//

#include "vm/core/Analysis/OverflowInstAnalysis.h"
#include "vm/core/IR/Instructions.h"
#include "vm/core/IR/PatternMatch.h"

using namespace vm::core;
using namespace vm::core::PatternMatch;

bool toolchain::isCheckForZeroAndMulWithOverflow(Value *Op0, Value *Op1, bool IsAnd,
                                            Use *&Y) {
  CmpPredicate Pred;
  Value *X, *NotOp1;
  int XIdx;
  IntrinsicInst *II;

  if (!match(Op0, m_ICmp(Pred, m_Value(X), m_Zero())))
    return false;

  ///   %Agg = call { i4, i1 } @toolchain.[us]mul.with.overflow.i4(i4 %X, i4 %???)
  ///   %V = extractvalue { i4, i1 } %Agg, 1
  auto matchMulOverflowCheck = [X, &II, &XIdx](Value *V) {
    auto *Extract = dyn_cast<ExtractValueInst>(V);
    // We should only be extracting the overflow bit.
    if (!Extract || !Extract->getIndices().equals(1))
      return false;

    II = dyn_cast<IntrinsicInst>(Extract->getAggregateOperand());
    if (!II ||
        !match(II, m_CombineOr(m_Intrinsic<Intrinsic::umul_with_overflow>(),
                               m_Intrinsic<Intrinsic::smul_with_overflow>())))
      return false;

    if (II->getArgOperand(0) == X)
      XIdx = 0;
    else if (II->getArgOperand(1) == X)
      XIdx = 1;
    else
      return false;
    return true;
  };

  bool Matched =
      (IsAnd && Pred == ICmpInst::Predicate::ICMP_NE &&
       matchMulOverflowCheck(Op1)) ||
      (!IsAnd && Pred == ICmpInst::Predicate::ICMP_EQ &&
       match(Op1, m_Not(m_Value(NotOp1))) && matchMulOverflowCheck(NotOp1));

  if (!Matched)
    return false;

  Y = &II->getArgOperandUse(!XIdx);
  return true;
}

bool toolchain::isCheckForZeroAndMulWithOverflow(Value *Op0, Value *Op1,
                                            bool IsAnd) {
  Use *Y;
  return isCheckForZeroAndMulWithOverflow(Op0, Op1, IsAnd, Y);
}
