//===--- LoopUnrolling.h - Unroll loops -------------------------*- C++ -*-===//
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
/// \file
/// This header contains the declarations of functions which are used to decide
/// which loops should be completely unrolled and mark their corresponding
/// CFGBlocks. It is done by tracking a stack of loops in the ProgramState. This
/// way specific loops can be marked as completely unrolled. For considering a
/// loop to be completely unrolled it has to fulfill the following requirements:
/// - Currently only forStmts can be considered.
/// - The bound has to be known.
/// - The counter variable has not escaped before/in the body of the loop and
///   changed only in the increment statement corresponding to the loop. It also
///   has to be initialized by a literal in the corresponding initStmt.
/// - Does not contain goto, switch and returnStmt.
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_STATICANALYZER_CORE_PATHSENSITIVE_LOOPUNROLLING_H
#define LANGUAGE_CORE_STATICANALYZER_CORE_PATHSENSITIVE_LOOPUNROLLING_H

#include "language/Core/Analysis/CFG.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/ProgramState.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/ExplodedGraph.h"
namespace language::Core {
namespace ento {

/// Returns if the given State indicates that is inside a completely unrolled
/// loop.
bool isUnrolledState(ProgramStateRef State);

/// Updates the stack of loops contained by the ProgramState.
ProgramStateRef updateLoopStack(const Stmt *LoopStmt, ASTContext &ASTCtx,
                                ExplodedNode* Pred, unsigned maxVisitOnPath);

/// Updates the given ProgramState. In current implementation it removes the top
/// element of the stack of loops.
ProgramStateRef processLoopEnd(const Stmt *LoopStmt, ProgramStateRef State);

} // end namespace ento
} // end namespace language::Core

#endif
