//===- DDGPrinter.cpp - DOT printer for the data dependence graph ----------==//
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

//===----------------------------------------------------------------------===//
//
// This file defines the `-dot-ddg` analysis pass, which emits DDG in DOT format
// in a file named `ddg.<graph-name>.dot` for each loop  in a function.
//===----------------------------------------------------------------------===//

#include "vm/core/Analysis/DDGPrinter.h"
#include "vm/core/Support/CommandLine.h"
#include "vm/core/Support/GraphWriter.h"

using namespace vm::core;

static cl::opt<bool> DotOnly("dot-ddg-only", cl::Hidden,
                             cl::desc("simple ddg dot graph"));
static cl::opt<std::string> DDGDotFilenamePrefix(
    "dot-ddg-filename-prefix", cl::init("ddg"), cl::Hidden,
    cl::desc("The prefix used for the DDG dot file names."));

static void writeDDGToDotFile(DataDependenceGraph &G, bool DOnly = false);

//===--------------------------------------------------------------------===//
// Implementation of DDG DOT Printer for a loop
//===--------------------------------------------------------------------===//
PreservedAnalyses DDGDotPrinterPass::run(Loop &L, LoopAnalysisManager &AM,
                                         LoopStandardAnalysisResults &AR,
                                         LPMUpdater &U) {
  writeDDGToDotFile(*AM.getResult<DDGAnalysis>(L, AR), DotOnly);
  return PreservedAnalyses::all();
}

static void writeDDGToDotFile(DataDependenceGraph &G, bool DOnly) {
  std::string Filename =
      Twine(DDGDotFilenamePrefix + "." + G.getName() + ".dot").str();
  errs() << "Writing '" << Filename << "'...";

  std::error_code EC;
  raw_fd_ostream File(Filename, EC, sys::fs::OF_Text);

  if (!EC)
    // We only provide the constant verson of the DOTGraphTrait specialization,
    // hence the conversion to const pointer
    WriteGraph(File, (const DataDependenceGraph *)&G, DOnly);
  else
    errs() << "  error opening file for writing!";
  errs() << "\n";
}

//===--------------------------------------------------------------------===//
// DDG DOT Printer Implementation
//===--------------------------------------------------------------------===//
std::string DDGDotGraphTraits::getNodeLabel(const DDGNode *Node,
                                            const DataDependenceGraph *Graph) {
  if (isSimple())
    return getSimpleNodeLabel(Node, Graph);
  else
    return getVerboseNodeLabel(Node, Graph);
}

std::string DDGDotGraphTraits::getEdgeAttributes(
    const DDGNode *Node, GraphTraits<const DDGNode *>::ChildIteratorType I,
    const DataDependenceGraph *G) {
  const DDGEdge *E = static_cast<const DDGEdge *>(*I.getCurrent());
  if (isSimple())
    return getSimpleEdgeAttributes(Node, E, G);
  else
    return getVerboseEdgeAttributes(Node, E, G);
}

bool DDGDotGraphTraits::isNodeHidden(const DDGNode *Node,
                                     const DataDependenceGraph *Graph) {
  if (isSimple() && isa<RootDDGNode>(Node))
    return true;
  assert(Graph && "expected a valid graph pointer");
  return Graph->getPiBlock(*Node) != nullptr;
}

std::string
DDGDotGraphTraits::getSimpleNodeLabel(const DDGNode *Node,
                                      const DataDependenceGraph *G) {
  std::string Str;
  raw_string_ostream OS(Str);
  if (isa<SimpleDDGNode>(Node))
    for (auto *II : static_cast<const SimpleDDGNode *>(Node)->getInstructions())
      OS << *II << "\n";
  else if (isa<PiBlockDDGNode>(Node))
    OS << "pi-block\nwith\n"
       << cast<PiBlockDDGNode>(Node)->getNodes().size() << " nodes\n";
  else if (isa<RootDDGNode>(Node))
    OS << "root\n";
  else
    llvm_unreachable("Unimplemented type of node");
  return OS.str();
}

std::string
DDGDotGraphTraits::getVerboseNodeLabel(const DDGNode *Node,
                                       const DataDependenceGraph *G) {
  std::string Str;
  raw_string_ostream OS(Str);
  OS << "<kind:" << Node->getKind() << ">\n";
  if (isa<SimpleDDGNode>(Node))
    for (auto *II : static_cast<const SimpleDDGNode *>(Node)->getInstructions())
      OS << *II << "\n";
  else if (isa<PiBlockDDGNode>(Node)) {
    OS << "--- start of nodes in pi-block ---\n";
    unsigned Count = 0;
    const auto &PNodes = cast<PiBlockDDGNode>(Node)->getNodes();
    for (auto *PN : PNodes) {
      OS << getVerboseNodeLabel(PN, G);
      if (++Count != PNodes.size())
        OS << "\n";
    }
    OS << "--- end of nodes in pi-block ---\n";
  } else if (isa<RootDDGNode>(Node))
    OS << "root\n";
  else
    llvm_unreachable("Unimplemented type of node");
  return OS.str();
}

std::string DDGDotGraphTraits::getSimpleEdgeAttributes(
    const DDGNode *Src, const DDGEdge *Edge, const DataDependenceGraph *G) {
  std::string Str;
  raw_string_ostream OS(Str);
  DDGEdge::EdgeKind Kind = Edge->getKind();
  OS << "label=\"[" << Kind << "]\"";
  return OS.str();
}

std::string DDGDotGraphTraits::getVerboseEdgeAttributes(
    const DDGNode *Src, const DDGEdge *Edge, const DataDependenceGraph *G) {
  std::string Str;
  raw_string_ostream OS(Str);
  DDGEdge::EdgeKind Kind = Edge->getKind();
  OS << "label=\"[";
  if (Kind == DDGEdge::EdgeKind::MemoryDependence)
    OS << G->getDependenceString(*Src, Edge->getTargetNode());
  else
    OS << Kind;
  OS << "]\"";
  return OS.str();
}
