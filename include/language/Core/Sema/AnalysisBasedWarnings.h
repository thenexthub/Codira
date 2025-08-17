//=- AnalysisBasedWarnings.h - Sema warnings based on libAnalysis -*- C++ -*-=//
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
// This file defines AnalysisBasedWarnings, a worker object used by Sema
// that issues warnings based on dataflow-analysis.
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_SEMA_ANALYSISBASEDWARNINGS_H
#define LANGUAGE_CORE_SEMA_ANALYSISBASEDWARNINGS_H

#include "language/Core/AST/Decl.h"
#include "toolchain/ADT/DenseMap.h"
#include <memory>

namespace language::Core {

class Decl;
class FunctionDecl;
class QualType;
class Sema;
namespace sema {
  class FunctionScopeInfo;
  class SemaPPCallbacks;
}

namespace sema {

class AnalysisBasedWarnings {
public:
  class Policy {
    friend class AnalysisBasedWarnings;
    friend class SemaPPCallbacks;
    // The warnings to run.
    LLVM_PREFERRED_TYPE(bool)
    unsigned enableCheckFallThrough : 1;
    LLVM_PREFERRED_TYPE(bool)
    unsigned enableCheckUnreachable : 1;
    LLVM_PREFERRED_TYPE(bool)
    unsigned enableThreadSafetyAnalysis : 1;
    LLVM_PREFERRED_TYPE(bool)
    unsigned enableConsumedAnalysis : 1;
  public:
    Policy();
    void disableCheckFallThrough() { enableCheckFallThrough = 0; }
  };

private:
  Sema &S;

  class InterProceduralData;
  std::unique_ptr<InterProceduralData> IPData;

  enum VisitFlag { NotVisited = 0, Visited = 1, Pending = 2 };
  toolchain::DenseMap<const FunctionDecl*, VisitFlag> VisitedFD;

  Policy PolicyOverrides;
  void clearOverrides();

  /// \name Statistics
  /// @{

  /// Number of function CFGs built and analyzed.
  unsigned NumFunctionsAnalyzed;

  /// Number of functions for which the CFG could not be successfully
  /// built.
  unsigned NumFunctionsWithBadCFGs;

  /// Total number of blocks across all CFGs.
  unsigned NumCFGBlocks;

  /// Largest number of CFG blocks for a single function analyzed.
  unsigned MaxCFGBlocksPerFunction;

  /// Total number of CFGs with variables analyzed for uninitialized
  /// uses.
  unsigned NumUninitAnalysisFunctions;

  /// Total number of variables analyzed for uninitialized uses.
  unsigned NumUninitAnalysisVariables;

  /// Max number of variables analyzed for uninitialized uses in a single
  /// function.
  unsigned MaxUninitAnalysisVariablesPerFunction;

  /// Total number of block visits during uninitialized use analysis.
  unsigned NumUninitAnalysisBlockVisits;

  /// Max number of block visits during uninitialized use analysis of
  /// a single function.
  unsigned MaxUninitAnalysisBlockVisitsPerFunction;

  /// @}

public:
  AnalysisBasedWarnings(Sema &s);
  ~AnalysisBasedWarnings();

  void IssueWarnings(Policy P, FunctionScopeInfo *fscope,
                     const Decl *D, QualType BlockType);

  // Issue warnings that require whole-translation-unit analysis.
  void IssueWarnings(TranslationUnitDecl *D);

  // Gets the default policy which is in effect at the given source location.
  Policy getPolicyInEffectAt(SourceLocation Loc);

  // Get the policies we may want to override due to things like #pragma clang
  // diagnostic handling. If a caller sets any of these policies to true, that
  // will override the policy used to issue warnings.
  Policy &getPolicyOverrides() { return PolicyOverrides; }

  void PrintStats() const;
};

} // namespace sema
} // namespace language::Core

#endif
