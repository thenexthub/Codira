//===-- lib/Semantics/check-do-forall.h -------------------------*- C++ -*-===//
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

#ifndef LANGUAGE_COMPABILITY_SEMANTICS_CHECK_DO_FORALL_H_
#define LANGUAGE_COMPABILITY_SEMANTICS_CHECK_DO_FORALL_H_

#include "language/Compability/Common/idioms.h"
#include "language/Compability/Semantics/semantics.h"

namespace language::Compability::parser {
struct AssignmentStmt;
struct CallStmt;
struct ConnectSpec;
struct CycleStmt;
struct DoConstruct;
struct ExitStmt;
struct Expr;
struct ForallAssignmentStmt;
struct ForallConstruct;
struct ForallStmt;
struct InquireSpec;
struct IoControlSpec;
struct OutputImpliedDo;
struct StatVariable;
} // namespace language::Compability::parser

namespace language::Compability::semantics {

// To specify different statement types used in semantic checking.
ENUM_CLASS(StmtType, CYCLE, EXIT)

// Perform semantic checks on DO and FORALL constructs and statements.
class DoForallChecker : public virtual BaseChecker {
public:
  explicit DoForallChecker(SemanticsContext &context) : context_{context} {}
  void Leave(const parser::AssignmentStmt &);
  void Leave(const parser::CallStmt &);
  void Leave(const parser::ConnectSpec &);
  void Enter(const parser::CycleStmt &);
  void Enter(const parser::DoConstruct &);
  void Leave(const parser::DoConstruct &);
  void Enter(const parser::ForallConstruct &);
  void Leave(const parser::ForallConstruct &);
  void Enter(const parser::ForallStmt &);
  void Leave(const parser::ForallStmt &);
  void Leave(const parser::ForallAssignmentStmt &s);
  void Enter(const parser::ExitStmt &);
  void Enter(const parser::Expr &);
  void Leave(const parser::Expr &);
  void Leave(const parser::InquireSpec &);
  void Leave(const parser::IoControlSpec &);
  void Leave(const parser::OutputImpliedDo &);
  void Leave(const parser::StatVariable &);

private:
  SemanticsContext &context_;
  int exprDepth_{0};
  std::list<SemanticsContext::IndexVarKind> nestedWithinConcurrent_;

  void SayBadLeave(
      StmtType, const char *enclosingStmt, const ConstructNode &) const;
  void CheckDoConcurrentExit(StmtType, const ConstructNode &) const;
  void CheckForBadLeave(StmtType, const ConstructNode &) const;
  void CheckNesting(StmtType, const parser::Name *) const;
};
} // namespace language::Compability::semantics
#endif
