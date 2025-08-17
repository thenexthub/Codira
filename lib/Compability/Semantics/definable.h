//===-- lib/Semantics/definable.h -------------------------------*- C++ -*-===//
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

#ifndef LANGUAGE_COMPABILITY_SEMANTICS_DEFINABLE_H_
#define LANGUAGE_COMPABILITY_SEMANTICS_DEFINABLE_H_

// Utilities for checking the definability of variables and pointers in context,
// including checks for attempted definitions in PURE subprograms.
// Fortran 2018 C1101, C1158, C1594, &c.

#include "language/Compability/Common/enum-set.h"
#include "language/Compability/Common/idioms.h"
#include "language/Compability/Evaluate/expression.h"
#include "language/Compability/Parser/char-block.h"
#include "language/Compability/Parser/message.h"
#include <optional>

namespace language::Compability::semantics {

class Symbol;
class Scope;

ENUM_CLASS(DefinabilityFlag,
    VectorSubscriptIsOk, // a vector subscript may appear (i.e., assignment)
    DuplicatesAreOk, // vector subscript may have duplicates
    PointerDefinition, // a pointer is being defined, not its target
    AcceptAllocatable, // treat allocatable as if it were a pointer
    SourcedAllocation, // ALLOCATE(a,SOURCE=)
    PolymorphicOkInPure, // don't check for polymorphic type in pure subprogram
    DoNotNoteDefinition, // context does not imply definition
    AllowEventLockOrNotifyType, PotentialDeallocation)

using DefinabilityFlags =
    common::EnumSet<DefinabilityFlag, DefinabilityFlag_enumSize>;

// Tests a symbol or LHS variable or pointer for definability in a given scope.
// When the entity is not definable, returns a Message suitable for attachment
// to an error or warning message (as a "because: addendum) to explain why the
// entity cannot be defined.
// When the entity can be defined in that context, returns std::nullopt.
std::optional<parser::Message> WhyNotDefinable(
    parser::CharBlock, const Scope &, DefinabilityFlags, const Symbol &);
std::optional<parser::Message> WhyNotDefinable(parser::CharBlock, const Scope &,
    DefinabilityFlags, const evaluate::Expr<evaluate::SomeType> &);

// If a symbol would not be definable in a pure scope, or not be usable as the
// target of a pointer assignment in a pure scope, return a constant string
// describing why.
const char *WhyBaseObjectIsSuspicious(const Symbol &, const Scope &);

} // namespace language::Compability::semantics
#endif // FORTRAN_SEMANTICS_DEFINABLE_H_
