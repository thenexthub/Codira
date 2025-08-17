//===--- LoopWidening.cpp - Widen loops -------------------------*- C++ -*-===//
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
///
/// This file contains functions which are used to widen loops. A loop may be
/// widened to approximate the exit state(s), without analyzing every
/// iteration. The widening is done by invalidating anything which might be
/// modified by the body of the loop.
///
//===----------------------------------------------------------------------===//

#include "language/Core/StaticAnalyzer/Core/PathSensitive/LoopWidening.h"
#include "language/Core/ASTMatchers/ASTMatchFinder.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/ExplodedGraph.h"

using namespace language::Core;
using namespace ento;
using namespace language::Core::ast_matchers;

const auto MatchRef = "matchref";

namespace language::Core {
namespace ento {

ProgramStateRef getWidenedLoopState(ProgramStateRef PrevState,
                                    const LocationContext *LCtx,
                                    unsigned BlockCount,
                                    ConstCFGElementRef Elem) {
  // Invalidate values in the current state.
  // TODO Make this more conservative by only invalidating values that might
  //      be modified by the body of the loop.
  // TODO Nested loops are currently widened as a result of the invalidation
  //      being so inprecise. When the invalidation is improved, the handling
  //      of nested loops will also need to be improved.
  ASTContext &ASTCtx = LCtx->getAnalysisDeclContext()->getASTContext();
  const StackFrameContext *STC = LCtx->getStackFrame();
  MemRegionManager &MRMgr = PrevState->getStateManager().getRegionManager();
  const MemRegion *Regions[] = {MRMgr.getStackLocalsRegion(STC),
                                MRMgr.getStackArgumentsRegion(STC),
                                MRMgr.getGlobalsRegion()};
  RegionAndSymbolInvalidationTraits ITraits;
  for (auto *Region : Regions) {
    ITraits.setTrait(Region,
                     RegionAndSymbolInvalidationTraits::TK_EntireMemSpace);
  }

  // References should not be invalidated.
  auto Matches = match(
      findAll(stmt(hasDescendant(
          varDecl(hasType(hasCanonicalType(referenceType()))).bind(MatchRef)))),
      *LCtx->getDecl()->getBody(), ASTCtx);
  for (BoundNodes Match : Matches) {
    const VarDecl *VD = Match.getNodeAs<VarDecl>(MatchRef);
    assert(VD);
    const VarRegion *VarMem = MRMgr.getVarRegion(VD, LCtx);
    ITraits.setTrait(VarMem,
                     RegionAndSymbolInvalidationTraits::TK_PreserveContents);
  }


  // 'this' pointer is not an lvalue, we should not invalidate it. If the loop
  // is located in a method, constructor or destructor, the value of 'this'
  // pointer should remain unchanged.  Ignore static methods, since they do not
  // have 'this' pointers.
  const CXXMethodDecl *CXXMD = dyn_cast<CXXMethodDecl>(STC->getDecl());
  if (CXXMD && CXXMD->isImplicitObjectMemberFunction()) {
    const CXXThisRegion *ThisR =
        MRMgr.getCXXThisRegion(CXXMD->getThisType(), STC);
    ITraits.setTrait(ThisR,
                     RegionAndSymbolInvalidationTraits::TK_PreserveContents);
  }

  return PrevState->invalidateRegions(Regions, Elem, BlockCount, LCtx, true,
                                      nullptr, nullptr, &ITraits);
}

} // end namespace ento
} // end namespace language::Core
