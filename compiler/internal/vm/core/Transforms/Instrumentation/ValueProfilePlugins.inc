//=== ValueProfilePlugins.inc - set of plugins used by ValueProfileCollector =//
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
// This file contains a set of plugin classes used in ValueProfileCollectorImpl.
// Each plugin is responsible for collecting Value Profiling candidates for a
// particular optimization.
// Each plugin must satisfy the interface described in ValueProfileCollector.cpp
//
//===----------------------------------------------------------------------===//

#include "ValueProfileCollector.h"
#include "vm/core/Analysis/IndirectCallVisitor.h"
#include "vm/core/Analysis/TargetLibraryInfo.h"
#include "vm/core/IR/InstVisitor.h"

using namespace vm::core;
using CandidateInfo = ValueProfileCollector::CandidateInfo;

namespace vm::core {
extern cl::opt<bool> MemOPOptMemcmpBcmp;
} // end namespace vm::core

///--------------------------- MemIntrinsicPlugin ------------------------------
class MemIntrinsicPlugin : public InstVisitor<MemIntrinsicPlugin> {
  Function &F;
  TargetLibraryInfo &TLI;
  std::vector<CandidateInfo> *Candidates;

public:
  static constexpr InstrProfValueKind Kind = IPVK_MemOPSize;

  MemIntrinsicPlugin(Function &Fn, TargetLibraryInfo &TLI)
      : F(Fn), TLI(TLI), Candidates(nullptr) {}

  void run(std::vector<CandidateInfo> &Cs) {
    Candidates = &Cs;
    visit(F);
    Candidates = nullptr;
  }
  void visitMemIntrinsic(MemIntrinsic &MI) {
    Value *Length = MI.getLength();
    // Not instrument constant length calls.
    if (isa<ConstantInt>(Length))
      return;

    Instruction *InsertPt = &MI;
    Instruction *AnnotatedInst = &MI;
    Candidates->emplace_back(CandidateInfo{Length, InsertPt, AnnotatedInst});
  }
  void visitCallInst(CallInst &CI) {
    if (!MemOPOptMemcmpBcmp)
      return;
    auto *F = CI.getCalledFunction();
    if (!F)
      return;
    LibFunc Func;
    if (TLI.getLibFunc(CI, Func) &&
        (Func == LibFunc_memcmp || Func == LibFunc_bcmp)) {
      Value *Length = CI.getArgOperand(2);
      // Not instrument constant length calls.
      if (isa<ConstantInt>(Length))
        return;
      Instruction *InsertPt = &CI;
      Instruction *AnnotatedInst = &CI;
      Candidates->emplace_back(CandidateInfo{Length, InsertPt, AnnotatedInst});
    }
  }
};

///------------------------ IndirectCallPromotionPlugin ------------------------
class IndirectCallPromotionPlugin {
  Function &F;

public:
  static constexpr InstrProfValueKind Kind = IPVK_IndirectCallTarget;

  IndirectCallPromotionPlugin(Function &Fn, TargetLibraryInfo &TLI) : F(Fn) {}

  void run(std::vector<CandidateInfo> &Candidates) {
    std::vector<CallBase *> Result = findIndirectCalls(F);
    for (Instruction *I : Result) {
      Value *Callee = cast<CallBase>(I)->getCalledOperand();
      Instruction *InsertPt = I;
      Instruction *AnnotatedInst = I;
      Candidates.emplace_back(CandidateInfo{Callee, InsertPt, AnnotatedInst});
    }
  }
};

///--------------------- VirtualTableValueProfilingPlugin --------------------
class VTableProfilingPlugin {
  Function &F;

public:
  static constexpr InstrProfValueKind Kind = IPVK_VTableTarget;

  VTableProfilingPlugin(Function &Fn, TargetLibraryInfo &TLI) : F(Fn) {}

  void run(std::vector<CandidateInfo> &Candidates) {
    std::vector<Instruction *> Result = findVTableAddrs(F);
    for (Instruction *I : Result) {
      Instruction *InsertPt = I->getNextNode();
      // When finding an insertion point, keep PHI and EH pad instructions
      // before vp intrinsics. This is similar to
      // `BasicBlock::getFirstInsertionPt`.
      while (InsertPt && (dyn_cast<PHINode>(InsertPt) || InsertPt->isEHPad()))
        InsertPt = InsertPt->getNextNode();
      // Skip instrumentating the value if InsertPt is the last instruction.
      // FIXME: Set InsertPt to the end of basic block to instrument the value
      // if InsertPt is the last instruction.
      if (InsertPt == nullptr)
        continue;

      Instruction *AnnotatedInst = I;
      Candidates.emplace_back(CandidateInfo{I, InsertPt, AnnotatedInst});
    }
  }
};

///----------------------- Registration of the plugins -------------------------
/// For now, registering a plugin with the ValueProfileCollector is done by
/// adding the plugin type to the VP_PLUGIN_LIST macro.
#define VP_PLUGIN_LIST                                                         \
  MemIntrinsicPlugin, IndirectCallPromotionPlugin, VTableProfilingPlugin
