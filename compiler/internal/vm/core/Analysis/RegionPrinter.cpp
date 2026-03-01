//===- RegionPrinter.cpp - Print regions tree pass ------------------------===//
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
// Print out the region tree of a function using dotty/graphviz.
//===----------------------------------------------------------------------===//

#include "vm/core/Analysis/RegionPrinter.h"
#include "vm/core/Analysis/DOTGraphTraitsPass.h"
#include "vm/core/Analysis/RegionInfo.h"
#include "vm/core/Analysis/RegionIterator.h"
#include "vm/core/InitializePasses.h"
#include "vm/core/Support/CommandLine.h"
#include "vm/core/Support/raw_ostream.h"
#ifndef NDEBUG
#include "vm/core/IR/LegacyPassManager.h"
#endif

using namespace vm::core;

//===----------------------------------------------------------------------===//
/// onlySimpleRegion - Show only the simple regions in the RegionViewer.
static cl::opt<bool>
onlySimpleRegions("only-simple-regions",
                  cl::desc("Show only simple regions in the graphviz viewer"),
                  cl::Hidden,
                  cl::init(false));

std::string
toolchain::DOTGraphTraits<RegionNode *>::getNodeLabel(RegionNode *Node,
                                                 RegionNode *Graph) {
  if (!Node->isSubRegion()) {
    BasicBlock *BB = Node->getNodeAs<BasicBlock>();

    if (isSimple())
      return DOTGraphTraits<DOTFuncInfo *>::getSimpleNodeLabel(BB, nullptr);
    else
      return DOTGraphTraits<DOTFuncInfo *>::getCompleteNodeLabel(BB, nullptr);
  }

  return "Not implemented";
}

template <>
struct toolchain::DOTGraphTraits<RegionInfo *>
    : public toolchain::DOTGraphTraits<RegionNode *> {

  DOTGraphTraits (bool isSimple = false)
    : DOTGraphTraits<RegionNode*>(isSimple) {}

  static std::string getGraphName(const RegionInfo *) { return "Region Graph"; }

  std::string getNodeLabel(RegionNode *Node, RegionInfo *G) {
    return DOTGraphTraits<RegionNode *>::getNodeLabel(
        Node, reinterpret_cast<RegionNode *>(G->getTopLevelRegion()));
  }

  std::string getEdgeAttributes(RegionNode *srcNode,
                                GraphTraits<RegionInfo *>::ChildIteratorType CI,
                                RegionInfo *G) {
    RegionNode *destNode = *CI;

    if (srcNode->isSubRegion() || destNode->isSubRegion())
      return "";

    // In case of a backedge, do not use it to define the layout of the nodes.
    BasicBlock *srcBB = srcNode->getNodeAs<BasicBlock>();
    BasicBlock *destBB = destNode->getNodeAs<BasicBlock>();

    Region *R = G->getRegionFor(destBB);

    while (R && R->getParent())
      if (R->getParent()->getEntry() == destBB)
        R = R->getParent();
      else
        break;

    if (R && R->getEntry() == destBB && R->contains(srcBB))
      return "constraint=false";

    return "";
  }

  // Print the cluster of the subregions. This groups the single basic blocks
  // and adds a different background color for each group.
  static void printRegionCluster(const Region &R, GraphWriter<RegionInfo *> &GW,
                                 unsigned depth = 0) {
    raw_ostream &O = GW.getOStream();
    O.indent(2 * depth) << "subgraph cluster_" << static_cast<const void*>(&R)
      << " {\n";
    O.indent(2 * (depth + 1)) << "label = \"\";\n";

    if (!onlySimpleRegions || R.isSimple()) {
      O.indent(2 * (depth + 1)) << "style = filled;\n";
      O.indent(2 * (depth + 1)) << "color = "
        << ((R.getDepth() * 2 % 12) + 1) << "\n";

    } else {
      O.indent(2 * (depth + 1)) << "style = solid;\n";
      O.indent(2 * (depth + 1)) << "color = "
        << ((R.getDepth() * 2 % 12) + 2) << "\n";
    }

    for (const auto &RI : R)
      printRegionCluster(*RI, GW, depth + 1);

    const RegionInfo &RI = *static_cast<const RegionInfo*>(R.getRegionInfo());

    for (auto *BB : R.blocks())
      if (RI.getRegionFor(BB) == &R)
        O.indent(2 * (depth + 1)) << "Node"
          << static_cast<const void*>(RI.getTopLevelRegion()->getBBNode(BB))
          << ";\n";

    O.indent(2 * depth) << "}\n";
  }

  static void addCustomGraphFeatures(const RegionInfo *G,
                                     GraphWriter<RegionInfo *> &GW) {
    raw_ostream &O = GW.getOStream();
    O << "\tcolorscheme = \"paired12\"\n";
    printRegionCluster(*G->getTopLevelRegion(), GW, 4);
  }
};

namespace {

struct RegionInfoPassGraphTraits {
  static RegionInfo *getGraph(RegionInfoPass *RIP) {
    return &RIP->getRegionInfo();
  }
};

struct RegionPrinter
    : public DOTGraphTraitsPrinterWrapperPass<
          RegionInfoPass, false, RegionInfo *, RegionInfoPassGraphTraits> {
  static char ID;
  RegionPrinter()
      : DOTGraphTraitsPrinterWrapperPass<RegionInfoPass, false, RegionInfo *,
                                         RegionInfoPassGraphTraits>("reg", ID) {
  }
};
char RegionPrinter::ID = 0;

struct RegionOnlyPrinter
    : public DOTGraphTraitsPrinterWrapperPass<
          RegionInfoPass, true, RegionInfo *, RegionInfoPassGraphTraits> {
  static char ID;
  RegionOnlyPrinter()
      : DOTGraphTraitsPrinterWrapperPass<RegionInfoPass, true, RegionInfo *,
                                         RegionInfoPassGraphTraits>("reg", ID) {
  }
};
char RegionOnlyPrinter::ID = 0;

struct RegionViewer
    : public DOTGraphTraitsViewerWrapperPass<
          RegionInfoPass, false, RegionInfo *, RegionInfoPassGraphTraits> {
  static char ID;
  RegionViewer()
      : DOTGraphTraitsViewerWrapperPass<RegionInfoPass, false, RegionInfo *,
                                        RegionInfoPassGraphTraits>("reg", ID) {}
};
char RegionViewer::ID = 0;

struct RegionOnlyViewer
    : public DOTGraphTraitsViewerWrapperPass<RegionInfoPass, true, RegionInfo *,
                                             RegionInfoPassGraphTraits> {
  static char ID;
  RegionOnlyViewer()
      : DOTGraphTraitsViewerWrapperPass<RegionInfoPass, true, RegionInfo *,
                                        RegionInfoPassGraphTraits>("regonly",
                                                                   ID) {}
};
char RegionOnlyViewer::ID = 0;

} //end anonymous namespace

INITIALIZE_PASS(RegionPrinter, "dot-regions",
                "Print regions of function to 'dot' file", true, true)

INITIALIZE_PASS(
    RegionOnlyPrinter, "dot-regions-only",
    "Print regions of function to 'dot' file (with no function bodies)", true,
    true)

INITIALIZE_PASS(RegionViewer, "view-regions", "View regions of function",
                true, true)

INITIALIZE_PASS(RegionOnlyViewer, "view-regions-only",
                "View regions of function (with no function bodies)",
                true, true)

FunctionPass *toolchain::createRegionPrinterPass() { return new RegionPrinter(); }

FunctionPass *toolchain::createRegionOnlyPrinterPass() {
  return new RegionOnlyPrinter();
}

FunctionPass* toolchain::createRegionViewerPass() {
  return new RegionViewer();
}

FunctionPass* toolchain::createRegionOnlyViewerPass() {
  return new RegionOnlyViewer();
}

#ifndef NDEBUG
static void viewRegionInfo(RegionInfo *RI, bool ShortNames) {
  assert(RI && "Argument must be non-null");

  toolchain::Function *F = RI->getTopLevelRegion()->getEntry()->getParent();
  std::string GraphName = DOTGraphTraits<RegionInfo *>::getGraphName(RI);

  toolchain::ViewGraph(RI, "reg", ShortNames,
                  Twine(GraphName) + " for '" + F->getName() + "' function");
}

static void invokeFunctionPass(const Function *F, FunctionPass *ViewerPass) {
  assert(F && "Argument must be non-null");
  assert(!F->isDeclaration() && "Function must have an implementation");

  // The viewer and analysis passes do not modify anything, so we can safely
  // remove the const qualifier
  auto NonConstF = const_cast<Function *>(F);

  toolchain::legacy::FunctionPassManager FPM(NonConstF->getParent());
  FPM.add(ViewerPass);
  FPM.doInitialization();
  FPM.run(*NonConstF);
  FPM.doFinalization();
}

void toolchain::viewRegion(RegionInfo *RI) { viewRegionInfo(RI, false); }

void toolchain::viewRegion(const Function *F) {
  invokeFunctionPass(F, createRegionViewerPass());
}

void toolchain::viewRegionOnly(RegionInfo *RI) { viewRegionInfo(RI, true); }

void toolchain::viewRegionOnly(const Function *F) {
  invokeFunctionPass(F, createRegionOnlyViewerPass());
}
#endif
