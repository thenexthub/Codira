//===--- SCCVisitor.h - SIL SCC Visitor -------------------------*- C++ -*-===//
//
// Copyright (c) NeXTHub Corporation. All rights reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// This code is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// version 2 for more details (a copy is included in the LICENSE file that
// accompanied this code).
//
// Author(-s): Tunjay Akbarli
//

//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_SIL_SCCVISITOR_H
#define LANGUAGE_SIL_SCCVISITOR_H

#include "language/SILOptimizer/Analysis/Analysis.h"
#include "language/SIL/CFG.h"
#include "language/SIL/SILArgument.h"
#include "language/SIL/SILBasicBlock.h"
#include "language/SIL/SILFunction.h"
#include "language/SIL/SILInstruction.h"
#include "language/SIL/SILValue.h"
#include "toolchain/ADT/DenseMap.h"
#include "toolchain/ADT/PostOrderIterator.h"
#include "toolchain/ADT/SetVector.h"
#include "toolchain/ADT/SmallVector.h"
#include <algorithm>
#include <tuple>

namespace language {

/// A visitor class for visiting the instructions and basic block
/// arguments of a SIL function one strongly connected component at a
/// time in reverse post-order.
///
/// Inherit from this in the usual CRTP fashion and define a visit()
/// function. This function will get called with a vector of
/// pointer-to-ValueBase which are the elements of the SCC.
template <typename ImplClass>
class SCCVisitor {
public:
  SCCVisitor(SILFunction &F) : F(F) {}
  ~SCCVisitor() {
    cleanup();
  }
  ImplClass &asImpl() {  return static_cast<ImplClass &>(*this); }

  void visit(toolchain::SmallVectorImpl<SILNode *> &SCC) { }

  void run() {
    toolchain::ReversePostOrderTraversal<SILFunction *> RPOT(&F);

    for (auto Iter = RPOT.begin(), E = RPOT.end(); Iter != E; ++Iter) {
      auto *BB = *Iter;
      for (auto &I : *BB)
        maybeDFS(&I);
    }

    cleanup();
  }

private:
  struct DFSInfo {
    SILNode *Node;
    int DFSNum;
    int LowNum;

    DFSInfo(SILNode *node, int num) : Node(node), DFSNum(num), LowNum(num) {}
  };

  SILFunction &F;
  int CurrentNum = 0;

  toolchain::DenseSet<SILNode *> Visited;
  toolchain::SetVector<SILNode *> DFSStack;
  typedef toolchain::DenseMap<SILNode *, std::unique_ptr<DFSInfo>> ValueInfoMapType;
  ValueInfoMapType ValueInfoMap;

  void cleanup() {
    Visited.clear();
    DFSStack.clear();
    ValueInfoMap.clear();
    CurrentNum = 0;
  }

  DFSInfo &addDFSInfo(SILNode *node) {
    auto insertion = ValueInfoMap.try_emplace(node,
                                              new DFSInfo(node, CurrentNum++));
    assert(insertion.second && "Cannot add DFS info more than once!");
    return *insertion.first->second;
  }

  DFSInfo &getDFSInfo(SILNode *node) {
    auto it = ValueInfoMap.find(node);
    assert(it != ValueInfoMap.end() &&
           "Expected to find value in DFS info map!");

    return *it->second;
  }

  void getArgsForTerminator(TermInst *Term, SILBasicBlock *SuccBB, int Index,
                            toolchain::SmallVectorImpl<SILValue> &Operands) {
    switch (Term->getTermKind()) {
    case TermKind::BranchInst:
      return Operands.push_back(cast<BranchInst>(Term)->getArg(Index));

    case TermKind::CondBranchInst: {
      auto *CBI = cast<CondBranchInst>(Term);
      if (SuccBB == CBI->getTrueBB())
        return Operands.push_back(CBI->getTrueArgs()[Index]);
      assert(SuccBB == CBI->getFalseBB() &&
             "Block is not a successor of terminator!");
      Operands.push_back(CBI->getFalseArgs()[Index]);
      return;
    }
        
    case TermKind::AwaitAsyncContinuationInst: {
      auto *AACI = cast<AwaitAsyncContinuationInst>(Term);
      Operands.push_back(AACI->getOperand());
      return;
    }

    case TermKind::SwitchEnumInst:
    case TermKind::SwitchEnumAddrInst:
    case TermKind::CheckedCastBranchInst:
    case TermKind::CheckedCastAddrBranchInst:
    case TermKind::DynamicMethodBranchInst:
      assert(Index == 0 && "Expected argument index to always be zero!");
      return Operands.push_back(Term->getOperand(0));

    case TermKind::UnreachableInst:
    case TermKind::ReturnInst:
    case TermKind::SwitchValueInst:
    case TermKind::ThrowInst:
    case TermKind::ThrowAddrInst:
    case TermKind::UnwindInst:
      toolchain_unreachable("Did not expect terminator that does not have args!");

    case TermKind::YieldInst:
      for (auto &O : cast<YieldInst>(Term)->getAllOperands())
        Operands.push_back(O.get());
      return;

    case TermKind::TryApplyInst:
      for (auto &O : cast<TryApplyInst>(Term)->getAllOperands())
        Operands.push_back(O.get());
      return;
    }
  }

  void collectOperandsForUser(SILNode *node,
                              toolchain::SmallVectorImpl<SILValue> &Operands) {
    if (auto *I = dyn_cast<SILInstruction>(node)) {
      for (auto &O : I->getAllOperands())
        Operands.push_back(O.get());
      return;
    }

    if (auto *A = dyn_cast<SILArgument>(node)) {
      auto *BB = A->getParent();
      auto Index = A->getIndex();

      for (auto *Pred : BB->getPredecessorBlocks())
        getArgsForTerminator(Pred->getTerminator(), BB, Index, Operands);
      return;
    }
  }

  void maybeDFS(SILInstruction *inst) {
    (void) maybeDFSCanonicalNode(inst->asSILNode());
  }

  /// Continue a DFS from the given node, finding the strongly
  /// component that User is a part of, calling visit() with that SCC,
  /// and returning the DFSInfo for the node.
  /// But if we've already visited the node, just return null.
  DFSInfo *maybeDFSCanonicalNode(SILNode *node) {
    if (!Visited.insert(node).second)
      return nullptr;

    DFSStack.insert(node);

    auto &nodeInfo = addDFSInfo(node);

    toolchain::SmallVector<SILValue, 4> operands;
    collectOperandsForUser(node, operands);

    // Visit each unvisited operand, updating the lowest DFS number we've seen
    // reachable in User's SCC.
    for (SILValue operandValue : operands) {
      SILNode *operandNode = operandValue;
      if (auto operandNodeInfo = maybeDFSCanonicalNode(operandNode)) {
        nodeInfo.LowNum = std::min(nodeInfo.LowNum, operandNodeInfo->LowNum);
      } else if (DFSStack.count(operandNode)) {
        auto operandNodeInfo = &getDFSInfo(operandNode);
        nodeInfo.LowNum = std::min(nodeInfo.LowNum, operandNodeInfo->DFSNum);
      }
    }

    // If User is the head of its own SCC, pop that SCC off the DFS stack.
    if (nodeInfo.DFSNum == nodeInfo.LowNum) {
      toolchain::SmallVector<SILNode *, 4> SCC;
      SILNode *poppedNode;
      do {
        poppedNode = DFSStack.pop_back_val();
        SCC.push_back(poppedNode);
      } while (poppedNode != node);

      asImpl().visit(SCC);
    }

    return &nodeInfo;
  }
};

}

#endif
