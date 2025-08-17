//===-- lib/Evaluate/fold-matmul.h ----------------------------------------===//
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

#ifndef LANGUAGE_COMPABILITY_EVALUATE_FOLD_MATMUL_H_
#define LANGUAGE_COMPABILITY_EVALUATE_FOLD_MATMUL_H_

#include "fold-implementation.h"

namespace language::Compability::evaluate {

template <typename T>
static Expr<T> FoldMatmul(FoldingContext &context, FunctionRef<T> &&funcRef) {
  using Element = typename Constant<T>::Element;
  auto args{funcRef.arguments()};
  CHECK(args.size() == 2);
  Folder<T> folder{context};
  Constant<T> *ma{folder.Folding(args[0])};
  Constant<T> *mb{folder.Folding(args[1])};
  if (!ma || !mb) {
    return Expr<T>{std::move(funcRef)};
  }
  CHECK(ma->Rank() >= 1 && ma->Rank() <= 2 && mb->Rank() >= 1 &&
      mb->Rank() <= 2 && (ma->Rank() == 2 || mb->Rank() == 2));
  ConstantSubscript commonExtent{ma->shape().back()};
  if (mb->shape().front() != commonExtent) {
    context.messages().Say(
        "Arguments to MATMUL have distinct extents %zd and %zd on their last and first dimensions"_err_en_US,
        commonExtent, mb->shape().front());
    return MakeInvalidIntrinsic(std::move(funcRef));
  }
  ConstantSubscript rows{ma->Rank() == 1 ? 1 : ma->shape()[0]};
  ConstantSubscript columns{mb->Rank() == 1 ? 1 : mb->shape()[1]};
  std::vector<Element> elements;
  elements.reserve(rows * columns);
  bool overflow{false};
  [[maybe_unused]] const auto &rounding{
      context.targetCharacteristics().roundingMode()};
  // result(j,k) = SUM(A(j,:) * B(:,k))
  for (ConstantSubscript ci{0}; ci < columns; ++ci) {
    for (ConstantSubscript ri{0}; ri < rows; ++ri) {
      ConstantSubscripts aAt{ma->lbounds()};
      if (ma->Rank() == 2) {
        aAt[0] += ri;
      }
      ConstantSubscripts bAt{mb->lbounds()};
      if (mb->Rank() == 2) {
        bAt[1] += ci;
      }
      Element sum{};
      [[maybe_unused]] Element correction{};
      for (ConstantSubscript j{0}; j < commonExtent; ++j) {
        Element aElt{ma->At(aAt)};
        Element bElt{mb->At(bAt)};
        if constexpr (T::category == TypeCategory::Real ||
            T::category == TypeCategory::Complex) {
          auto product{aElt.Multiply(bElt)};
          overflow |= product.flags.test(RealFlag::Overflow);
          if constexpr (useKahanSummation) {
            auto next{product.value.Subtract(correction, rounding)};
            overflow |= next.flags.test(RealFlag::Overflow);
            auto added{sum.Add(next.value, rounding)};
            overflow |= added.flags.test(RealFlag::Overflow);
            correction = added.value.Subtract(sum, rounding)
                             .value.Subtract(next.value, rounding)
                             .value;
            sum = std::move(added.value);
          } else {
            auto added{sum.Add(product.value)};
            overflow |= added.flags.test(RealFlag::Overflow);
            sum = std::move(added.value);
          }
        } else if constexpr (T::category == TypeCategory::Integer) {
          auto product{aElt.MultiplySigned(bElt)};
          overflow |= product.SignedMultiplicationOverflowed();
          auto added{sum.AddSigned(product.lower)};
          overflow |= added.overflow;
          sum = std::move(added.value);
        } else if constexpr (T::category == TypeCategory::Unsigned) {
          sum = sum.AddUnsigned(aElt.MultiplyUnsigned(bElt).lower).value;
        } else {
          static_assert(T::category == TypeCategory::Logical);
          sum = sum.OR(aElt.AND(bElt));
        }
        ++aAt.back();
        ++bAt.front();
      }
      elements.push_back(sum);
    }
  }
  if (overflow &&
      context.languageFeatures().ShouldWarn(
          common::UsageWarning::FoldingException)) {
    context.messages().Say(common::UsageWarning::FoldingException,
        "MATMUL of %s data overflowed during computation"_warn_en_US,
        T::AsFortran());
  }
  ConstantSubscripts shape;
  if (ma->Rank() == 2) {
    shape.push_back(rows);
  }
  if (mb->Rank() == 2) {
    shape.push_back(columns);
  }
  return Expr<T>{Constant<T>{std::move(elements), std::move(shape)}};
}
} // namespace language::Compability::evaluate
#endif // FORTRAN_EVALUATE_FOLD_MATMUL_H_
