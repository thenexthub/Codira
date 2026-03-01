//===-- AMDGPULowerKernelArguments.cpp ------------------------------------------===//
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
/// \file This pass replaces accesses to kernel arguments with loads from
/// offsets from the kernarg base pointer.
//
//===----------------------------------------------------------------------===//

#include "AMDGPU.h"
#include "GCNSubtarget.h"
#include "vm/core/Analysis/ValueTracking.h"
#include "vm/core/CodeGen/TargetPassConfig.h"
#include "vm/core/IR/Attributes.h"
#include "vm/core/IR/IRBuilder.h"
#include "vm/core/IR/IntrinsicsAMDGPU.h"
#include "vm/core/IR/MDBuilder.h"
#include "vm/core/Target/TargetMachine.h"

#define DEBUG_TYPE "amdgpu-lower-kernel-arguments"

using namespace vm::core;

namespace {

class AMDGPULowerKernelArguments : public FunctionPass {
public:
  static char ID;

  AMDGPULowerKernelArguments() : FunctionPass(ID) {}

  bool runOnFunction(Function &F) override;

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<TargetPassConfig>();
    AU.setPreservesAll();
 }
};

} // end anonymous namespace

// skip allocas
static BasicBlock::iterator getInsertPt(BasicBlock &BB) {
  BasicBlock::iterator InsPt = BB.getFirstInsertionPt();
  for (BasicBlock::iterator E = BB.end(); InsPt != E; ++InsPt) {
    AllocaInst *AI = dyn_cast<AllocaInst>(&*InsPt);

    // If this is a dynamic alloca, the value may depend on the loaded kernargs,
    // so loads will need to be inserted before it.
    if (!AI || !AI->isStaticAlloca())
      break;
  }

  return InsPt;
}

static bool lowerKernelArguments(Function &F, const TargetMachine &TM) {
  CallingConv::ID CC = F.getCallingConv();
  if (CC != CallingConv::AMDGPU_KERNEL || F.arg_empty())
    return false;

  const GCNSubtarget &ST = TM.getSubtarget<GCNSubtarget>(F);
  LLVMContext &Ctx = F.getContext();
  const DataLayout &DL = F.getDataLayout();
  BasicBlock &EntryBlock = *F.begin();
  IRBuilder<> Builder(&EntryBlock, getInsertPt(EntryBlock));

  const Align KernArgBaseAlign(16); // FIXME: Increase if necessary
  const uint64_t BaseOffset = ST.getExplicitKernelArgOffset();

  Align MaxAlign;
  // FIXME: Alignment is broken with explicit arg offset.;
  const uint64_t TotalKernArgSize = ST.getKernArgSegmentSize(F, MaxAlign);
  if (TotalKernArgSize == 0)
    return false;

  CallInst *KernArgSegment =
      Builder.CreateIntrinsic(Intrinsic::amdgcn_kernarg_segment_ptr, {},
                              nullptr, F.getName() + ".kernarg.segment");
  KernArgSegment->addRetAttr(Attribute::NonNull);
  KernArgSegment->addRetAttr(
      Attribute::getWithDereferenceableBytes(Ctx, TotalKernArgSize));

  uint64_t ExplicitArgOffset = 0;
  for (Argument &Arg : F.args()) {
    const bool IsByRef = Arg.hasByRefAttr();
    Type *ArgTy = IsByRef ? Arg.getParamByRefType() : Arg.getType();
    MaybeAlign ParamAlign = IsByRef ? Arg.getParamAlign() : std::nullopt;
    Align ABITypeAlign = DL.getValueOrABITypeAlignment(ParamAlign, ArgTy);

    uint64_t Size = DL.getTypeSizeInBits(ArgTy);
    uint64_t AllocSize = DL.getTypeAllocSize(ArgTy);

    uint64_t EltOffset = alignTo(ExplicitArgOffset, ABITypeAlign) + BaseOffset;
    ExplicitArgOffset = alignTo(ExplicitArgOffset, ABITypeAlign) + AllocSize;

    // Skip inreg arguments which should be preloaded.
    if (Arg.use_empty() || Arg.hasInRegAttr())
      continue;

    // If this is byval, the loads are already explicit in the function. We just
    // need to rewrite the pointer values.
    if (IsByRef) {
      Value *ArgOffsetPtr = Builder.CreateConstInBoundsGEP1_64(
          Builder.getInt8Ty(), KernArgSegment, EltOffset,
          Arg.getName() + ".byval.kernarg.offset");

      Value *CastOffsetPtr =
          Builder.CreateAddrSpaceCast(ArgOffsetPtr, Arg.getType());
      Arg.replaceAllUsesWith(CastOffsetPtr);
      continue;
    }

    if (PointerType *PT = dyn_cast<PointerType>(ArgTy)) {
      // FIXME: Hack. We rely on AssertZext to be able to fold DS addressing
      // modes on SI to know the high bits are 0 so pointer adds don't wrap. We
      // can't represent this with range metadata because it's only allowed for
      // integer types.
      if ((PT->getAddressSpace() == AMDGPUAS::LOCAL_ADDRESS ||
           PT->getAddressSpace() == AMDGPUAS::REGION_ADDRESS) &&
          !ST.hasUsableDSOffset())
        continue;

      // FIXME: We can replace this with equivalent alias.scope/noalias
      // metadata, but this appears to be a lot of work.
      if (Arg.hasNoAliasAttr())
        continue;
    }

    auto *VT = dyn_cast<FixedVectorType>(ArgTy);
    bool IsV3 = VT && VT->getNumElements() == 3;
    bool DoShiftOpt = Size < 32 && !ArgTy->isAggregateType();

    VectorType *V4Ty = nullptr;

    int64_t AlignDownOffset = alignDown(EltOffset, 4);
    int64_t OffsetDiff = EltOffset - AlignDownOffset;
    Align AdjustedAlign = commonAlignment(
        KernArgBaseAlign, DoShiftOpt ? AlignDownOffset : EltOffset);

    Value *ArgPtr;
    Type *AdjustedArgTy;
    if (DoShiftOpt) { // FIXME: Handle aggregate types
      // Since we don't have sub-dword scalar loads, avoid doing an extload by
      // loading earlier than the argument address, and extracting the relevant
      // bits.
      // TODO: Update this for GFX12 which does have scalar sub-dword loads.
      //
      // Additionally widen any sub-dword load to i32 even if suitably aligned,
      // so that CSE between different argument loads works easily.
      ArgPtr = Builder.CreateConstInBoundsGEP1_64(
          Builder.getInt8Ty(), KernArgSegment, AlignDownOffset,
          Arg.getName() + ".kernarg.offset.align.down");
      AdjustedArgTy = Builder.getInt32Ty();
    } else {
      ArgPtr = Builder.CreateConstInBoundsGEP1_64(
          Builder.getInt8Ty(), KernArgSegment, EltOffset,
          Arg.getName() + ".kernarg.offset");
      AdjustedArgTy = ArgTy;
    }

    if (IsV3 && Size >= 32) {
      V4Ty = FixedVectorType::get(VT->getElementType(), 4);
      // Use the hack that clang uses to avoid SelectionDAG ruining v3 loads
      AdjustedArgTy = V4Ty;
    }

    LoadInst *Load =
        Builder.CreateAlignedLoad(AdjustedArgTy, ArgPtr, AdjustedAlign);
    Load->setMetadata(LLVMContext::MD_invariant_load, MDNode::get(Ctx, {}));

    MDBuilder MDB(Ctx);

    if (Arg.hasAttribute(Attribute::NoUndef))
      Load->setMetadata(LLVMContext::MD_noundef, MDNode::get(Ctx, {}));

    if (Arg.hasAttribute(Attribute::Range)) {
      const ConstantRange &Range =
          Arg.getAttribute(Attribute::Range).getValueAsConstantRange();
      Load->setMetadata(LLVMContext::MD_range,
                        MDB.createRange(Range.getLower(), Range.getUpper()));
    }

    if (isa<PointerType>(ArgTy)) {
      if (Arg.hasNonNullAttr())
        Load->setMetadata(LLVMContext::MD_nonnull, MDNode::get(Ctx, {}));

      uint64_t DerefBytes = Arg.getDereferenceableBytes();
      if (DerefBytes != 0) {
        Load->setMetadata(
          LLVMContext::MD_dereferenceable,
          MDNode::get(Ctx,
                      MDB.createConstant(
                        ConstantInt::get(Builder.getInt64Ty(), DerefBytes))));
      }

      uint64_t DerefOrNullBytes = Arg.getDereferenceableOrNullBytes();
      if (DerefOrNullBytes != 0) {
        Load->setMetadata(
          LLVMContext::MD_dereferenceable_or_null,
          MDNode::get(Ctx,
                      MDB.createConstant(ConstantInt::get(Builder.getInt64Ty(),
                                                          DerefOrNullBytes))));
      }

      if (MaybeAlign ParamAlign = Arg.getParamAlign()) {
        Load->setMetadata(
            LLVMContext::MD_align,
            MDNode::get(Ctx, MDB.createConstant(ConstantInt::get(
                                 Builder.getInt64Ty(), ParamAlign->value()))));
      }
    }

    // TODO: Convert noalias arg to !noalias

    if (DoShiftOpt) {
      Value *ExtractBits = OffsetDiff == 0 ?
        Load : Builder.CreateLShr(Load, OffsetDiff * 8);

      IntegerType *ArgIntTy = Builder.getIntNTy(Size);
      Value *Trunc = Builder.CreateTrunc(ExtractBits, ArgIntTy);
      Value *NewVal = Builder.CreateBitCast(Trunc, ArgTy,
                                            Arg.getName() + ".load");
      Arg.replaceAllUsesWith(NewVal);
    } else if (IsV3) {
      Value *Shuf = Builder.CreateShuffleVector(Load, ArrayRef<int>{0, 1, 2},
                                                Arg.getName() + ".load");
      Arg.replaceAllUsesWith(Shuf);
    } else {
      Load->setName(Arg.getName() + ".load");
      Arg.replaceAllUsesWith(Load);
    }
  }

  KernArgSegment->addRetAttr(
      Attribute::getWithAlignment(Ctx, std::max(KernArgBaseAlign, MaxAlign)));

  return true;
}

bool AMDGPULowerKernelArguments::runOnFunction(Function &F) {
  auto &TPC = getAnalysis<TargetPassConfig>();
  const TargetMachine &TM = TPC.getTM<TargetMachine>();
  return lowerKernelArguments(F, TM);
}

INITIALIZE_PASS_BEGIN(AMDGPULowerKernelArguments, DEBUG_TYPE,
                      "AMDGPU Lower Kernel Arguments", false, false)
INITIALIZE_PASS_END(AMDGPULowerKernelArguments, DEBUG_TYPE, "AMDGPU Lower Kernel Arguments",
                    false, false)

char AMDGPULowerKernelArguments::ID = 0;

FunctionPass *toolchain::createAMDGPULowerKernelArgumentsPass() {
  return new AMDGPULowerKernelArguments();
}

PreservedAnalyses
AMDGPULowerKernelArgumentsPass::run(Function &F, FunctionAnalysisManager &AM) {
  bool Changed = lowerKernelArguments(F, TM);
  if (Changed) {
    // TODO: Preserves a lot more.
    PreservedAnalyses PA;
    PA.preserveSet<CFGAnalyses>();
    return PA;
  }

  return PreservedAnalyses::all();
}
