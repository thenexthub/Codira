//===-- Lower/OpenMP/DataSharingProcessor.h ---------------------*- C++ -*-===//
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
#ifndef LANGUAGE_COMPABILITY_LOWER_DATASHARINGPROCESSOR_H
#define LANGUAGE_COMPABILITY_LOWER_DATASHARINGPROCESSOR_H

#include "language/Compability/Lower/AbstractConverter.h"
#include "language/Compability/Lower/OpenMP.h"
#include "language/Compability/Lower/OpenMP/Clauses.h"
#include "language/Compability/Optimizer/Builder/FIRBuilder.h"
#include "language/Compability/Parser/parse-tree.h"
#include "language/Compability/Semantics/symbol.h"
#include "mlir/Dialect/OpenMP/OpenMPDialect.h"
#include <variant>

namespace mlir {
namespace omp {
struct PrivateClauseOps;
} // namespace omp
} // namespace mlir

namespace language::Compability {
namespace lower {
namespace omp {

class DataSharingProcessor {
private:
  /// A symbol visitor that keeps track of the currently active OpenMPConstruct
  /// at any point in time. This is used to track Symbol definition scopes in
  /// order to tell which OMP scope defined vs. references a certain Symbol.
  struct OMPConstructSymbolVisitor {
    OMPConstructSymbolVisitor(semantics::SemanticsContext &ctx)
        : version(ctx.langOptions().OpenMPVersion) {}
    template <typename T>
    bool Pre(const T &) {
      return true;
    }
    template <typename T>
    void Post(const T &) {}

    bool Pre(const parser::OpenMPConstruct &omp) {
      // Skip constructs that may not have privatizations.
      if (isOpenMPPrivatizingConstruct(omp, version))
        constructs.push_back(&omp);
      return true;
    }

    void Post(const parser::OpenMPConstruct &omp) {
      if (isOpenMPPrivatizingConstruct(omp, version))
        constructs.pop_back();
    }

    void Post(const parser::Name &name) {
      auto current = !constructs.empty() ? constructs.back() : ConstructPtr();
      symDefMap.try_emplace(name.symbol, current);
    }

    bool Pre(const parser::DeclarationConstruct &decl) {
      constructs.push_back(&decl);
      return true;
    }

    void Post(const parser::DeclarationConstruct &decl) {
      constructs.pop_back();
    }

    /// Given a \p symbol and an \p eval, returns true if eval is the OMP
    /// construct that defines symbol.
    bool isSymbolDefineBy(const semantics::Symbol *symbol,
                          lower::pft::Evaluation &eval) const;

    // Given a \p symbol, returns true if it is defined by a nested
    // `DeclarationConstruct`.
    bool
    isSymbolDefineByNestedDeclaration(const semantics::Symbol *symbol) const;

  private:
    using ConstructPtr = std::variant<const parser::OpenMPConstruct *,
                                      const parser::DeclarationConstruct *>;
    toolchain::SmallVector<ConstructPtr> constructs;
    toolchain::DenseMap<semantics::Symbol *, ConstructPtr> symDefMap;

    unsigned version;
  };

  mlir::OpBuilder::InsertPoint lastPrivIP;
  toolchain::SmallVector<mlir::Value> loopIVs;
  // Symbols in private, firstprivate, and/or lastprivate clauses.
  toolchain::SetVector<const semantics::Symbol *> explicitlyPrivatizedSymbols;
  toolchain::SetVector<const semantics::Symbol *> defaultSymbols;
  toolchain::SetVector<const semantics::Symbol *> implicitSymbols;
  toolchain::SetVector<const semantics::Symbol *> preDeterminedSymbols;
  toolchain::SetVector<const semantics::Symbol *> allPrivatizedSymbols;

  lower::AbstractConverter &converter;
  semantics::SemanticsContext &semaCtx;
  fir::FirOpBuilder &firOpBuilder;
  omp::List<omp::Clause> clauses;
  lower::pft::Evaluation &eval;
  bool shouldCollectPreDeterminedSymbols;
  bool useDelayedPrivatization;
  toolchain::SmallSet<const semantics::Symbol *, 16> mightHaveReadHostSym;
  lower::SymMap &symTable;
  bool isTargetPrivatization;
  OMPConstructSymbolVisitor visitor;

  bool needBarrier();
  void collectSymbols(semantics::Symbol::Flag flag,
                      toolchain::SetVector<const semantics::Symbol *> &symbols);
  void collectSymbolsInNestedRegions(
      lower::pft::Evaluation &eval, semantics::Symbol::Flag flag,
      toolchain::SetVector<const semantics::Symbol *> &symbolsInNestedRegions);
  void collectOmpObjectListSymbol(
      const omp::ObjectList &objects,
      toolchain::SetVector<const semantics::Symbol *> &symbolSet);
  void collectSymbolsForPrivatization();
  void insertBarrier(mlir::omp::PrivateClauseOps *clauseOps);
  void collectDefaultSymbols();
  void collectImplicitSymbols();
  void collectPreDeterminedSymbols();
  void privatize(mlir::omp::PrivateClauseOps *clauseOps);
  void copyLastPrivatize(mlir::Operation *op);
  void insertLastPrivateCompare(mlir::Operation *op);
  void cloneSymbol(const semantics::Symbol *sym);
  void
  copyFirstPrivateSymbol(const semantics::Symbol *sym,
                         mlir::OpBuilder::InsertPoint *copyAssignIP = nullptr);
  void copyLastPrivateSymbol(const semantics::Symbol *sym,
                             mlir::OpBuilder::InsertPoint *lastPrivIP);
  void insertDeallocs();

  static bool isOpenMPPrivatizingConstruct(const parser::OpenMPConstruct &omp,
                                           unsigned version);
  bool isOpenMPPrivatizingEvaluation(const pft::Evaluation &eval) const;

public:
  DataSharingProcessor(lower::AbstractConverter &converter,
                       semantics::SemanticsContext &semaCtx,
                       const List<Clause> &clauses,
                       lower::pft::Evaluation &eval,
                       bool shouldCollectPreDeterminedSymbols,
                       bool useDelayedPrivatization, lower::SymMap &symTable,
                       bool isTargetPrivatization = false);

  DataSharingProcessor(lower::AbstractConverter &converter,
                       semantics::SemanticsContext &semaCtx,
                       lower::pft::Evaluation &eval,
                       bool useDelayedPrivatization, lower::SymMap &symTable,
                       bool isTargetPrivatization = false);

  // Privatisation is split into two steps.
  // Step1 performs cloning of all privatisation clauses and copying for
  // firstprivates. Step1 is performed at the place where process/processStep1
  // is called. This is usually inside the Operation corresponding to the OpenMP
  // construct, for looping constructs this is just before the Operation. The
  // split into two steps was performed basically to be able to call
  // privatisation for looping constructs before the operation is created since
  // the bounds of the MLIR OpenMP operation can be privatised.
  // Step2 performs the copying for lastprivates and requires knowledge of the
  // MLIR operation to insert the last private update. Step2 adds
  // dealocation code as well.
  void processStep1(mlir::omp::PrivateClauseOps *clauseOps = nullptr);
  void processStep2(mlir::Operation *op, bool isLoop);

  void pushLoopIV(mlir::Value iv) { loopIVs.push_back(iv); }

  const toolchain::SetVector<const semantics::Symbol *> &
  getAllSymbolsToPrivatize() const {
    return allPrivatizedSymbols;
  }

  toolchain::ArrayRef<const semantics::Symbol *> getDelayedPrivSymbols() const {
    return useDelayedPrivatization
               ? allPrivatizedSymbols.getArrayRef()
               : toolchain::ArrayRef<const semantics::Symbol *>();
  }

  void privatizeSymbol(const semantics::Symbol *symToPrivatize,
                       mlir::omp::PrivateClauseOps *clauseOps);
};

} // namespace omp
} // namespace lower
} // namespace language::Compability

#endif // FORTRAN_LOWER_DATASHARINGPROCESSOR_H
