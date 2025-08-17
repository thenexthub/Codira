//===-- lib/Semantics/pointer-assignment.h --------------------------------===//
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

#ifndef LANGUAGE_COMPABILITY_SEMANTICS_POINTER_ASSIGNMENT_H_
#define LANGUAGE_COMPABILITY_SEMANTICS_POINTER_ASSIGNMENT_H_

#include "language/Compability/Evaluate/expression.h"
#include "language/Compability/Parser/char-block.h"
#include "language/Compability/Semantics/type.h"
#include <string>

namespace language::Compability::evaluate::characteristics {
struct DummyDataObject;
}

namespace language::Compability::semantics {

class SemanticsContext;
class Symbol;

bool CheckPointerAssignment(
    SemanticsContext &, const evaluate::Assignment &, const Scope &);
bool CheckPointerAssignment(SemanticsContext &, const SomeExpr &lhs,
    const SomeExpr &rhs, const Scope &, bool isBoundsRemapping,
    bool isAssumedRank);
bool CheckPointerAssignment(SemanticsContext &, parser::CharBlock source,
    const std::string &description,
    const evaluate::characteristics::DummyDataObject &, const SomeExpr &rhs,
    const Scope &, bool isAssumedRank, bool IsPointerActualArgument);

bool CheckStructConstructorPointerComponent(
    SemanticsContext &, const Symbol &lhs, const SomeExpr &rhs, const Scope &);

// Checks whether an expression is a valid static initializer for a
// particular pointer designator.
bool CheckInitialDataPointerTarget(SemanticsContext &, const SomeExpr &pointer,
    const SomeExpr &init, const Scope &);

} // namespace language::Compability::semantics

#endif // FORTRAN_SEMANTICS_POINTER_ASSIGNMENT_H_
