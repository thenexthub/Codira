//===-- ConvertConstant.h -- lowering of constants --------------*- C++ -*-===//
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
///
/// Implements the conversion from evaluate::Constant to FIR.
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_COMPABILITY_LOWER_CONVERTCONSTANT_H
#define LANGUAGE_COMPABILITY_LOWER_CONVERTCONSTANT_H

#include "language/Compability/Evaluate/constant.h"
#include "language/Compability/Lower/Support/Utils.h"
#include "language/Compability/Optimizer/Builder/BoxValue.h"
#include "language/Compability/Optimizer/Builder/FIRBuilder.h"

namespace language::Compability::lower {
class AbstractConverter;

/// Class to lower evaluate::Constant to fir::ExtendedValue.
template <typename T>
class ConstantBuilder {
public:
  /// Lower \p constant into a fir::ExtendedValue.
  /// If \p outlineBigConstantsInReadOnlyMemory is set, character, derived
  /// type, and array constants will be lowered into read only memory
  /// fir.global, and the resulting fir::ExtendedValue will contain the address
  /// of the fir.global. This option should not be set if the constant is being
  /// lowered while the builder is already in a fir.global body because
  /// fir.global initialization body cannot contain code manipulating memory
  /// (e.g.  fir.load/fir.store...).
  static fir::ExtendedValue gen(language::Compability::lower::AbstractConverter &converter,
                                mlir::Location loc,
                                const evaluate::Constant<T> &constant,
                                bool outlineBigConstantsInReadOnlyMemory);
};
using namespace evaluate;
FOR_EACH_SPECIFIC_TYPE(extern template class ConstantBuilder, )

template <typename T>
fir::ExtendedValue convertConstant(language::Compability::lower::AbstractConverter &converter,
                                   mlir::Location loc,
                                   const evaluate::Constant<T> &constant,
                                   bool outlineBigConstantsInReadOnlyMemory) {
  return ConstantBuilder<T>::gen(converter, loc, constant,
                                 outlineBigConstantsInReadOnlyMemory);
}

/// Create a fir.global array with a dense attribute containing the value of
/// \p initExpr.
/// Using a dense attribute allows faster MLIR compilation times compared to
/// creating an initialization body for the initial value. However, a dense
/// attribute can only be created if initExpr is a non-empty rank 1 numerical or
/// logical Constant<T>. Otherwise, the value returned will be null.
fir::GlobalOp tryCreatingDenseGlobal(fir::FirOpBuilder &builder,
                                     mlir::Location loc, mlir::Type symTy,
                                     toolchain::StringRef globalName,
                                     mlir::StringAttr linkage, bool isConst,
                                     const language::Compability::lower::SomeExpr &initExpr,
                                     cuf::DataAttributeAttr dataAttr = {});

/// Lower a StructureConstructor that must be lowered in read only data although
/// it may not be wrapped into a Constant<T> (this may be the case for derived
/// type descriptor compiler generated data that is not fully compliant with
/// Fortran constant expression but can and must still be lowered into read only
/// memory).
fir::ExtendedValue
genInlinedStructureCtorLit(language::Compability::lower::AbstractConverter &converter,
                           mlir::Location loc,
                           const language::Compability::evaluate::StructureConstructor &ctor);

} // namespace language::Compability::lower

#endif // FORTRAN_LOWER_CONVERTCONSTANT_H
