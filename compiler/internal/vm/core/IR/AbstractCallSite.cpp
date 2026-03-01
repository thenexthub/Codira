//===-- AbstractCallSite.cpp - Implementation of abstract call sites ------===//
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
// This file implements abstract call sites which unify the interface for
// direct, indirect, and callback call sites.
//
// For more information see:
// https://toolchain.org/devmtg/2018-10/talk-abstracts.html#talk20
//
//===----------------------------------------------------------------------===//

#include "vm/core/IR/AbstractCallSite.h"
#include "vm/core/ADT/Statistic.h"

using namespace vm::core;

#define DEBUG_TYPE "abstract-call-sites"

STATISTIC(NumCallbackCallSites, "Number of callback call sites created");
STATISTIC(NumDirectAbstractCallSites,
          "Number of direct abstract call sites created");
STATISTIC(NumInvalidAbstractCallSitesUnknownUse,
          "Number of invalid abstract call sites created (unknown use)");
STATISTIC(NumInvalidAbstractCallSitesUnknownCallee,
          "Number of invalid abstract call sites created (unknown callee)");
STATISTIC(NumInvalidAbstractCallSitesNoCallback,
          "Number of invalid abstract call sites created (no callback)");

void AbstractCallSite::getCallbackUses(
    const CallBase &CB, SmallVectorImpl<const Use *> &CallbackUses) {
  const Function *Callee = CB.getCalledFunction();
  if (!Callee)
    return;

  MDNode *CallbackMD = Callee->getMetadata(LLVMContext::MD_callback);
  if (!CallbackMD)
    return;

  for (const MDOperand &Op : CallbackMD->operands()) {
    MDNode *OpMD = cast<MDNode>(Op.get());
    auto *CBCalleeIdxAsCM = cast<ConstantAsMetadata>(OpMD->getOperand(0));
    uint64_t CBCalleeIdx =
        cast<ConstantInt>(CBCalleeIdxAsCM->getValue())->getZExtValue();
    if (CBCalleeIdx < CB.arg_size())
      CallbackUses.push_back(CB.arg_begin() + CBCalleeIdx);
  }
}

/// Create an abstract call site from a use.
AbstractCallSite::AbstractCallSite(const Use *U)
    : CB(dyn_cast<CallBase>(U->getUser())) {

  // First handle unknown users.
  if (!CB) {

    // If the use is actually in a constant cast expression which itself
    // has only one use, we look through the constant cast expression.
    // This happens by updating the use @p U to the use of the constant
    // cast expression and afterwards re-initializing CB accordingly.
    if (ConstantExpr *CE = dyn_cast<ConstantExpr>(U->getUser()))
      if (CE->hasOneUse() && CE->isCast()) {
        U = &*CE->use_begin();
        CB = dyn_cast<CallBase>(U->getUser());
      }

    if (!CB) {
      NumInvalidAbstractCallSitesUnknownUse++;
      return;
    }
  }

  // Then handle direct or indirect calls. Thus, if U is the callee of the
  // call site CB it is not a callback and we are done.
  if (CB->isCallee(U)) {
    NumDirectAbstractCallSites++;
    return;
  }

  // If we cannot identify the broker function we cannot create a callback and
  // invalidate the abstract call site.
  Function *Callee = CB->getCalledFunction();
  if (!Callee) {
    NumInvalidAbstractCallSitesUnknownCallee++;
    CB = nullptr;
    return;
  }

  MDNode *CallbackMD = Callee->getMetadata(LLVMContext::MD_callback);
  if (!CallbackMD) {
    NumInvalidAbstractCallSitesNoCallback++;
    CB = nullptr;
    return;
  }

  unsigned UseIdx = CB->getArgOperandNo(U);
  MDNode *CallbackEncMD = nullptr;
  for (const MDOperand &Op : CallbackMD->operands()) {
    MDNode *OpMD = cast<MDNode>(Op.get());
    auto *CBCalleeIdxAsCM = cast<ConstantAsMetadata>(OpMD->getOperand(0));
    uint64_t CBCalleeIdx =
        cast<ConstantInt>(CBCalleeIdxAsCM->getValue())->getZExtValue();
    if (CBCalleeIdx != UseIdx)
      continue;
    CallbackEncMD = OpMD;
    break;
  }

  if (!CallbackEncMD) {
    NumInvalidAbstractCallSitesNoCallback++;
    CB = nullptr;
    return;
  }

  NumCallbackCallSites++;

  assert(CallbackEncMD->getNumOperands() >= 2 && "Incomplete !callback metadata");

  unsigned NumCallOperands = CB->arg_size();
  // Skip the var-arg flag at the end when reading the metadata.
  for (unsigned u = 0, e = CallbackEncMD->getNumOperands() - 1; u < e; u++) {
    Metadata *OpAsM = CallbackEncMD->getOperand(u).get();
    auto *OpAsCM = cast<ConstantAsMetadata>(OpAsM);
    assert(OpAsCM->getType()->isIntegerTy(64) &&
           "Malformed !callback metadata");

    int64_t Idx = cast<ConstantInt>(OpAsCM->getValue())->getSExtValue();
    assert(-1 <= Idx && Idx <= NumCallOperands &&
           "Out-of-bounds !callback metadata index");

    CI.ParameterEncoding.push_back(Idx);
  }

  if (!Callee->isVarArg())
    return;

  Metadata *VarArgFlagAsM =
      CallbackEncMD->getOperand(CallbackEncMD->getNumOperands() - 1).get();
  auto *VarArgFlagAsCM = cast<ConstantAsMetadata>(VarArgFlagAsM);
  assert(VarArgFlagAsCM->getType()->isIntegerTy(1) &&
         "Malformed !callback metadata var-arg flag");

  if (VarArgFlagAsCM->getValue()->isNullValue())
    return;

  // Add all variadic arguments at the end.
  for (unsigned u = Callee->arg_size(); u < NumCallOperands; u++)
    CI.ParameterEncoding.push_back(u);
}
