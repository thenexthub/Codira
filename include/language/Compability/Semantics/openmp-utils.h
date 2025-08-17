//===-- lib/Semantics/openmp-utils.h --------------------------------------===//
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
// Common utilities used in OpenMP semantic checks.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_COMPABILITY_SEMANTICS_OPENMP_UTILS_H
#define LANGUAGE_COMPABILITY_SEMANTICS_OPENMP_UTILS_H

#include "language/Compability/Evaluate/type.h"
#include "language/Compability/Parser/char-block.h"
#include "language/Compability/Parser/parse-tree.h"
#include "language/Compability/Semantics/tools.h"

#include "toolchain/ADT/ArrayRef.h"

#include <optional>
#include <string>
#include <type_traits>
#include <utility>

namespace language::Compability::semantics {
class SemanticsContext;
class Symbol;

// Add this namespace to avoid potential conflicts
namespace omp {
template <typename T, typename U = std::remove_const_t<T>> U AsRvalue(T &t) {
  return U(t);
}

template <typename T> T &&AsRvalue(T &&t) { return std::move(t); }

// There is no consistent way to get the source of an ActionStmt, but there
// is "source" in Statement<T>. This structure keeps the ActionStmt with the
// extracted source for further use.
struct SourcedActionStmt {
  const parser::ActionStmt *stmt{nullptr};
  parser::CharBlock source;

  operator bool() const { return stmt != nullptr; }
};

SourcedActionStmt GetActionStmt(const parser::ExecutionPartConstruct *x);
SourcedActionStmt GetActionStmt(const parser::Block &block);

std::string ThisVersion(unsigned version);
std::string TryVersion(unsigned version);

const parser::Designator *GetDesignatorFromObj(const parser::OmpObject &object);
const parser::DataRef *GetDataRefFromObj(const parser::OmpObject &object);
const parser::ArrayElement *GetArrayElementFromObj(
    const parser::OmpObject &object);
const Symbol *GetObjectSymbol(const parser::OmpObject &object);
const Symbol *GetArgumentSymbol(const parser::OmpArgument &argument);
std::optional<parser::CharBlock> GetObjectSource(
    const parser::OmpObject &object);

bool IsCommonBlock(const Symbol &sym);
bool IsExtendedListItem(const Symbol &sym);
bool IsVariableListItem(const Symbol &sym);
bool IsVarOrFunctionRef(const MaybeExpr &expr);

bool IsMapEnteringType(parser::OmpMapType::Value type);
bool IsMapExitingType(parser::OmpMapType::Value type);

std::optional<SomeExpr> GetEvaluateExpr(const parser::Expr &parserExpr);
std::optional<evaluate::DynamicType> GetDynamicType(
    const parser::Expr &parserExpr);

std::optional<bool> IsContiguous(
    SemanticsContext &semaCtx, const parser::OmpObject &object);

std::vector<SomeExpr> GetAllDesignators(const SomeExpr &expr);
const SomeExpr *HasStorageOverlap(
    const SomeExpr &base, toolchain::ArrayRef<SomeExpr> exprs);
bool IsAssignment(const parser::ActionStmt *x);
bool IsPointerAssignment(const evaluate::Assignment &x);
const parser::Block &GetInnermostExecPart(const parser::Block &block);
} // namespace omp
} // namespace language::Compability::semantics

#endif // FORTRAN_SEMANTICS_OPENMP_UTILS_H
