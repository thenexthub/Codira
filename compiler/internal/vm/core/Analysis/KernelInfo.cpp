//===- KernelInfo.cpp - Kernel Analysis -----------------------------------===//
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
// This file defines the KernelInfoPrinter class used to emit remarks about
// function properties from a GPU kernel.
//
//===----------------------------------------------------------------------===//

#include "vm/core/Analysis/KernelInfo.h"
#include "vm/core/ADT/SmallString.h"
#include "vm/core/ADT/StringExtras.h"
#include "vm/core/Analysis/OptimizationRemarkEmitter.h"
#include "vm/core/Analysis/TargetTransformInfo.h"
#include "vm/core/IR/DebugInfo.h"
#include "vm/core/IR/Dominators.h"
#include "vm/core/IR/Instructions.h"
#include "vm/core/IR/Metadata.h"
#include "vm/core/IR/Module.h"
#include "vm/core/IR/PassManager.h"

using namespace vm::core;

#define DEBUG_TYPE "kernel-info"

namespace {

/// Data structure holding function info for kernels.
class KernelInfo {
  void updateForBB(const BasicBlock &BB, OptimizationRemarkEmitter &ORE);

public:
  static void emitKernelInfo(Function &F, FunctionAnalysisManager &FAM,
                             TargetMachine *TM);

  /// Whether the function has external linkage and is not a kernel function.
  bool ExternalNotKernel = false;

  /// Launch bounds.
  SmallVector<std::pair<StringRef, int64_t>> LaunchBounds;

  /// The number of alloca instructions inside the function, the number of those
  /// with allocation sizes that cannot be determined at compile time, and the
  /// sum of the sizes that can be.
  ///
  /// With the current implementation for at least some GPU archs,
  /// AllocasDyn > 0 might not be possible, but we report AllocasDyn anyway in
  /// case the implementation changes.
  int64_t Allocas = 0;
  int64_t AllocasDyn = 0;
  int64_t AllocasStaticSizeSum = 0;

  /// Number of direct/indirect calls (anything derived from CallBase).
  int64_t DirectCalls = 0;
  int64_t IndirectCalls = 0;

  /// Number of direct calls made from this function to other functions
  /// defined in this module.
  int64_t DirectCallsToDefinedFunctions = 0;

  /// Number of direct calls to inline assembly.
  int64_t InlineAssemblyCalls = 0;

  /// Number of calls of type InvokeInst.
  int64_t Invokes = 0;

  /// Target-specific flat address space.
  unsigned FlatAddrspace;

  /// Number of flat address space memory accesses (via load, store, etc.).
  int64_t FlatAddrspaceAccesses = 0;
};

} // end anonymous namespace

static void identifyCallee(OptimizationRemark &R, const Module *M,
                           const Value *V, StringRef Kind = "") {
  SmallString<100> Name; // might be function name or asm expression
  if (const Function *F = dyn_cast<Function>(V)) {
    if (auto *SubProgram = F->getSubprogram()) {
      if (SubProgram->isArtificial())
        R << "artificial ";
      Name = SubProgram->getName();
    }
  }
  if (Name.empty()) {
    raw_svector_ostream OS(Name);
    V->printAsOperand(OS, /*PrintType=*/false, M);
  }
  if (!Kind.empty())
    R << Kind << " ";
  R << "'" << Name << "'";
}

static void identifyFunction(OptimizationRemark &R, const Function &F) {
  identifyCallee(R, F.getParent(), &F, "function");
}

static void remarkAlloca(OptimizationRemarkEmitter &ORE, const Function &Caller,
                         const AllocaInst &Alloca,
                         TypeSize::ScalarTy StaticSize) {
  ORE.emit([&] {
    StringRef DbgName;
    DebugLoc Loc;
    bool Artificial = false;
    auto DVRs = findDVRDeclares(&const_cast<AllocaInst &>(Alloca));
    if (!DVRs.empty()) {
      const DbgVariableRecord &DVR = **DVRs.begin();
      DbgName = DVR.getVariable()->getName();
      Loc = DVR.getDebugLoc();
      Artificial = DVR.Variable->isArtificial();
    }
    OptimizationRemark R(DEBUG_TYPE, "Alloca", DiagnosticLocation(Loc),
                         Alloca.getParent());
    R << "in ";
    identifyFunction(R, Caller);
    R << ", ";
    if (Artificial)
      R << "artificial ";
    SmallString<20> ValName;
    raw_svector_ostream OS(ValName);
    Alloca.printAsOperand(OS, /*PrintType=*/false, Caller.getParent());
    R << "alloca ('" << ValName << "') ";
    if (!DbgName.empty())
      R << "for '" << DbgName << "' ";
    else
      R << "without debug info ";
    R << "with ";
    if (StaticSize)
      R << "static size of " << itostr(StaticSize) << " bytes";
    else
      R << "dynamic size";
    return R;
  });
}

static void remarkCall(OptimizationRemarkEmitter &ORE, const Function &Caller,
                       const CallBase &Call, StringRef CallKind,
                       StringRef RemarkKind) {
  ORE.emit([&] {
    OptimizationRemark R(DEBUG_TYPE, RemarkKind, &Call);
    R << "in ";
    identifyFunction(R, Caller);
    R << ", " << CallKind << ", callee is ";
    identifyCallee(R, Caller.getParent(), Call.getCalledOperand());
    return R;
  });
}

static void remarkFlatAddrspaceAccess(OptimizationRemarkEmitter &ORE,
                                      const Function &Caller,
                                      const Instruction &Inst) {
  ORE.emit([&] {
    OptimizationRemark R(DEBUG_TYPE, "FlatAddrspaceAccess", &Inst);
    R << "in ";
    identifyFunction(R, Caller);
    if (const IntrinsicInst *II = dyn_cast<IntrinsicInst>(&Inst)) {
      R << ", '" << II->getCalledFunction()->getName() << "' call";
    } else {
      R << ", '" << Inst.getOpcodeName() << "' instruction";
    }
    if (!Inst.getType()->isVoidTy()) {
      SmallString<20> Name;
      raw_svector_ostream OS(Name);
      Inst.printAsOperand(OS, /*PrintType=*/false, Caller.getParent());
      R << " ('" << Name << "')";
    }
    R << " accesses memory in flat address space";
    return R;
  });
}

void KernelInfo::updateForBB(const BasicBlock &BB,
                             OptimizationRemarkEmitter &ORE) {
  const Function &F = *BB.getParent();
  const Module &M = *F.getParent();
  const DataLayout &DL = M.getDataLayout();
  for (const Instruction &I : BB.instructionsWithoutDebug()) {
    if (const AllocaInst *Alloca = dyn_cast<AllocaInst>(&I)) {
      ++Allocas;
      TypeSize::ScalarTy StaticSize = 0;
      if (std::optional<TypeSize> Size = Alloca->getAllocationSize(DL)) {
        StaticSize = Size->getFixedValue();
        assert(StaticSize <=
               (TypeSize::ScalarTy)std::numeric_limits<int64_t>::max());
        AllocasStaticSizeSum += StaticSize;
      } else {
        ++AllocasDyn;
      }
      remarkAlloca(ORE, F, *Alloca, StaticSize);
    } else if (const CallBase *Call = dyn_cast<CallBase>(&I)) {
      SmallString<40> CallKind;
      SmallString<40> RemarkKind;
      if (Call->isIndirectCall()) {
        ++IndirectCalls;
        CallKind += "indirect";
        RemarkKind += "Indirect";
      } else {
        ++DirectCalls;
        CallKind += "direct";
        RemarkKind += "Direct";
      }
      if (isa<InvokeInst>(Call)) {
        ++Invokes;
        CallKind += " invoke";
        RemarkKind += "Invoke";
      } else {
        CallKind += " call";
        RemarkKind += "Call";
      }
      if (!Call->isIndirectCall()) {
        if (const Function *Callee = Call->getCalledFunction()) {
          if (!Callee->isIntrinsic() && !Callee->isDeclaration()) {
            ++DirectCallsToDefinedFunctions;
            CallKind += " to defined function";
            RemarkKind += "ToDefinedFunction";
          }
        } else if (Call->isInlineAsm()) {
          ++InlineAssemblyCalls;
          CallKind += " to inline assembly";
          RemarkKind += "ToInlineAssembly";
        }
      }
      remarkCall(ORE, F, *Call, CallKind, RemarkKind);
      if (const AnyMemIntrinsic *MI = dyn_cast<AnyMemIntrinsic>(Call)) {
        if (MI->getDestAddressSpace() == FlatAddrspace) {
          ++FlatAddrspaceAccesses;
          remarkFlatAddrspaceAccess(ORE, F, I);
        } else if (const AnyMemTransferInst *MT =
                       dyn_cast<AnyMemTransferInst>(MI)) {
          if (MT->getSourceAddressSpace() == FlatAddrspace) {
            ++FlatAddrspaceAccesses;
            remarkFlatAddrspaceAccess(ORE, F, I);
          }
        }
      }
    } else if (const LoadInst *Load = dyn_cast<LoadInst>(&I)) {
      if (Load->getPointerAddressSpace() == FlatAddrspace) {
        ++FlatAddrspaceAccesses;
        remarkFlatAddrspaceAccess(ORE, F, I);
      }
    } else if (const StoreInst *Store = dyn_cast<StoreInst>(&I)) {
      if (Store->getPointerAddressSpace() == FlatAddrspace) {
        ++FlatAddrspaceAccesses;
        remarkFlatAddrspaceAccess(ORE, F, I);
      }
    } else if (const AtomicRMWInst *At = dyn_cast<AtomicRMWInst>(&I)) {
      if (At->getPointerAddressSpace() == FlatAddrspace) {
        ++FlatAddrspaceAccesses;
        remarkFlatAddrspaceAccess(ORE, F, I);
      }
    } else if (const AtomicCmpXchgInst *At = dyn_cast<AtomicCmpXchgInst>(&I)) {
      if (At->getPointerAddressSpace() == FlatAddrspace) {
        ++FlatAddrspaceAccesses;
        remarkFlatAddrspaceAccess(ORE, F, I);
      }
    }
  }
}

static void remarkProperty(OptimizationRemarkEmitter &ORE, const Function &F,
                           StringRef Name, int64_t Value) {
  ORE.emit([&] {
    OptimizationRemark R(DEBUG_TYPE, Name, &F);
    R << "in ";
    identifyFunction(R, F);
    R << ", " << Name << " = " << itostr(Value);
    return R;
  });
}

static std::optional<int64_t> parseFnAttrAsInteger(Function &F,
                                                   StringRef Name) {
  if (!F.hasFnAttribute(Name))
    return std::nullopt;
  return F.getFnAttributeAsParsedInteger(Name);
}

void KernelInfo::emitKernelInfo(Function &F, FunctionAnalysisManager &FAM,
                                TargetMachine *TM) {
  KernelInfo KI;
  TargetTransformInfo &TheTTI = FAM.getResult<TargetIRAnalysis>(F);
  KI.FlatAddrspace = TheTTI.getFlatAddressSpace();

  // Record function properties.
  KI.ExternalNotKernel = F.hasExternalLinkage() && !F.hasKernelCallingConv();
  for (StringRef Name : {"omp_target_num_teams", "omp_target_thread_limit"}) {
    if (auto Val = parseFnAttrAsInteger(F, Name))
      KI.LaunchBounds.push_back({Name, *Val});
  }
  TheTTI.collectKernelLaunchBounds(F, KI.LaunchBounds);

  auto &ORE = FAM.getResult<OptimizationRemarkEmitterAnalysis>(F);
  for (const auto &BB : F)
    KI.updateForBB(BB, ORE);

#define REMARK_PROPERTY(PROP_NAME)                                             \
  remarkProperty(ORE, F, #PROP_NAME, KI.PROP_NAME)
  REMARK_PROPERTY(ExternalNotKernel);
  for (auto LB : KI.LaunchBounds)
    remarkProperty(ORE, F, LB.first, LB.second);
  REMARK_PROPERTY(Allocas);
  REMARK_PROPERTY(AllocasStaticSizeSum);
  REMARK_PROPERTY(AllocasDyn);
  REMARK_PROPERTY(DirectCalls);
  REMARK_PROPERTY(IndirectCalls);
  REMARK_PROPERTY(DirectCallsToDefinedFunctions);
  REMARK_PROPERTY(InlineAssemblyCalls);
  REMARK_PROPERTY(Invokes);
  REMARK_PROPERTY(FlatAddrspaceAccesses);
#undef REMARK_PROPERTY
}

PreservedAnalyses KernelInfoPrinter::run(Function &F,
                                         FunctionAnalysisManager &AM) {
  // Skip it if remarks are not enabled as it will do nothing useful.
  if (F.getContext().getDiagHandlerPtr()->isPassedOptRemarkEnabled(DEBUG_TYPE))
    KernelInfo::emitKernelInfo(F, AM, TM);
  return PreservedAnalyses::all();
}
