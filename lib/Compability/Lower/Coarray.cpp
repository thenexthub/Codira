//===-- Coarray.cpp -------------------------------------------------------===//
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
///
/// Implementation of the lowering of image related constructs and expressions.
/// Fortran images can form teams, communicate via coarrays, etc.
///
//===----------------------------------------------------------------------===//

#include "language/Compability/Lower/Coarray.h"
#include "language/Compability/Lower/AbstractConverter.h"
#include "language/Compability/Lower/SymbolMap.h"
#include "language/Compability/Optimizer/Builder/FIRBuilder.h"
#include "language/Compability/Optimizer/Builder/Todo.h"
#include "language/Compability/Parser/parse-tree.h"
#include "language/Compability/Semantics/expression.h"

//===----------------------------------------------------------------------===//
// TEAM statements and constructs
//===----------------------------------------------------------------------===//

void language::Compability::lower::genChangeTeamConstruct(
    language::Compability::lower::AbstractConverter &converter,
    language::Compability::lower::pft::Evaluation &,
    const language::Compability::parser::ChangeTeamConstruct &) {
  TODO(converter.getCurrentLocation(), "coarray: CHANGE TEAM construct");
}

void language::Compability::lower::genChangeTeamStmt(
    language::Compability::lower::AbstractConverter &converter,
    language::Compability::lower::pft::Evaluation &,
    const language::Compability::parser::ChangeTeamStmt &) {
  TODO(converter.getCurrentLocation(), "coarray: CHANGE TEAM statement");
}

void language::Compability::lower::genEndChangeTeamStmt(
    language::Compability::lower::AbstractConverter &converter,
    language::Compability::lower::pft::Evaluation &,
    const language::Compability::parser::EndChangeTeamStmt &) {
  TODO(converter.getCurrentLocation(), "coarray: END CHANGE TEAM statement");
}

void language::Compability::lower::genFormTeamStatement(
    language::Compability::lower::AbstractConverter &converter,
    language::Compability::lower::pft::Evaluation &, const language::Compability::parser::FormTeamStmt &) {
  TODO(converter.getCurrentLocation(), "coarray: FORM TEAM statement");
}

//===----------------------------------------------------------------------===//
// COARRAY expressions
//===----------------------------------------------------------------------===//

fir::ExtendedValue language::Compability::lower::CoarrayExprHelper::genAddr(
    const language::Compability::evaluate::CoarrayRef &expr) {
  (void)symMap;
  TODO(converter.getCurrentLocation(), "co-array address");
}

fir::ExtendedValue language::Compability::lower::CoarrayExprHelper::genValue(
    const language::Compability::evaluate::CoarrayRef &expr) {
  TODO(converter.getCurrentLocation(), "co-array value");
}
