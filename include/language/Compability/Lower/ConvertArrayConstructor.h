//===-- ConvertArrayConstructor.h -- Array constructor lowering -*- C++ -*-===//
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
/// Implements the conversion from evaluate::ArrayConstructor to HLFIR.
///
//===----------------------------------------------------------------------===//
#ifndef LANGUAGE_COMPABILITY_LOWER_CONVERTARRAYCONSTRUCTOR_H
#define LANGUAGE_COMPABILITY_LOWER_CONVERTARRAYCONSTRUCTOR_H

#include "language/Compability/Evaluate/type.h"
#include "language/Compability/Optimizer/Builder/HLFIRTools.h"

namespace language::Compability::evaluate {
template <typename T>
class ArrayConstructor;
}

namespace language::Compability::lower {
class AbstractConverter;
class SymMap;
class StatementContext;

/// Class to lower evaluate::ArrayConstructor<T> to hlfir::EntityWithAttributes.
template <typename T>
class ArrayConstructorBuilder {
public:
  static hlfir::EntityWithAttributes
  gen(mlir::Location loc, language::Compability::lower::AbstractConverter &converter,
      const language::Compability::evaluate::ArrayConstructor<T> &expr,
      language::Compability::lower::SymMap &symMap,
      language::Compability::lower::StatementContext &stmtCtx);
};
using namespace evaluate;
FOR_EACH_SPECIFIC_TYPE(extern template class ArrayConstructorBuilder, )
} // namespace language::Compability::lower

#endif // FORTRAN_LOWER_CONVERTARRAYCONSTRUCTOR_H
