//===-- lib/Semantics/assignment.h ------------------------------*- C++ -*-===//
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

#ifndef LANGUAGE_COMPABILITY_SEMANTICS_ASSIGNMENT_H_
#define LANGUAGE_COMPABILITY_SEMANTICS_ASSIGNMENT_H_

#include "language/Compability/Common/indirection.h"
#include "language/Compability/Evaluate/expression.h"
#include "language/Compability/Semantics/semantics.h"

namespace language::Compability::parser {
class ContextualMessages;
struct AssignmentStmt;
struct EndWhereStmt;
struct MaskedElsewhereStmt;
struct PointerAssignmentStmt;
struct WhereConstructStmt;
struct WhereStmt;
} // namespace language::Compability::parser

namespace language::Compability::semantics {

class AssignmentContext;
class Scope;
class Symbol;

// Applies checks from C1594(5-6) on copying pointers in pure subprograms
bool CheckCopyabilityInPureScope(parser::ContextualMessages &,
    const evaluate::Expr<evaluate::SomeType> &, const Scope &);

class AssignmentChecker : public virtual BaseChecker {
public:
  explicit AssignmentChecker(SemanticsContext &);
  ~AssignmentChecker();
  void Enter(const parser::OpenMPDeclareReductionConstruct &x);
  void Enter(const parser::AssignmentStmt &);
  void Enter(const parser::PointerAssignmentStmt &);
  void Enter(const parser::WhereStmt &);
  void Leave(const parser::WhereStmt &);
  void Enter(const parser::WhereConstructStmt &);
  void Leave(const parser::EndWhereStmt &);
  void Enter(const parser::MaskedElsewhereStmt &);
  void Leave(const parser::MaskedElsewhereStmt &);

  SemanticsContext &context();

private:
  common::Indirection<AssignmentContext> context_;
};

} // namespace language::Compability::semantics

extern template class language::Compability::common::Indirection<
    language::Compability::semantics::AssignmentContext>;
#endif // FORTRAN_SEMANTICS_ASSIGNMENT_H_
