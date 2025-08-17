//===-- lib/Semantics/program-tree.h ----------------------------*- C++ -*-===//
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

#ifndef LANGUAGE_COMPABILITY_SEMANTICS_PROGRAM_TREE_H_
#define LANGUAGE_COMPABILITY_SEMANTICS_PROGRAM_TREE_H_

#include "symbol.h"
#include "language/Compability/Parser/parse-tree.h"
#include <list>
#include <variant>

// A ProgramTree represents a tree of program units and their contained
// subprograms. The root nodes represent: main program, function, subroutine,
// module subprogram, module, or submodule.
// Each node of the tree consists of:
//   - the statement that introduces the program unit
//   - the specification part
//   - the execution part if applicable (not for module or submodule)
//   - a child node for each contained subprogram

namespace language::Compability::semantics {

class Scope;
class SemanticsContext;

class ProgramTree {
public:
  using EntryStmtList = std::list<common::Reference<const parser::EntryStmt>>;
  using GenericSpecList =
      std::list<common::Reference<const parser::GenericSpec>>;

  // Build the ProgramTree rooted at one of these program units.
  static ProgramTree &Build(const parser::ProgramUnit &, SemanticsContext &);
  static std::optional<ProgramTree> Build(
      const parser::MainProgram &, SemanticsContext &);
  static std::optional<ProgramTree> Build(
      const parser::FunctionSubprogram &, SemanticsContext &);
  static std::optional<ProgramTree> Build(
      const parser::SubroutineSubprogram &, SemanticsContext &);
  static std::optional<ProgramTree> Build(
      const parser::SeparateModuleSubprogram &, SemanticsContext &);
  static std::optional<ProgramTree> Build(
      const parser::Module &, SemanticsContext &);
  static std::optional<ProgramTree> Build(
      const parser::Submodule &, SemanticsContext &);
  static std::optional<ProgramTree> Build(
      const parser::BlockData &, SemanticsContext &);
  static std::optional<ProgramTree> Build(
      const parser::CompilerDirective &, SemanticsContext &);
  static std::optional<ProgramTree> Build(
      const parser::OpenACCRoutineConstruct &, SemanticsContext &);

  ENUM_CLASS(Kind, // kind of node
      Program, Function, Subroutine, MpSubprogram, Module, Submodule, BlockData)
  using Stmt = std::variant< // the statement that introduces the program unit
      const parser::Statement<parser::ProgramStmt> *,
      const parser::Statement<parser::FunctionStmt> *,
      const parser::Statement<parser::SubroutineStmt> *,
      const parser::Statement<parser::MpSubprogramStmt> *,
      const parser::Statement<parser::ModuleStmt> *,
      const parser::Statement<parser::SubmoduleStmt> *,
      const parser::Statement<parser::BlockDataStmt> *>;

  ProgramTree(const parser::Name &name, const parser::SpecificationPart &spec,
      const parser::ExecutionPart *exec = nullptr)
      : name_{name}, spec_{spec}, exec_{exec} {}

  const parser::Name &name() const { return name_; }
  Kind GetKind() const;
  const Stmt &stmt() const { return stmt_; }
  bool isSpecificationPartResolved() const {
    return isSpecificationPartResolved_;
  }
  void set_isSpecificationPartResolved(bool yes = true) {
    isSpecificationPartResolved_ = yes;
  }
  const parser::ParentIdentifier &GetParentId() const; // only for Submodule
  const parser::SpecificationPart &spec() const { return spec_; }
  const parser::ExecutionPart *exec() const { return exec_; }
  std::list<ProgramTree> &children() { return children_; }
  const std::list<ProgramTree> &children() const { return children_; }
  const EntryStmtList &entryStmts() const { return entryStmts_; }
  const GenericSpecList &genericSpecs() const { return genericSpecs_; }

  Symbol::Flag GetSubpFlag() const;
  bool IsModule() const; // Module or Submodule
  bool HasModulePrefix() const; // in function or subroutine stmt
  Scope *scope() const { return scope_; }
  void set_scope(Scope &);
  const parser::LanguageBindingSpec *bindingSpec() const {
    return bindingSpec_;
  }
  ProgramTree &set_bindingSpec(const parser::LanguageBindingSpec *spec) {
    bindingSpec_ = spec;
    return *this;
  }
  void AddChild(ProgramTree &&);
  void AddEntry(const parser::EntryStmt &);
  void AddGeneric(const parser::GenericSpec &);

  template <typename T>
  ProgramTree &set_stmt(const parser::Statement<T> &stmt) {
    stmt_ = &stmt;
    return *this;
  }
  template <typename T>
  ProgramTree &set_endStmt(const parser::Statement<T> &stmt) {
    endStmt_ = &stmt.source;
    return *this;
  }

private:
  const parser::Name &name_;
  Stmt stmt_{
      static_cast<const parser::Statement<parser::ProgramStmt> *>(nullptr)};
  const parser::SpecificationPart &spec_;
  const parser::ExecutionPart *exec_{nullptr};
  std::list<ProgramTree> children_;
  EntryStmtList entryStmts_;
  GenericSpecList genericSpecs_;
  Scope *scope_{nullptr};
  const parser::CharBlock *endStmt_{nullptr};
  bool isSpecificationPartResolved_{false};
  const parser::LanguageBindingSpec *bindingSpec_{nullptr};
};

} // namespace language::Compability::semantics
#endif // FORTRAN_SEMANTICS_PROGRAM_TREE_H_
