//===-- lib/Evaluate/fold-complex.cpp -------------------------------------===//
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

#include "fold-implementation.h"
#include "fold-matmul.h"
#include "fold-reduction.h"

namespace language::Compability::evaluate {

template <int KIND>
Expr<Type<TypeCategory::Complex, KIND>> FoldIntrinsicFunction(
    FoldingContext &context,
    FunctionRef<Type<TypeCategory::Complex, KIND>> &&funcRef) {
  using T = Type<TypeCategory::Complex, KIND>;
  using Part = typename T::Part;
  ActualArguments &args{funcRef.arguments()};
  auto *intrinsic{std::get_if<SpecificIntrinsic>(&funcRef.proc().u)};
  CHECK(intrinsic);
  std::string name{intrinsic->name};
  if (name == "acos" || name == "acosh" || name == "asin" || name == "asinh" ||
      name == "atan" || name == "atanh" || name == "cos" || name == "cosh" ||
      name == "exp" || name == "log" || name == "sin" || name == "sinh" ||
      name == "sqrt" || name == "tan" || name == "tanh") {
    if (auto callable{GetHostRuntimeWrapper<T, T>(name)}) {
      return FoldElementalIntrinsic<T, T>(
          context, std::move(funcRef), *callable);
    } else if (context.languageFeatures().ShouldWarn(
                   common::UsageWarning::FoldingFailure)) {
      context.messages().Say(common::UsageWarning::FoldingFailure,
          "%s(complex(kind=%d)) cannot be folded on host"_warn_en_US, name,
          KIND);
    }
  } else if (name == "conjg") {
    return FoldElementalIntrinsic<T, T>(
        context, std::move(funcRef), &Scalar<T>::CONJG);
  } else if (name == "cmplx") {
    if (args.size() > 0 && args[0].has_value()) {
      if (auto *x{UnwrapExpr<Expr<SomeComplex>>(args[0])}) {
        // CMPLX(X [, KIND]) with complex X
        return Fold(context, ConvertToType<T>(std::move(*x)));
      } else {
        if (args.size() >= 2 && args[1].has_value()) {
          // Do not fold CMPLX with an Y argument that may be absent at runtime
          // into a complex constructor so that lowering can deal with the
          // optional aspect (there is no optional aspect with the complex
          // constructor).
          if (MayBePassedAsAbsentOptional(*args[1]->UnwrapExpr())) {
            return Expr<T>{std::move(funcRef)};
          }
        }
        // CMPLX(X [, Y [, KIND]]) with non-complex X
        Expr<SomeType> re{std::move(*args[0].value().UnwrapExpr())};
        Expr<SomeType> im{args.size() >= 2 && args[1].has_value()
                ? std::move(*args[1]->UnwrapExpr())
                : AsGenericExpr(Constant<Part>{Scalar<Part>{}})};
        return Fold(context,
            Expr<T>{
                ComplexConstructor<KIND>{ToReal<KIND>(context, std::move(re)),
                    ToReal<KIND>(context, std::move(im))}});
      }
    }
  } else if (name == "dot_product") {
    return FoldDotProduct<T>(context, std::move(funcRef));
  } else if (name == "matmul") {
    return FoldMatmul(context, std::move(funcRef));
  } else if (name == "product") {
    auto one{Scalar<Part>::FromInteger(value::Integer<8>{1}).value};
    return FoldProduct<T>(context, std::move(funcRef), Scalar<T>{one});
  } else if (name == "sum") {
    return FoldSum<T>(context, std::move(funcRef));
  }
  return Expr<T>{std::move(funcRef)};
}

template <int KIND>
Expr<Type<TypeCategory::Complex, KIND>> FoldOperation(
    FoldingContext &context, ComplexConstructor<KIND> &&x) {
  if (auto array{ApplyElementwise(context, x)}) {
    return *array;
  }
  using ComplexType = Type<TypeCategory::Complex, KIND>;
  if (auto folded{OperandsAreConstants(x)}) {
    using RealType = typename ComplexType::Part;
    Constant<ComplexType> result{
        Scalar<ComplexType>{folded->first, folded->second}};
    if (const auto *re{UnwrapConstantValue<RealType>(x.left())};
        re && re->result().isFromInexactLiteralConversion()) {
      result.result().set_isFromInexactLiteralConversion();
    } else if (const auto *im{UnwrapConstantValue<RealType>(x.right())};
        im && im->result().isFromInexactLiteralConversion()) {
      result.result().set_isFromInexactLiteralConversion();
    }
    return Expr<ComplexType>{std::move(result)};
  }
  return Expr<ComplexType>{std::move(x)};
}

#ifdef _MSC_VER // disable bogus warning about missing definitions
#pragma warning(disable : 4661)
#endif
FOR_EACH_COMPLEX_KIND(template class ExpressionBase, )
template class ExpressionBase<SomeComplex>;
} // namespace language::Compability::evaluate
