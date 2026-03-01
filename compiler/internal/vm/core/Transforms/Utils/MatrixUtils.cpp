//===- MatrixUtils.cpp - Utilities to lower matrix intrinsics ---*- C++ -*-===//
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
// Utilities for generating tiled loops for matrix operations.
//
//===----------------------------------------------------------------------===//

#include "vm/core/Transforms/Utils/MatrixUtils.h"
#include "vm/core/Analysis/DomTreeUpdater.h"
#include "vm/core/Analysis/LoopInfo.h"
#include "vm/core/IR/BasicBlock.h"
#include "vm/core/IR/Dominators.h"
#include "vm/core/IR/IRBuilder.h"
#include "vm/core/IR/Type.h"

using namespace vm::core;

BasicBlock *TileInfo::CreateLoop(BasicBlock *Preheader, BasicBlock *Exit,
                                 Value *Bound, Value *Step, StringRef Name,
                                 IRBuilderBase &B, DomTreeUpdater &DTU, Loop *L,
                                 LoopInfo &LI) {
  LLVMContext &Ctx = Preheader->getContext();
  BasicBlock *Header = BasicBlock::Create(
      Preheader->getContext(), Name + ".header", Preheader->getParent(), Exit);
  BasicBlock *Body = BasicBlock::Create(Header->getContext(), Name + ".body",
                                        Header->getParent(), Exit);
  BasicBlock *Latch = BasicBlock::Create(Header->getContext(), Name + ".latch",
                                         Header->getParent(), Exit);

  Type *I32Ty = Type::getInt64Ty(Ctx);
  BranchInst::Create(Body, Header);
  BranchInst::Create(Latch, Body);
  PHINode *IV =
      PHINode::Create(I32Ty, 2, Name + ".iv", Header->getTerminator()->getIterator());
  IV->addIncoming(ConstantInt::get(I32Ty, 0), Preheader);

  B.SetInsertPoint(Latch);
  Value *Inc = B.CreateAdd(IV, Step, Name + ".step");
  Value *Cond = B.CreateICmpNE(Inc, Bound, Name + ".cond");
  BranchInst::Create(Header, Exit, Cond, Latch);
  IV->addIncoming(Inc, Latch);

  BranchInst *PreheaderBr = cast<BranchInst>(Preheader->getTerminator());
  BasicBlock *Tmp = PreheaderBr->getSuccessor(0);
  PreheaderBr->setSuccessor(0, Header);
  DTU.applyUpdatesPermissive({
      {DominatorTree::Delete, Preheader, Tmp},
      {DominatorTree::Insert, Header, Body},
      {DominatorTree::Insert, Body, Latch},
      {DominatorTree::Insert, Latch, Header},
      {DominatorTree::Insert, Latch, Exit},
      {DominatorTree::Insert, Preheader, Header},
  });

  L->addBasicBlockToLoop(Header, LI);
  L->addBasicBlockToLoop(Body, LI);
  L->addBasicBlockToLoop(Latch, LI);
  return Body;
}

// Creates the following loop nest skeleton:
//  for C = 0; C < NumColumns; C += TileSize
//    for R = 0; R < NumRows; R += TileSize
//      for K = 0; K < Inner ; K += TileSize
BasicBlock *TileInfo::CreateTiledLoops(BasicBlock *Start, BasicBlock *End,
                                       IRBuilderBase &B, DomTreeUpdater &DTU,
                                       LoopInfo &LI) {
  Loop *ColumnLoopInfo = LI.AllocateLoop();
  Loop *RowLoopInfo = LI.AllocateLoop();
  Loop *KLoopInfo = LI.AllocateLoop();
  RowLoopInfo->addChildLoop(KLoopInfo);
  ColumnLoopInfo->addChildLoop(RowLoopInfo);
  if (Loop *ParentL = LI.getLoopFor(Start))
    ParentL->addChildLoop(ColumnLoopInfo);
  else
    LI.addTopLevelLoop(ColumnLoopInfo);

  BasicBlock *ColBody =
      CreateLoop(Start, End, B.getInt64(NumColumns), B.getInt64(TileSize),
                 "cols", B, DTU, ColumnLoopInfo, LI);
  ColumnLoop.Latch = ColBody->getSingleSuccessor();
  BasicBlock *RowBody =
      CreateLoop(ColBody, ColumnLoop.Latch, B.getInt64(NumRows),
                 B.getInt64(TileSize), "rows", B, DTU, RowLoopInfo, LI);
  RowLoop.Latch = RowBody->getSingleSuccessor();

  BasicBlock *InnerBody =
      CreateLoop(RowBody, RowLoop.Latch, B.getInt64(NumInner),
                 B.getInt64(TileSize), "inner", B, DTU, KLoopInfo, LI);
  KLoop.Latch = InnerBody->getSingleSuccessor();
  ColumnLoop.Header = ColBody->getSinglePredecessor();
  RowLoop.Header = RowBody->getSinglePredecessor();
  KLoop.Header = InnerBody->getSinglePredecessor();
  RowLoop.Index = &*RowLoop.Header->begin();
  ColumnLoop.Index = &*ColumnLoop.Header->begin();
  KLoop.Index = &*KLoop.Header->begin();

  return InnerBody;
}
