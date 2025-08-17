//===-- Lower/OpenMP.h -- lower Open MP directives --------------*- C++ -*-===//
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

#ifndef LANGUAGE_COMPABILITY_LOWER_OPENMP_H
#define LANGUAGE_COMPABILITY_LOWER_OPENMP_H

#include "toolchain/ADT/SmallVector.h"

#include <cinttypes>
#include <utility>

namespace mlir {
class Operation;
class Location;
namespace omp {
enum class DeclareTargetDeviceType : uint32_t;
enum class DeclareTargetCaptureClause : uint32_t;
} // namespace omp
} // namespace mlir

namespace fir {
class FirOpBuilder;
} // namespace fir

namespace language::Compability {
namespace parser {
struct OpenMPConstruct;
struct OpenMPDeclarativeConstruct;
struct OmpEndLoopDirective;
struct OmpClauseList;
} // namespace parser

namespace semantics {
class Symbol;
class SemanticsContext;
} // namespace semantics

namespace lower {

class AbstractConverter;
class SymMap;

namespace pft {
struct Evaluation;
struct Variable;
} // namespace pft

struct OMPDeferredDeclareTargetInfo {
  mlir::omp::DeclareTargetCaptureClause declareTargetCaptureClause;
  mlir::omp::DeclareTargetDeviceType declareTargetDeviceType;
  bool automap = false;
  const language::Compability::semantics::Symbol &sym;
};

// Generate the OpenMP terminator for Operation at Location.
mlir::Operation *genOpenMPTerminator(fir::FirOpBuilder &, mlir::Operation *,
                                     mlir::Location);

void genOpenMPConstruct(AbstractConverter &, language::Compability::lower::SymMap &,
                        semantics::SemanticsContext &, pft::Evaluation &,
                        const parser::OpenMPConstruct &);
void genOpenMPDeclarativeConstruct(AbstractConverter &,
                                   language::Compability::lower::SymMap &,
                                   semantics::SemanticsContext &,
                                   pft::Evaluation &,
                                   const parser::OpenMPDeclarativeConstruct &);
/// Symbols in OpenMP code can have flags (e.g. threadprivate directive)
/// that require additional handling when lowering the corresponding
/// variable. Perform such handling according to the flags on the symbol.
/// The variable \p var is required to have a `Symbol`.
void genOpenMPSymbolProperties(AbstractConverter &converter,
                               const pft::Variable &var);

int64_t getCollapseValue(const language::Compability::parser::OmpClauseList &clauseList);
void genThreadprivateOp(AbstractConverter &, const pft::Variable &);
void genDeclareTargetIntGlobal(AbstractConverter &, const pft::Variable &);
bool isOpenMPTargetConstruct(const parser::OpenMPConstruct &);
bool isOpenMPDeviceDeclareTarget(language::Compability::lower::AbstractConverter &,
                                 language::Compability::semantics::SemanticsContext &,
                                 language::Compability::lower::pft::Evaluation &,
                                 const parser::OpenMPDeclarativeConstruct &);
void gatherOpenMPDeferredDeclareTargets(
    language::Compability::lower::AbstractConverter &, language::Compability::semantics::SemanticsContext &,
    language::Compability::lower::pft::Evaluation &,
    const parser::OpenMPDeclarativeConstruct &,
    toolchain::SmallVectorImpl<OMPDeferredDeclareTargetInfo> &);
bool markOpenMPDeferredDeclareTargetFunctions(
    mlir::Operation *, toolchain::SmallVectorImpl<OMPDeferredDeclareTargetInfo> &,
    AbstractConverter &);
void genOpenMPRequires(mlir::Operation *, const language::Compability::semantics::Symbol *);

} // namespace lower
} // namespace language::Compability

#endif // FORTRAN_LOWER_OPENMP_H
