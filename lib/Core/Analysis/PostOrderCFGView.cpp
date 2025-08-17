//===- PostOrderCFGView.cpp - Post order view of CFG blocks ---------------===//
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
// This file implements post order view of the blocks in a CFG.
//
//===----------------------------------------------------------------------===//

#include "language/Core/Analysis/Analyses/PostOrderCFGView.h"
#include "language/Core/Analysis/AnalysisDeclContext.h"
#include "language/Core/Analysis/CFG.h"

using namespace language::Core;

void PostOrderCFGView::anchor() {}

PostOrderCFGView::PostOrderCFGView(const CFG *cfg) {
  Blocks.reserve(cfg->getNumBlockIDs());
  CFGBlockSet BSet(cfg);

  for (po_iterator I = po_iterator::begin(cfg, BSet),
                   E = po_iterator::end(cfg, BSet); I != E; ++I) {
    BlockOrder[*I] = Blocks.size() + 1;
    Blocks.push_back(*I);
  }
}

std::unique_ptr<PostOrderCFGView>
PostOrderCFGView::create(AnalysisDeclContext &ctx) {
  const CFG *cfg = ctx.getCFG();
  if (!cfg)
    return nullptr;
  return std::make_unique<PostOrderCFGView>(cfg);
}

const void *PostOrderCFGView::getTag() { static int x; return &x; }

bool PostOrderCFGView::BlockOrderCompare::operator()(const CFGBlock *b1,
                                                     const CFGBlock *b2) const {
  PostOrderCFGView::BlockOrderTy::const_iterator b1It = POV.BlockOrder.find(b1);
  PostOrderCFGView::BlockOrderTy::const_iterator b2It = POV.BlockOrder.find(b2);

  unsigned b1V = (b1It == POV.BlockOrder.end()) ? 0 : b1It->second;
  unsigned b2V = (b2It == POV.BlockOrder.end()) ? 0 : b2It->second;
  return b1V > b2V;
}
