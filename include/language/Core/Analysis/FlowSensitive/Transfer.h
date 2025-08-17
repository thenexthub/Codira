//===-- Transfer.h ----------------------------------------------*- C++ -*-===//
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
//  This file defines a transfer function that evaluates a program statement and
//  updates an environment accordingly.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_ANALYSIS_FLOWSENSITIVE_TRANSFER_H
#define LANGUAGE_CORE_ANALYSIS_FLOWSENSITIVE_TRANSFER_H

#include "language/Core/AST/Stmt.h"
#include "language/Core/Analysis/FlowSensitive/DataflowAnalysisContext.h"
#include "language/Core/Analysis/FlowSensitive/DataflowEnvironment.h"
#include "language/Core/Analysis/FlowSensitive/TypeErasedDataflowAnalysis.h"

namespace language::Core {
namespace dataflow {

/// Maps statements to the environments of basic blocks that contain them.
class StmtToEnvMap {
public:
  // `CurBlockID` is the ID of the block currently being processed, and
  // `CurState` is the pending state currently associated with this block. These
  // are supplied separately as the pending state for the current block may not
  // yet be represented in `BlockToState`.
  StmtToEnvMap(const AdornedCFG &ACFG,
               toolchain::ArrayRef<std::optional<TypeErasedDataflowAnalysisState>>
                   BlockToState,
               unsigned CurBlockID,
               const TypeErasedDataflowAnalysisState &CurState)
      : ACFG(ACFG), BlockToState(BlockToState), CurBlockID(CurBlockID),
        CurState(CurState) {}

  /// Returns the environment of the basic block that contains `S`.
  /// The result is guaranteed never to be null.
  const Environment *getEnvironment(const Stmt &S) const;

private:
  const AdornedCFG &ACFG;
  toolchain::ArrayRef<std::optional<TypeErasedDataflowAnalysisState>> BlockToState;
  unsigned CurBlockID;
  const TypeErasedDataflowAnalysisState &CurState;
};

/// Evaluates `S` and updates `Env` accordingly.
///
/// Requirements:
///
///  `S` must not be `ParenExpr` or `ExprWithCleanups`.
void transfer(const StmtToEnvMap &StmtToEnv, const Stmt &S, Environment &Env,
              Environment::ValueModel &Model);

} // namespace dataflow
} // namespace language::Core

#endif // LANGUAGE_CORE_ANALYSIS_FLOWSENSITIVE_TRANSFER_H
