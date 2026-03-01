//===- LinearTransform.cpp - MLIR LinearTransform Class -------------------===//
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

#include "mlir/Analysis/Presburger/LinearTransform.h"
#include "mlir/Analysis/Presburger/IntegerRelation.h"
#include "mlir/Analysis/Presburger/Matrix.h"
#include <utility>

using namespace mlir;
using namespace presburger;

LinearTransform::LinearTransform(IntMatrix &&oMatrix) : matrix(oMatrix) {}
LinearTransform::LinearTransform(const IntMatrix &oMatrix) : matrix(oMatrix) {}

std::pair<unsigned, LinearTransform>
LinearTransform::makeTransformToColumnEchelon(const IntMatrix &m) {
  // Compute the hermite normal form of m. This, is by definition, is in column
  // echelon form.
  auto [h, u] = m.computeHermiteNormalForm();

  // Since the matrix is in column ecehlon form, a zero column means the rest of
  // the columns are zero. Thus, once we find a zero column, we can stop.
  unsigned col, e;
  for (col = 0, e = m.getNumColumns(); col < e; ++col) {
    bool zeroCol = true;
    for (unsigned row = 0, f = m.getNumRows(); row < f; ++row) {
      if (h(row, col) != 0) {
        zeroCol = false;
        break;
      }
    }

    if (zeroCol)
      break;
  }

  return {col, LinearTransform(std::move(u))};
}

IntegerRelation LinearTransform::applyTo(const IntegerRelation &rel) const {
  IntegerRelation result(rel.getSpace());

  for (unsigned i = 0, e = rel.getNumEqualities(); i < e; ++i) {
    ArrayRef<DynamicAPInt> eq = rel.getEquality(i);

    const DynamicAPInt &c = eq.back();

    SmallVector<DynamicAPInt, 8> newEq = preMultiplyWithRow(eq.drop_back());
    newEq.emplace_back(c);
    result.addEquality(newEq);
  }

  for (unsigned i = 0, e = rel.getNumInequalities(); i < e; ++i) {
    ArrayRef<DynamicAPInt> ineq = rel.getInequality(i);

    const DynamicAPInt &c = ineq.back();

    SmallVector<DynamicAPInt, 8> newIneq = preMultiplyWithRow(ineq.drop_back());
    newIneq.emplace_back(c);
    result.addInequality(newIneq);
  }

  return result;
}
