//===-- Lower/Support/Utils.h -- utilities ----------------------*- C++ -*-===//
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
// Coding style: https://mlir.toolchain.org/getting_started/DeveloperGuide/
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_COMPABILITY_LOWER_SUPPORT_UTILS_H
#define LANGUAGE_COMPABILITY_LOWER_SUPPORT_UTILS_H

#include "language/Compability/Common/indirection.h"
#include "language/Compability/Lower/IterationSpace.h"
#include "language/Compability/Parser/char-block.h"
#include "language/Compability/Semantics/tools.h"
#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/IR/BuiltinAttributes.h"
#include "toolchain/ADT/SmallSet.h"
#include "toolchain/ADT/StringRef.h"

namespace language::Compability::lower {
using SomeExpr = language::Compability::evaluate::Expr<language::Compability::evaluate::SomeType>;
} // end namespace language::Compability::lower

//===----------------------------------------------------------------------===//
// Small inline helper functions to deal with repetitive, clumsy conversions.
//===----------------------------------------------------------------------===//

/// Convert an F18 CharBlock to an LLVM StringRef.
inline toolchain::StringRef toStringRef(const language::Compability::parser::CharBlock &cb) {
  return {cb.begin(), cb.size()};
}

/// Template helper to remove language::Compability::common::Indirection wrappers.
template <typename A>
const A &removeIndirection(const A &a) {
  return a;
}
template <typename A>
const A &removeIndirection(const language::Compability::common::Indirection<A> &a) {
  return a.value();
}

/// Clone subexpression and wrap it as a generic `language::Compability::evaluate::Expr`.
template <typename A>
static language::Compability::lower::SomeExpr toEvExpr(const A &x) {
  return language::Compability::evaluate::AsGenericExpr(language::Compability::common::Clone(x));
}

template <language::Compability::common::TypeCategory FROM>
static language::Compability::lower::SomeExpr ignoreEvConvert(
    const language::Compability::evaluate::Convert<
        language::Compability::evaluate::Type<language::Compability::common::TypeCategory::Integer, 8>,
        FROM> &x) {
  return toEvExpr(x.left());
}
template <typename A>
static language::Compability::lower::SomeExpr ignoreEvConvert(const A &x) {
  return toEvExpr(x);
}

/// A vector subscript expression may be wrapped with a cast to INTEGER*8.
/// Get rid of it here so the vector can be loaded. Add it back when
/// generating the elemental evaluation (inside the loop nest).
inline language::Compability::lower::SomeExpr
ignoreEvConvert(const language::Compability::evaluate::Expr<language::Compability::evaluate::Type<
                    language::Compability::common::TypeCategory::Integer, 8>> &x) {
  return language::Compability::common::visit(
      [](const auto &v) { return ignoreEvConvert(v); }, x.u);
}

/// Zip two containers of the same size together and flatten the pairs. `flatZip
/// [1;2] [3;4]` yields `[1;3;2;4]`.
template <typename A>
A flatZip(const A &container1, const A &container2) {
  assert(container1.size() == container2.size());
  A result;
  for (auto [e1, e2] : toolchain::zip(container1, container2)) {
    result.emplace_back(e1);
    result.emplace_back(e2);
  }
  return result;
}

namespace language::Compability::lower {
unsigned getHashValue(const language::Compability::lower::SomeExpr *x);
unsigned getHashValue(const language::Compability::lower::ExplicitIterSpace::ArrayBases &x);

bool isEqual(const language::Compability::lower::SomeExpr *x,
             const language::Compability::lower::SomeExpr *y);
bool isEqual(const language::Compability::lower::ExplicitIterSpace::ArrayBases &x,
             const language::Compability::lower::ExplicitIterSpace::ArrayBases &y);

template <typename OpType, typename OperandsStructType>
void privatizeSymbol(
    lower::AbstractConverter &converter, fir::FirOpBuilder &firOpBuilder,
    lower::SymMap &symTable,
    toolchain::SetVector<const semantics::Symbol *> &allPrivatizedSymbols,
    toolchain::SmallSet<const semantics::Symbol *, 16> &mightHaveReadHostSym,
    const semantics::Symbol *symToPrivatize, OperandsStructType *clauseOps);

} // end namespace language::Compability::lower

// DenseMapInfo for pointers to language::Compability::lower::SomeExpr.
namespace toolchain {
template <>
struct DenseMapInfo<const language::Compability::lower::SomeExpr *> {
  static inline const language::Compability::lower::SomeExpr *getEmptyKey() {
    return reinterpret_cast<language::Compability::lower::SomeExpr *>(~0);
  }
  static inline const language::Compability::lower::SomeExpr *getTombstoneKey() {
    return reinterpret_cast<language::Compability::lower::SomeExpr *>(~0 - 1);
  }
  static unsigned getHashValue(const language::Compability::lower::SomeExpr *v) {
    return language::Compability::lower::getHashValue(v);
  }
  static bool isEqual(const language::Compability::lower::SomeExpr *lhs,
                      const language::Compability::lower::SomeExpr *rhs) {
    return language::Compability::lower::isEqual(lhs, rhs);
  }
};
} // namespace toolchain

#endif // FORTRAN_LOWER_SUPPORT_UTILS_H
