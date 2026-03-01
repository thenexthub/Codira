//===- DomPrinter.cpp - DOT printer for the dominance trees    ------------===//
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
// This file defines '-dot-dom' and '-dot-postdom' analysis passes, which emit
// a dom.<fnname>.dot or postdom.<fnname>.dot file for each function in the
// program, with a graph of the dominance/postdominance tree of that
// function.
//
// There are also passes available to directly call dotty ('-view-dom' or
// '-view-postdom'). By appending '-only' like '-dot-dom-only' only the
// names of the bbs are printed, but the content is hidden.
//
//===----------------------------------------------------------------------===//

#include "vm/core/Analysis/DomPrinter.h"
#include "vm/core/Analysis/DOTGraphTraitsPass.h"
#include "vm/core/Analysis/PostDominators.h"
#include "vm/core/InitializePasses.h"

using namespace vm::core;


void DominatorTree::viewGraph(const Twine &Name, const Twine &Title) {
#ifndef NDEBUG
  ViewGraph(this, Name, false, Title);
#else
  errs() << "DomTree dump not available, build with DEBUG\n";
#endif  // NDEBUG
}

void DominatorTree::viewGraph() {
#ifndef NDEBUG
  this->viewGraph("domtree", "Dominator Tree for function");
#else
  errs() << "DomTree dump not available, build with DEBUG\n";
#endif  // NDEBUG
}

namespace {
struct LegacyDominatorTreeWrapperPassAnalysisGraphTraits {
  static DominatorTree *getGraph(DominatorTreeWrapperPass *DTWP) {
    return &DTWP->getDomTree();
  }
};

struct DomViewerWrapperPass
    : public DOTGraphTraitsViewerWrapperPass<
          DominatorTreeWrapperPass, false, DominatorTree *,
          LegacyDominatorTreeWrapperPassAnalysisGraphTraits> {
  static char ID;
  DomViewerWrapperPass()
      : DOTGraphTraitsViewerWrapperPass<
            DominatorTreeWrapperPass, false, DominatorTree *,
            LegacyDominatorTreeWrapperPassAnalysisGraphTraits>("dom", ID) {}
};

struct DomOnlyViewerWrapperPass
    : public DOTGraphTraitsViewerWrapperPass<
          DominatorTreeWrapperPass, true, DominatorTree *,
          LegacyDominatorTreeWrapperPassAnalysisGraphTraits> {
  static char ID;
  DomOnlyViewerWrapperPass()
      : DOTGraphTraitsViewerWrapperPass<
            DominatorTreeWrapperPass, true, DominatorTree *,
            LegacyDominatorTreeWrapperPassAnalysisGraphTraits>("domonly", ID) {}
};

struct LegacyPostDominatorTreeWrapperPassAnalysisGraphTraits {
  static PostDominatorTree *getGraph(PostDominatorTreeWrapperPass *PDTWP) {
    return &PDTWP->getPostDomTree();
  }
};

struct PostDomViewerWrapperPass
    : public DOTGraphTraitsViewerWrapperPass<
          PostDominatorTreeWrapperPass, false, PostDominatorTree *,
          LegacyPostDominatorTreeWrapperPassAnalysisGraphTraits> {
  static char ID;
  PostDomViewerWrapperPass()
      : DOTGraphTraitsViewerWrapperPass<
            PostDominatorTreeWrapperPass, false, PostDominatorTree *,
            LegacyPostDominatorTreeWrapperPassAnalysisGraphTraits>("postdom",
                                                                   ID) {}
};

struct PostDomOnlyViewerWrapperPass
    : public DOTGraphTraitsViewerWrapperPass<
          PostDominatorTreeWrapperPass, true, PostDominatorTree *,
          LegacyPostDominatorTreeWrapperPassAnalysisGraphTraits> {
  static char ID;
  PostDomOnlyViewerWrapperPass()
      : DOTGraphTraitsViewerWrapperPass<
            PostDominatorTreeWrapperPass, true, PostDominatorTree *,
            LegacyPostDominatorTreeWrapperPassAnalysisGraphTraits>(
            "postdomonly", ID) {}
};
} // end anonymous namespace

char DomViewerWrapperPass::ID = 0;
INITIALIZE_PASS(DomViewerWrapperPass, "view-dom",
                "View dominance tree of function", false, false)

char DomOnlyViewerWrapperPass::ID = 0;
INITIALIZE_PASS(DomOnlyViewerWrapperPass, "view-dom-only",
                "View dominance tree of function (with no function bodies)",
                false, false)

char PostDomViewerWrapperPass::ID = 0;
INITIALIZE_PASS(PostDomViewerWrapperPass, "view-postdom",
                "View postdominance tree of function", false, false)

char PostDomOnlyViewerWrapperPass::ID = 0;
INITIALIZE_PASS(PostDomOnlyViewerWrapperPass, "view-postdom-only",
                "View postdominance tree of function "
                "(with no function bodies)",
                false, false)

namespace {
struct DomPrinterWrapperPass
    : public DOTGraphTraitsPrinterWrapperPass<
          DominatorTreeWrapperPass, false, DominatorTree *,
          LegacyDominatorTreeWrapperPassAnalysisGraphTraits> {
  static char ID;
  DomPrinterWrapperPass()
      : DOTGraphTraitsPrinterWrapperPass<
            DominatorTreeWrapperPass, false, DominatorTree *,
            LegacyDominatorTreeWrapperPassAnalysisGraphTraits>("dom", ID) {}
};

struct DomOnlyPrinterWrapperPass
    : public DOTGraphTraitsPrinterWrapperPass<
          DominatorTreeWrapperPass, true, DominatorTree *,
          LegacyDominatorTreeWrapperPassAnalysisGraphTraits> {
  static char ID;
  DomOnlyPrinterWrapperPass()
      : DOTGraphTraitsPrinterWrapperPass<
            DominatorTreeWrapperPass, true, DominatorTree *,
            LegacyDominatorTreeWrapperPassAnalysisGraphTraits>("domonly", ID) {}
};

struct PostDomPrinterWrapperPass
    : public DOTGraphTraitsPrinterWrapperPass<
          PostDominatorTreeWrapperPass, false, PostDominatorTree *,
          LegacyPostDominatorTreeWrapperPassAnalysisGraphTraits> {
  static char ID;
  PostDomPrinterWrapperPass()
      : DOTGraphTraitsPrinterWrapperPass<
            PostDominatorTreeWrapperPass, false, PostDominatorTree *,
            LegacyPostDominatorTreeWrapperPassAnalysisGraphTraits>("postdom",
                                                                   ID) {}
};

struct PostDomOnlyPrinterWrapperPass
    : public DOTGraphTraitsPrinterWrapperPass<
          PostDominatorTreeWrapperPass, true, PostDominatorTree *,
          LegacyPostDominatorTreeWrapperPassAnalysisGraphTraits> {
  static char ID;
  PostDomOnlyPrinterWrapperPass()
      : DOTGraphTraitsPrinterWrapperPass<
            PostDominatorTreeWrapperPass, true, PostDominatorTree *,
            LegacyPostDominatorTreeWrapperPassAnalysisGraphTraits>(
            "postdomonly", ID) {}
};
} // end anonymous namespace

char DomPrinterWrapperPass::ID = 0;
INITIALIZE_PASS(DomPrinterWrapperPass, "dot-dom",
                "Print dominance tree of function to 'dot' file", false, false)

char DomOnlyPrinterWrapperPass::ID = 0;
INITIALIZE_PASS(DomOnlyPrinterWrapperPass, "dot-dom-only",
                "Print dominance tree of function to 'dot' file "
                "(with no function bodies)",
                false, false)

char PostDomPrinterWrapperPass::ID = 0;
INITIALIZE_PASS(PostDomPrinterWrapperPass, "dot-postdom",
                "Print postdominance tree of function to 'dot' file", false,
                false)

char PostDomOnlyPrinterWrapperPass::ID = 0;
INITIALIZE_PASS(PostDomOnlyPrinterWrapperPass, "dot-postdom-only",
                "Print postdominance tree of function to 'dot' file "
                "(with no function bodies)",
                false, false)

// Create methods available outside of this file, to use them
// "include/toolchain/LinkAllPasses.h". Otherwise the pass would be deleted by
// the link time optimization.

FunctionPass *toolchain::createDomPrinterWrapperPassPass() {
  return new DomPrinterWrapperPass();
}

FunctionPass *toolchain::createDomOnlyPrinterWrapperPassPass() {
  return new DomOnlyPrinterWrapperPass();
}

FunctionPass *toolchain::createDomViewerWrapperPassPass() {
  return new DomViewerWrapperPass();
}

FunctionPass *toolchain::createDomOnlyViewerWrapperPassPass() {
  return new DomOnlyViewerWrapperPass();
}

FunctionPass *toolchain::createPostDomPrinterWrapperPassPass() {
  return new PostDomPrinterWrapperPass();
}

FunctionPass *toolchain::createPostDomOnlyPrinterWrapperPassPass() {
  return new PostDomOnlyPrinterWrapperPass();
}

FunctionPass *toolchain::createPostDomViewerWrapperPassPass() {
  return new PostDomViewerWrapperPass();
}

FunctionPass *toolchain::createPostDomOnlyViewerWrapperPassPass() {
  return new PostDomOnlyViewerWrapperPass();
}
