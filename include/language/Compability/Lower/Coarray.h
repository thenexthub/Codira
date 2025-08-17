//===-- Lower/Coarray.h -- image related lowering ---------------*- C++ -*-===//
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

#ifndef LANGUAGE_COMPABILITY_LOWER_COARRAY_H
#define LANGUAGE_COMPABILITY_LOWER_COARRAY_H

#include "language/Compability/Lower/AbstractConverter.h"
#include "language/Compability/Optimizer/Builder/BoxValue.h"

namespace language::Compability {

namespace parser {
struct ChangeTeamConstruct;
struct ChangeTeamStmt;
struct EndChangeTeamStmt;
struct FormTeamStmt;
} // namespace parser

namespace evaluate {
class CoarrayRef;
} // namespace evaluate

namespace lower {

class SymMap;

namespace pft {
struct Evaluation;
} // namespace pft

//===----------------------------------------------------------------------===//
// TEAM constructs
//===----------------------------------------------------------------------===//

void genChangeTeamConstruct(AbstractConverter &, pft::Evaluation &eval,
                            const parser::ChangeTeamConstruct &);
void genChangeTeamStmt(AbstractConverter &, pft::Evaluation &eval,
                       const parser::ChangeTeamStmt &);
void genEndChangeTeamStmt(AbstractConverter &, pft::Evaluation &eval,
                          const parser::EndChangeTeamStmt &);
void genFormTeamStatement(AbstractConverter &, pft::Evaluation &eval,
                          const parser::FormTeamStmt &);

//===----------------------------------------------------------------------===//
// COARRAY expressions
//===----------------------------------------------------------------------===//

/// Coarray expression lowering helper. A coarray expression is expected to be
/// lowered into runtime support calls. For example, expressions may use a
/// message-passing runtime to access another image's data.
class CoarrayExprHelper {
public:
  explicit CoarrayExprHelper(AbstractConverter &converter, mlir::Location loc,
                             SymMap &syms)
      : converter{converter}, symMap{syms}, loc{loc} {}
  CoarrayExprHelper(const CoarrayExprHelper &) = delete;

  /// Generate the address of a co-array expression.
  fir::ExtendedValue genAddr(const evaluate::CoarrayRef &expr);

  /// Generate the value of a co-array expression.
  fir::ExtendedValue genValue(const evaluate::CoarrayRef &expr);

private:
  AbstractConverter &converter;
  SymMap &symMap;
  mlir::Location loc;
};

} // namespace lower
} // namespace language::Compability

#endif // FORTRAN_LOWER_COARRAY_H
