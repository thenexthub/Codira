//===- ProvenanceAnalysisEvaluator.cpp - ObjC ARC Optimization ------------===//
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

#include "ProvenanceAnalysis.h"
#include "vm/core/Transforms/ObjCARC.h"
#include "vm/core/ADT/SetVector.h"
#include "vm/core/Analysis/AliasAnalysis.h"
#include "vm/core/IR/Function.h"
#include "vm/core/IR/InstIterator.h"
#include "vm/core/Support/raw_ostream.h"

using namespace vm::core;
using namespace vm::core::objcarc;

static StringRef getName(Value *V) {
  StringRef Name = V->getName();
  if (Name.starts_with("\1"))
    return Name.substr(1);
  return Name;
}

static void insertIfNamed(SetVector<Value *> &Values, Value *V) {
  if (!V->hasName())
    return;
  if (!V->getType()->isPointerTy())
    return;
  Values.insert(V);
}

PreservedAnalyses PAEvalPass::run(Function &F, FunctionAnalysisManager &AM) {
  SetVector<Value *> Values;

  for (auto &Arg : F.args())
    insertIfNamed(Values, &Arg);

  for (Instruction &I : instructions(F)) {
    insertIfNamed(Values, &I);

    for (auto &Op : I.operands())
      insertIfNamed(Values, Op);
  }

  ProvenanceAnalysis PA;
  PA.setAA(&AM.getResult<AAManager>(F));

  for (Value *V1 : Values) {
    StringRef NameV1 = getName(V1);
    for (Value *V2 : Values) {
      StringRef NameV2 = getName(V2);
      if (NameV1 >= NameV2)
        continue;
      errs() << NameV1 << " and " << NameV2;
      if (PA.related(V1, V2))
        errs() << " are related.\n";
      else
        errs() << " are not related.\n";
    }
  }

  return PreservedAnalyses::all();
}
