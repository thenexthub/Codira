//===-- Lower/OpenACC.h -- lower OpenACC directives -------------*- C++ -*-===//
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

#ifndef LANGUAGE_COMPABILITY_LOWER_OPENACC_H
#define LANGUAGE_COMPABILITY_LOWER_OPENACC_H

#include "mlir/Dialect/OpenACC/OpenACC.h"

namespace toolchain {
template <typename T, unsigned N>
class SmallVector;
class StringRef;
} // namespace toolchain

namespace mlir {
namespace func {
class FuncOp;
} // namespace func
class Location;
class Type;
class ModuleOp;
class OpBuilder;
class Value;
} // namespace mlir

namespace fir {
class FirOpBuilder;
} // namespace fir

namespace language::Compability {
namespace evaluate {
struct ProcedureDesignator;
} // namespace evaluate

namespace parser {
struct AccClauseList;
struct DoConstruct;
struct OpenACCConstruct;
struct OpenACCDeclarativeConstruct;
struct OpenACCRoutineConstruct;
} // namespace parser

namespace semantics {
class OpenACCRoutineInfo;
class SemanticsContext;
class Symbol;
} // namespace semantics

namespace lower {

class AbstractConverter;
class StatementContext;
class SymMap;

namespace pft {
struct Evaluation;
} // namespace pft

static constexpr toolchain::StringRef declarePostAllocSuffix =
    "_acc_declare_update_desc_post_alloc";
static constexpr toolchain::StringRef declarePreDeallocSuffix =
    "_acc_declare_update_desc_pre_dealloc";
static constexpr toolchain::StringRef declarePostDeallocSuffix =
    "_acc_declare_update_desc_post_dealloc";

static constexpr toolchain::StringRef privatizationRecipePrefix = "privatization";

mlir::Value genOpenACCConstruct(AbstractConverter &,
                                language::Compability::semantics::SemanticsContext &,
                                pft::Evaluation &,
                                const parser::OpenACCConstruct &);
void genOpenACCDeclarativeConstruct(
    AbstractConverter &, language::Compability::semantics::SemanticsContext &,
    StatementContext &, const parser::OpenACCDeclarativeConstruct &);
void genOpenACCRoutineConstruct(
    AbstractConverter &, mlir::ModuleOp, mlir::func::FuncOp,
    const std::vector<language::Compability::semantics::OpenACCRoutineInfo> &);

/// Get a acc.private.recipe op for the given type or create it if it does not
/// exist yet.
mlir::acc::PrivateRecipeOp createOrGetPrivateRecipe(fir::FirOpBuilder &,
                                                    toolchain::StringRef,
                                                    mlir::Location, mlir::Type);

/// Get a acc.reduction.recipe op for the given type or create it if it does not
/// exist yet.
mlir::acc::ReductionRecipeOp
createOrGetReductionRecipe(fir::FirOpBuilder &, toolchain::StringRef, mlir::Location,
                           mlir::Type, mlir::acc::ReductionOperator,
                           toolchain::SmallVector<mlir::Value> &);

/// Get a acc.firstprivate.recipe op for the given type or create it if it does
/// not exist yet.
mlir::acc::FirstprivateRecipeOp
createOrGetFirstprivateRecipe(fir::FirOpBuilder &, toolchain::StringRef,
                              mlir::Location, mlir::Type,
                              toolchain::SmallVector<mlir::Value> &);

void attachDeclarePostAllocAction(AbstractConverter &, fir::FirOpBuilder &,
                                  const language::Compability::semantics::Symbol &);
void attachDeclarePreDeallocAction(AbstractConverter &, fir::FirOpBuilder &,
                                   mlir::Value beginOpValue,
                                   const language::Compability::semantics::Symbol &);
void attachDeclarePostDeallocAction(AbstractConverter &, fir::FirOpBuilder &,
                                    const language::Compability::semantics::Symbol &);

void genOpenACCTerminator(fir::FirOpBuilder &, mlir::Operation *,
                          mlir::Location);

/// Used to obtain the number of contained loops to look for
/// since this is dependent on number of tile operands and collapse
/// clause.
uint64_t getLoopCountForCollapseAndTile(const language::Compability::parser::AccClauseList &);

/// Checks whether the current insertion point is inside OpenACC loop.
bool isInOpenACCLoop(fir::FirOpBuilder &);

/// Checks whether the current insertion point is inside OpenACC compute
/// construct.
bool isInsideOpenACCComputeConstruct(fir::FirOpBuilder &);

void setInsertionPointAfterOpenACCLoopIfInside(fir::FirOpBuilder &);

void genEarlyReturnInOpenACCLoop(fir::FirOpBuilder &, mlir::Location);

/// Generates an OpenACC loop from a do construct in order to
/// properly capture the loop bounds, parallelism determination mode,
/// and to privatize the loop variables.
/// When the conversion is rejected, nullptr is returned.
mlir::Operation *genOpenACCLoopFromDoConstruct(
    AbstractConverter &converter,
    language::Compability::semantics::SemanticsContext &semanticsContext,
    language::Compability::lower::SymMap &localSymbols,
    const language::Compability::parser::DoConstruct &doConstruct, pft::Evaluation &eval);

} // namespace lower
} // namespace language::Compability

#endif // FORTRAN_LOWER_OPENACC_H
