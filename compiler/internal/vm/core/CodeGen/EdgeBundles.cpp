//===-------- EdgeBundles.cpp - Bundles of CFG edges ----------------------===//
//
// Copyright (c) NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
// http://www.apache.org/licenses/LICENSE-2.0
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
// This file provides the implementation of the EdgeBundles analysis.
//
//===----------------------------------------------------------------------===//

#include "vm/core/CodeGen/EdgeBundles.h"
#include "vm/core/ADT/Twine.h"
#include "vm/core/CodeGen/MachineBasicBlock.h"
#include "vm/core/CodeGen/MachineFunction.h"
#include "vm/core/CodeGen/Passes.h"
#include "vm/core/InitializePasses.h"
#include "vm/core/Support/CommandLine.h"
#include "vm/core/Support/GraphWriter.h"
#include "vm/core/Support/raw_ostream.h"

using namespace vm::core;

static cl::opt<bool>
ViewEdgeBundles("view-edge-bundles", cl::Hidden,
                cl::desc("Pop up a window to show edge bundle graphs"));

char EdgeBundlesWrapperLegacy::ID = 0;

INITIALIZE_PASS(EdgeBundlesWrapperLegacy, "edge-bundles",
                "Bundle Machine CFG Edges",
                /* cfg = */ true, /* is_analysis = */ true)

char &toolchain::EdgeBundlesWrapperLegacyID = EdgeBundlesWrapperLegacy::ID;

void EdgeBundlesWrapperLegacy::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  MachineFunctionPass::getAnalysisUsage(AU);
}

AnalysisKey EdgeBundlesAnalysis::Key;

EdgeBundles EdgeBundlesAnalysis::run(MachineFunction &MF,
                                     MachineFunctionAnalysisManager &MFAM) {
  EdgeBundles Impl(MF);
  return Impl;
}

bool EdgeBundlesWrapperLegacy::runOnMachineFunction(MachineFunction &MF) {
  Impl.reset(new EdgeBundles(MF));
  return false;
}

EdgeBundles::EdgeBundles(MachineFunction &MF) : MF(&MF) { init(); }

void EdgeBundles::init() {
  EC.clear();
  EC.grow(2 * MF->getNumBlockIDs());

  for (const auto &MBB : *MF) {
    unsigned OutE = 2 * MBB.getNumber() + 1;
    // Join the outgoing bundle with the ingoing bundles of all successors.
    for (const MachineBasicBlock *Succ : MBB.successors())
      EC.join(OutE, 2 * Succ->getNumber());
  }
  EC.compress();
  if (ViewEdgeBundles)
    view();

  // Compute the reverse mapping.
  Blocks.clear();
  Blocks.resize(getNumBundles());

  for (unsigned i = 0, e = MF->getNumBlockIDs(); i != e; ++i) {
    unsigned b0 = getBundle(i, false);
    unsigned b1 = getBundle(i, true);
    Blocks[b0].push_back(i);
    if (b1 != b0)
      Blocks[b1].push_back(i);
  }
}

/// Specialize WriteGraph, the standard implementation won't work.
template <>
raw_ostream &toolchain::WriteGraph<>(raw_ostream &O, const EdgeBundles &G,
                                bool ShortNames, const Twine &Title) {
  const MachineFunction *MF = G.getMachineFunction();

  O << "digraph {\n";
  for (const auto &MBB : *MF) {
    unsigned BB = MBB.getNumber();
    O << "\t\"" << printMBBReference(MBB) << "\" [ shape=box, label=\""
      << printMBBReference(MBB) << "\" ]\n"
      << '\t' << G.getBundle(BB, false) << " -> \"" << printMBBReference(MBB)
      << "\"\n"
      << "\t\"" << printMBBReference(MBB) << "\" -> " << G.getBundle(BB, true)
      << '\n';
    for (const MachineBasicBlock *Succ : MBB.successors())
      O << "\t\"" << printMBBReference(MBB) << "\" -> \""
        << printMBBReference(*Succ) << "\" [ color=lightgray ]\n";
  }
  O << "}\n";
  return O;
}

/// view - Visualize the annotated bipartite CFG with Graphviz.
void EdgeBundles::view() const {
  ViewGraph(*this, "EdgeBundles");
}

bool EdgeBundles::invalidate(MachineFunction &MF, const PreservedAnalyses &PA,
                             MachineFunctionAnalysisManager::Invalidator &Inv) {
  // Invalidated when CFG is not preserved
  auto PAC = PA.getChecker<EdgeBundlesAnalysis>();
  return !PAC.preserved() && !PAC.preservedSet<CFGAnalyses>() &&
         !PAC.preservedSet<AllAnalysesOn<MachineFunction>>();
}
