//==--AnalyzerStatsChecker.cpp - Analyzer visitation statistics --*- C++ -*-==//
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
// This file reports various statistics about analyzer visitation.
//===----------------------------------------------------------------------===//
#include "language/Core/AST/DeclObjC.h"
#include "language/Core/Basic/SourceManager.h"
#include "language/Core/StaticAnalyzer/Checkers/BuiltinCheckerRegistration.h"
#include "language/Core/StaticAnalyzer/Core/BugReporter/BugReporter.h"
#include "language/Core/StaticAnalyzer/Core/Checker.h"
#include "language/Core/StaticAnalyzer/Core/CheckerManager.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/EntryPointStats.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/ExplodedGraph.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/ExprEngine.h"
#include "toolchain/ADT/STLExtras.h"
#include "toolchain/ADT/SmallPtrSet.h"
#include "toolchain/Support/raw_ostream.h"
#include <optional>

using namespace language::Core;
using namespace ento;

#define DEBUG_TYPE "StatsChecker"

STAT_COUNTER(NumBlocks, "The # of blocks in top level functions");
STAT_COUNTER(NumBlocksUnreachable,
             "The # of unreachable blocks in analyzing top level functions");

namespace {
class AnalyzerStatsChecker : public Checker<check::EndAnalysis> {
public:
  void checkEndAnalysis(ExplodedGraph &G, BugReporter &B,ExprEngine &Eng) const;
};
}

void AnalyzerStatsChecker::checkEndAnalysis(ExplodedGraph &G,
                                            BugReporter &B,
                                            ExprEngine &Eng) const {
  const CFG *C = nullptr;
  const SourceManager &SM = B.getSourceManager();
  toolchain::SmallPtrSet<const CFGBlock*, 32> reachable;

  const LocationContext *LC = Eng.getRootLocationContext();

  const Decl *D = LC->getDecl();

  // Iterate over the exploded graph.
  for (const ExplodedNode &N : G.nodes()) {
    const ProgramPoint &P = N.getLocation();

    // Only check the coverage in the top level function (optimization).
    if (D != P.getLocationContext()->getDecl())
      continue;

    if (std::optional<BlockEntrance> BE = P.getAs<BlockEntrance>()) {
      const CFGBlock *CB = BE->getBlock();
      reachable.insert(CB);
    }
  }

  // Get the CFG and the Decl of this block.
  C = LC->getCFG();

  unsigned total = 0, unreachable = 0;

  // Find CFGBlocks that were not covered by any node
  for (CFG::const_iterator I = C->begin(); I != C->end(); ++I) {
    const CFGBlock *CB = *I;
    ++total;
    // Check if the block is unreachable
    if (!reachable.count(CB)) {
      ++unreachable;
    }
  }

  // We never 'reach' the entry block, so correct the unreachable count
  unreachable--;
  // There is no BlockEntrance corresponding to the exit block as well, so
  // assume it is reached as well.
  unreachable--;

  // Generate the warning string
  SmallString<128> buf;
  toolchain::raw_svector_ostream output(buf);
  PresumedLoc Loc = SM.getPresumedLoc(D->getLocation());
  if (!Loc.isValid())
    return;

  if (isa<FunctionDecl, ObjCMethodDecl>(D)) {
    const NamedDecl *ND = cast<NamedDecl>(D);
    output << *ND;
  } else if (isa<BlockDecl>(D)) {
    output << "block(line:" << Loc.getLine() << ":col:" << Loc.getColumn();
  }

  NumBlocksUnreachable += unreachable;
  NumBlocks += total;
  std::string NameOfRootFunction = std::string(output.str());

  output << " -> Total CFGBlocks: " << total << " | Unreachable CFGBlocks: "
      << unreachable << " | Exhausted Block: "
      << (Eng.wasBlocksExhausted() ? "yes" : "no")
      << " | Empty WorkList: "
      << (Eng.hasEmptyWorkList() ? "yes" : "no");

  B.EmitBasicReport(D, this, "Analyzer Statistics", "Internal Statistics",
                    output.str(), PathDiagnosticLocation(D, SM));

  // Emit warning for each block we bailed out on.
  const CoreEngine &CE = Eng.getCoreEngine();
  for (const BlockEdge &BE : make_first_range(CE.exhausted_blocks())) {
    const CFGBlock *Exit = BE.getDst();
    if (Exit->empty())
      continue;
    const CFGElement &CE = Exit->front();
    if (std::optional<CFGStmt> CS = CE.getAs<CFGStmt>()) {
      SmallString<128> bufI;
      toolchain::raw_svector_ostream outputI(bufI);
      outputI << "(" << NameOfRootFunction << ")" <<
                 ": The analyzer generated a sink at this point";
      B.EmitBasicReport(
          D, this, "Sink Point", "Internal Statistics", outputI.str(),
          PathDiagnosticLocation::createBegin(CS->getStmt(), SM, LC));
    }
  }
}

void ento::registerAnalyzerStatsChecker(CheckerManager &mgr) {
  mgr.registerChecker<AnalyzerStatsChecker>();
}

bool ento::shouldRegisterAnalyzerStatsChecker(const CheckerManager &mgr) {
  return true;
}
