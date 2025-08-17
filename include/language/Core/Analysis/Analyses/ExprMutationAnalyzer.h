//===---------- ExprMutationAnalyzer.h ------------------------------------===//
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
#ifndef LANGUAGE_CORE_ANALYSIS_ANALYSES_EXPRMUTATIONANALYZER_H
#define LANGUAGE_CORE_ANALYSIS_ANALYSES_EXPRMUTATIONANALYZER_H

#include "language/Core/ASTMatchers/ASTMatchers.h"
#include "toolchain/ADT/DenseMap.h"
#include <memory>

namespace language::Core {

class FunctionParmMutationAnalyzer;

/// Analyzes whether any mutative operations are applied to an expression within
/// a given statement.
class ExprMutationAnalyzer {
  friend class FunctionParmMutationAnalyzer;

public:
  struct Memoized {
    using ResultMap = toolchain::DenseMap<const Expr *, const Stmt *>;
    using FunctionParaAnalyzerMap =
        toolchain::SmallDenseMap<const FunctionDecl *,
                            std::unique_ptr<FunctionParmMutationAnalyzer>>;

    ResultMap Results;
    ResultMap PointeeResults;
    FunctionParaAnalyzerMap FuncParmAnalyzer;

    void clear() {
      Results.clear();
      PointeeResults.clear();
      FuncParmAnalyzer.clear();
    }
  };
  struct Analyzer {
    Analyzer(const Stmt &Stm, ASTContext &Context, Memoized &Memorized)
        : Stm(Stm), Context(Context), Memorized(Memorized) {}

    const Stmt *findMutation(const Expr *Exp);
    const Stmt *findMutation(const Decl *Dec);

    const Stmt *findPointeeMutation(const Expr *Exp);
    const Stmt *findPointeeMutation(const Decl *Dec);

  private:
    using MutationFinder = const Stmt *(Analyzer::*)(const Expr *);

    const Stmt *findMutationMemoized(const Expr *Exp,
                                     toolchain::ArrayRef<MutationFinder> Finders,
                                     Memoized::ResultMap &MemoizedResults);
    const Stmt *tryEachDeclRef(const Decl *Dec, MutationFinder Finder);

    const Stmt *findExprMutation(ArrayRef<ast_matchers::BoundNodes> Matches);
    const Stmt *findDeclMutation(ArrayRef<ast_matchers::BoundNodes> Matches);
    const Stmt *
    findExprPointeeMutation(ArrayRef<ast_matchers::BoundNodes> Matches);
    const Stmt *
    findDeclPointeeMutation(ArrayRef<ast_matchers::BoundNodes> Matches);

    const Stmt *findDirectMutation(const Expr *Exp);
    const Stmt *findMemberMutation(const Expr *Exp);
    const Stmt *findArrayElementMutation(const Expr *Exp);
    const Stmt *findCastMutation(const Expr *Exp);
    const Stmt *findRangeLoopMutation(const Expr *Exp);
    const Stmt *findReferenceMutation(const Expr *Exp);
    const Stmt *findFunctionArgMutation(const Expr *Exp);

    const Stmt *findPointeeValueMutation(const Expr *Exp);
    const Stmt *findPointeeMemberMutation(const Expr *Exp);
    const Stmt *findPointeeToNonConst(const Expr *Exp);

    const Stmt &Stm;
    ASTContext &Context;
    Memoized &Memorized;
  };

  ExprMutationAnalyzer(const Stmt &Stm, ASTContext &Context)
      : Memorized(), A(Stm, Context, Memorized) {}

  /// check whether stmt is unevaluated. mutation analyzer will ignore the
  /// content in unevaluated stmt.
  static bool isUnevaluated(const Stmt *Stm, ASTContext &Context);

  bool isMutated(const Expr *Exp) { return findMutation(Exp) != nullptr; }
  bool isMutated(const Decl *Dec) { return findMutation(Dec) != nullptr; }
  const Stmt *findMutation(const Expr *Exp) { return A.findMutation(Exp); }
  const Stmt *findMutation(const Decl *Dec) { return A.findMutation(Dec); }

  bool isPointeeMutated(const Expr *Exp) {
    return findPointeeMutation(Exp) != nullptr;
  }
  bool isPointeeMutated(const Decl *Dec) {
    return findPointeeMutation(Dec) != nullptr;
  }
  const Stmt *findPointeeMutation(const Expr *Exp) {
    return A.findPointeeMutation(Exp);
  }
  const Stmt *findPointeeMutation(const Decl *Dec) {
    return A.findPointeeMutation(Dec);
  }

private:
  Memoized Memorized;
  Analyzer A;
};

// A convenient wrapper around ExprMutationAnalyzer for analyzing function
// params.
class FunctionParmMutationAnalyzer {
public:
  static FunctionParmMutationAnalyzer *
  getFunctionParmMutationAnalyzer(const FunctionDecl &Func, ASTContext &Context,
                                  ExprMutationAnalyzer::Memoized &Memorized) {
    auto it = Memorized.FuncParmAnalyzer.find(&Func);
    if (it == Memorized.FuncParmAnalyzer.end()) {
      // Creating a new instance of FunctionParmMutationAnalyzer below may add
      // additional elements to FuncParmAnalyzer. If we did try_emplace before
      // creating a new instance, the returned iterator of try_emplace could be
      // invalidated.
      it =
          Memorized.FuncParmAnalyzer
              .try_emplace(&Func, std::unique_ptr<FunctionParmMutationAnalyzer>(
                                      new FunctionParmMutationAnalyzer(
                                          Func, Context, Memorized)))
              .first;
    }
    return it->getSecond().get();
  }

  bool isMutated(const ParmVarDecl *Parm) {
    return findMutation(Parm) != nullptr;
  }
  const Stmt *findMutation(const ParmVarDecl *Parm);

private:
  ExprMutationAnalyzer::Analyzer BodyAnalyzer;
  toolchain::DenseMap<const ParmVarDecl *, const Stmt *> Results;

  FunctionParmMutationAnalyzer(const FunctionDecl &Func, ASTContext &Context,
                               ExprMutationAnalyzer::Memoized &Memorized);
};

} // namespace language::Core

#endif // LANGUAGE_CORE_ANALYSIS_ANALYSES_EXPRMUTATIONANALYZER_H
