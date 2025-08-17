//===-------- RISCV.cpp - Emit LLVM Code for builtins ---------------------===//
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
// This contains code to emit Builtin calls as LLVM code.
//
//===----------------------------------------------------------------------===//

#include "CodeGenFunction.h"
#include "language/Core/Basic/TargetBuiltins.h"
#include "toolchain/IR/IntrinsicsRISCV.h"
#include "toolchain/TargetParser/RISCVISAInfo.h"
#include "toolchain/TargetParser/RISCVTargetParser.h"

using namespace language::Core;
using namespace CodeGen;
using namespace toolchain;

Value *CodeGenFunction::EmitRISCVCpuInit() {
  toolchain::FunctionType *FTy = toolchain::FunctionType::get(VoidTy, {VoidPtrTy}, false);
  toolchain::FunctionCallee Func =
      CGM.CreateRuntimeFunction(FTy, "__init_riscv_feature_bits");
  auto *CalleeGV = cast<toolchain::GlobalValue>(Func.getCallee());
  CalleeGV->setDSOLocal(true);
  CalleeGV->setDLLStorageClass(toolchain::GlobalValue::DefaultStorageClass);
  return Builder.CreateCall(Func, {toolchain::ConstantPointerNull::get(VoidPtrTy)});
}

Value *CodeGenFunction::EmitRISCVCpuSupports(const CallExpr *E) {

  const Expr *FeatureExpr = E->getArg(0)->IgnoreParenCasts();
  StringRef FeatureStr = cast<StringLiteral>(FeatureExpr)->getString();
  if (!getContext().getTargetInfo().validateCpuSupports(FeatureStr))
    return Builder.getFalse();

  return EmitRISCVCpuSupports(ArrayRef<StringRef>(FeatureStr));
}

static Value *loadRISCVFeatureBits(unsigned Index, CGBuilderTy &Builder,
                                   CodeGenModule &CGM) {
  toolchain::Type *Int32Ty = Builder.getInt32Ty();
  toolchain::Type *Int64Ty = Builder.getInt64Ty();
  toolchain::ArrayType *ArrayOfInt64Ty =
      toolchain::ArrayType::get(Int64Ty, toolchain::RISCVISAInfo::FeatureBitSize);
  toolchain::Type *StructTy = toolchain::StructType::get(Int32Ty, ArrayOfInt64Ty);
  toolchain::Constant *RISCVFeaturesBits =
      CGM.CreateRuntimeVariable(StructTy, "__riscv_feature_bits");
  cast<toolchain::GlobalValue>(RISCVFeaturesBits)->setDSOLocal(true);
  Value *IndexVal = toolchain::ConstantInt::get(Int32Ty, Index);
  toolchain::Value *GEPIndices[] = {Builder.getInt32(0), Builder.getInt32(1),
                               IndexVal};
  Value *Ptr =
      Builder.CreateInBoundsGEP(StructTy, RISCVFeaturesBits, GEPIndices);
  Value *FeaturesBit =
      Builder.CreateAlignedLoad(Int64Ty, Ptr, CharUnits::fromQuantity(8));
  return FeaturesBit;
}

Value *CodeGenFunction::EmitRISCVCpuSupports(ArrayRef<StringRef> FeaturesStrs) {
  const unsigned RISCVFeatureLength = toolchain::RISCVISAInfo::FeatureBitSize;
  uint64_t RequireBitMasks[RISCVFeatureLength] = {0};

  for (auto Feat : FeaturesStrs) {
    auto [GroupID, BitPos] = RISCVISAInfo::getRISCVFeaturesBitsInfo(Feat);

    // If there isn't BitPos for this feature, skip this version.
    // It also report the warning to user during compilation.
    if (BitPos == -1)
      return Builder.getFalse();

    RequireBitMasks[GroupID] |= (1ULL << BitPos);
  }

  Value *Result = nullptr;
  for (unsigned Idx = 0; Idx < RISCVFeatureLength; Idx++) {
    if (RequireBitMasks[Idx] == 0)
      continue;

    Value *Mask = Builder.getInt64(RequireBitMasks[Idx]);
    Value *Bitset =
        Builder.CreateAnd(loadRISCVFeatureBits(Idx, Builder, CGM), Mask);
    Value *CmpV = Builder.CreateICmpEQ(Bitset, Mask);
    Result = (!Result) ? CmpV : Builder.CreateAnd(Result, CmpV);
  }

  assert(Result && "Should have value here.");

  return Result;
}

Value *CodeGenFunction::EmitRISCVCpuIs(const CallExpr *E) {
  const Expr *CPUExpr = E->getArg(0)->IgnoreParenCasts();
  StringRef CPUStr = cast<language::Core::StringLiteral>(CPUExpr)->getString();
  return EmitRISCVCpuIs(CPUStr);
}

Value *CodeGenFunction::EmitRISCVCpuIs(StringRef CPUStr) {
  toolchain::Type *Int32Ty = Builder.getInt32Ty();
  toolchain::Type *Int64Ty = Builder.getInt64Ty();
  toolchain::StructType *StructTy = toolchain::StructType::get(Int32Ty, Int64Ty, Int64Ty);
  toolchain::Constant *RISCVCPUModel =
      CGM.CreateRuntimeVariable(StructTy, "__riscv_cpu_model");
  cast<toolchain::GlobalValue>(RISCVCPUModel)->setDSOLocal(true);

  auto loadRISCVCPUID = [&](unsigned Index) {
    Value *Ptr = Builder.CreateStructGEP(StructTy, RISCVCPUModel, Index);
    Value *CPUID = Builder.CreateAlignedLoad(StructTy->getTypeAtIndex(Index),
                                             Ptr, toolchain::MaybeAlign());
    return CPUID;
  };

  const toolchain::RISCV::CPUModel Model = toolchain::RISCV::getCPUModel(CPUStr);

  // Compare mvendorid.
  Value *VendorID = loadRISCVCPUID(0);
  Value *Result =
      Builder.CreateICmpEQ(VendorID, Builder.getInt32(Model.MVendorID));

  // Compare marchid.
  Value *ArchID = loadRISCVCPUID(1);
  Result = Builder.CreateAnd(
      Result, Builder.CreateICmpEQ(ArchID, Builder.getInt64(Model.MArchID)));

  // Compare mimpid.
  Value *ImpID = loadRISCVCPUID(2);
  Result = Builder.CreateAnd(
      Result, Builder.CreateICmpEQ(ImpID, Builder.getInt64(Model.MImpID)));

  return Result;
}

Value *CodeGenFunction::EmitRISCVBuiltinExpr(unsigned BuiltinID,
                                             const CallExpr *E,
                                             ReturnValueSlot ReturnValue) {

  if (BuiltinID == Builtin::BI__builtin_cpu_supports)
    return EmitRISCVCpuSupports(E);
  if (BuiltinID == Builtin::BI__builtin_cpu_init)
    return EmitRISCVCpuInit();
  if (BuiltinID == Builtin::BI__builtin_cpu_is)
    return EmitRISCVCpuIs(E);

  SmallVector<Value *, 4> Ops;
  toolchain::Type *ResultType = ConvertType(E->getType());

  // Find out if any arguments are required to be integer constant expressions.
  unsigned ICEArguments = 0;
  ASTContext::GetBuiltinTypeError Error;
  getContext().GetBuiltinType(BuiltinID, Error, &ICEArguments);
  if (Error == ASTContext::GE_Missing_type) {
    // Vector intrinsics don't have a type string.
    assert(BuiltinID >= language::Core::RISCV::FirstRVVBuiltin &&
           BuiltinID <= language::Core::RISCV::LastRVVBuiltin);
    ICEArguments = 0;
    if (BuiltinID == RISCVVector::BI__builtin_rvv_vget_v ||
        BuiltinID == RISCVVector::BI__builtin_rvv_vset_v)
      ICEArguments = 1 << 1;
  } else {
    assert(Error == ASTContext::GE_None && "Unexpected error");
  }

  if (BuiltinID == RISCV::BI__builtin_riscv_ntl_load)
    ICEArguments |= (1 << 1);
  if (BuiltinID == RISCV::BI__builtin_riscv_ntl_store)
    ICEArguments |= (1 << 2);

  for (unsigned i = 0, e = E->getNumArgs(); i != e; i++) {
    // Handle aggregate argument, namely RVV tuple types in segment load/store
    if (hasAggregateEvaluationKind(E->getArg(i)->getType())) {
      LValue L = EmitAggExprToLValue(E->getArg(i));
      toolchain::Value *AggValue = Builder.CreateLoad(L.getAddress());
      Ops.push_back(AggValue);
      continue;
    }
    Ops.push_back(EmitScalarOrConstFoldImmArg(ICEArguments, i, E));
  }

  Intrinsic::ID ID = Intrinsic::not_intrinsic;
  // The 0th bit simulates the `vta` of RVV
  // The 1st bit simulates the `vma` of RVV
  constexpr unsigned RVV_VTA = 0x1;
  constexpr unsigned RVV_VMA = 0x2;
  int PolicyAttrs = 0;
  bool IsMasked = false;
  // This is used by segment load/store to determine it's toolchain type.
  unsigned SegInstSEW = 8;

  // Required for overloaded intrinsics.
  toolchain::SmallVector<toolchain::Type *, 2> IntrinsicTypes;
  switch (BuiltinID) {
  default: toolchain_unreachable("unexpected builtin ID");
  case RISCV::BI__builtin_riscv_orc_b_32:
  case RISCV::BI__builtin_riscv_orc_b_64:
  case RISCV::BI__builtin_riscv_clmul_32:
  case RISCV::BI__builtin_riscv_clmul_64:
  case RISCV::BI__builtin_riscv_clmulh_32:
  case RISCV::BI__builtin_riscv_clmulh_64:
  case RISCV::BI__builtin_riscv_clmulr_32:
  case RISCV::BI__builtin_riscv_clmulr_64:
  case RISCV::BI__builtin_riscv_xperm4_32:
  case RISCV::BI__builtin_riscv_xperm4_64:
  case RISCV::BI__builtin_riscv_xperm8_32:
  case RISCV::BI__builtin_riscv_xperm8_64:
  case RISCV::BI__builtin_riscv_brev8_32:
  case RISCV::BI__builtin_riscv_brev8_64:
  case RISCV::BI__builtin_riscv_zip_32:
  case RISCV::BI__builtin_riscv_unzip_32: {
    switch (BuiltinID) {
    default: toolchain_unreachable("unexpected builtin ID");
    // Zbb
    case RISCV::BI__builtin_riscv_orc_b_32:
    case RISCV::BI__builtin_riscv_orc_b_64:
      ID = Intrinsic::riscv_orc_b;
      break;

    // Zbc
    case RISCV::BI__builtin_riscv_clmul_32:
    case RISCV::BI__builtin_riscv_clmul_64:
      ID = Intrinsic::riscv_clmul;
      break;
    case RISCV::BI__builtin_riscv_clmulh_32:
    case RISCV::BI__builtin_riscv_clmulh_64:
      ID = Intrinsic::riscv_clmulh;
      break;
    case RISCV::BI__builtin_riscv_clmulr_32:
    case RISCV::BI__builtin_riscv_clmulr_64:
      ID = Intrinsic::riscv_clmulr;
      break;

    // Zbkx
    case RISCV::BI__builtin_riscv_xperm8_32:
    case RISCV::BI__builtin_riscv_xperm8_64:
      ID = Intrinsic::riscv_xperm8;
      break;
    case RISCV::BI__builtin_riscv_xperm4_32:
    case RISCV::BI__builtin_riscv_xperm4_64:
      ID = Intrinsic::riscv_xperm4;
      break;

    // Zbkb
    case RISCV::BI__builtin_riscv_brev8_32:
    case RISCV::BI__builtin_riscv_brev8_64:
      ID = Intrinsic::riscv_brev8;
      break;
    case RISCV::BI__builtin_riscv_zip_32:
      ID = Intrinsic::riscv_zip;
      break;
    case RISCV::BI__builtin_riscv_unzip_32:
      ID = Intrinsic::riscv_unzip;
      break;
    }

    IntrinsicTypes = {ResultType};
    break;
  }

  // Zk builtins

  // Zknh
  case RISCV::BI__builtin_riscv_sha256sig0:
    ID = Intrinsic::riscv_sha256sig0;
    break;
  case RISCV::BI__builtin_riscv_sha256sig1:
    ID = Intrinsic::riscv_sha256sig1;
    break;
  case RISCV::BI__builtin_riscv_sha256sum0:
    ID = Intrinsic::riscv_sha256sum0;
    break;
  case RISCV::BI__builtin_riscv_sha256sum1:
    ID = Intrinsic::riscv_sha256sum1;
    break;

  // Zksed
  case RISCV::BI__builtin_riscv_sm4ks:
    ID = Intrinsic::riscv_sm4ks;
    break;
  case RISCV::BI__builtin_riscv_sm4ed:
    ID = Intrinsic::riscv_sm4ed;
    break;

  // Zksh
  case RISCV::BI__builtin_riscv_sm3p0:
    ID = Intrinsic::riscv_sm3p0;
    break;
  case RISCV::BI__builtin_riscv_sm3p1:
    ID = Intrinsic::riscv_sm3p1;
    break;

  case RISCV::BI__builtin_riscv_clz_32:
  case RISCV::BI__builtin_riscv_clz_64: {
    Function *F = CGM.getIntrinsic(Intrinsic::ctlz, Ops[0]->getType());
    Value *Result = Builder.CreateCall(F, {Ops[0], Builder.getInt1(false)});
    if (Result->getType() != ResultType)
      Result =
          Builder.CreateIntCast(Result, ResultType, /*isSigned*/ false, "cast");
    return Result;
  }
  case RISCV::BI__builtin_riscv_ctz_32:
  case RISCV::BI__builtin_riscv_ctz_64: {
    Function *F = CGM.getIntrinsic(Intrinsic::cttz, Ops[0]->getType());
    Value *Result = Builder.CreateCall(F, {Ops[0], Builder.getInt1(false)});
    if (Result->getType() != ResultType)
      Result =
          Builder.CreateIntCast(Result, ResultType, /*isSigned*/ false, "cast");
    return Result;
  }

  // Zihintntl
  case RISCV::BI__builtin_riscv_ntl_load: {
    toolchain::Type *ResTy = ConvertType(E->getType());
    unsigned DomainVal = 5; // Default __RISCV_NTLH_ALL
    if (Ops.size() == 2)
      DomainVal = cast<ConstantInt>(Ops[1])->getZExtValue();

    toolchain::MDNode *RISCVDomainNode = toolchain::MDNode::get(
        getLLVMContext(),
        toolchain::ConstantAsMetadata::get(Builder.getInt32(DomainVal)));
    toolchain::MDNode *NontemporalNode = toolchain::MDNode::get(
        getLLVMContext(), toolchain::ConstantAsMetadata::get(Builder.getInt32(1)));

    int Width;
    if(ResTy->isScalableTy()) {
      const ScalableVectorType *SVTy = cast<ScalableVectorType>(ResTy);
      toolchain::Type *ScalarTy = ResTy->getScalarType();
      Width = ScalarTy->getPrimitiveSizeInBits() *
              SVTy->getElementCount().getKnownMinValue();
    } else
      Width = ResTy->getPrimitiveSizeInBits();
    LoadInst *Load = Builder.CreateLoad(
        Address(Ops[0], ResTy, CharUnits::fromQuantity(Width / 8)));

    Load->setMetadata(toolchain::LLVMContext::MD_nontemporal, NontemporalNode);
    Load->setMetadata(CGM.getModule().getMDKindID("riscv-nontemporal-domain"),
                      RISCVDomainNode);

    return Load;
  }
  case RISCV::BI__builtin_riscv_ntl_store: {
    unsigned DomainVal = 5; // Default __RISCV_NTLH_ALL
    if (Ops.size() == 3)
      DomainVal = cast<ConstantInt>(Ops[2])->getZExtValue();

    toolchain::MDNode *RISCVDomainNode = toolchain::MDNode::get(
        getLLVMContext(),
        toolchain::ConstantAsMetadata::get(Builder.getInt32(DomainVal)));
    toolchain::MDNode *NontemporalNode = toolchain::MDNode::get(
        getLLVMContext(), toolchain::ConstantAsMetadata::get(Builder.getInt32(1)));

    StoreInst *Store = Builder.CreateDefaultAlignedStore(Ops[1], Ops[0]);
    Store->setMetadata(toolchain::LLVMContext::MD_nontemporal, NontemporalNode);
    Store->setMetadata(CGM.getModule().getMDKindID("riscv-nontemporal-domain"),
                       RISCVDomainNode);

    return Store;
  }
  // Zihintpause
  case RISCV::BI__builtin_riscv_pause: {
    toolchain::Function *Fn = CGM.getIntrinsic(toolchain::Intrinsic::riscv_pause);
    return Builder.CreateCall(Fn, {});
  }

  // XCValu
  case RISCV::BI__builtin_riscv_cv_alu_addN:
    ID = Intrinsic::riscv_cv_alu_addN;
    break;
  case RISCV::BI__builtin_riscv_cv_alu_addRN:
    ID = Intrinsic::riscv_cv_alu_addRN;
    break;
  case RISCV::BI__builtin_riscv_cv_alu_adduN:
    ID = Intrinsic::riscv_cv_alu_adduN;
    break;
  case RISCV::BI__builtin_riscv_cv_alu_adduRN:
    ID = Intrinsic::riscv_cv_alu_adduRN;
    break;
  case RISCV::BI__builtin_riscv_cv_alu_clip:
    ID = Intrinsic::riscv_cv_alu_clip;
    break;
  case RISCV::BI__builtin_riscv_cv_alu_clipu:
    ID = Intrinsic::riscv_cv_alu_clipu;
    break;
  case RISCV::BI__builtin_riscv_cv_alu_extbs:
    return Builder.CreateSExt(Builder.CreateTrunc(Ops[0], Int8Ty), Int32Ty,
                              "extbs");
  case RISCV::BI__builtin_riscv_cv_alu_extbz:
    return Builder.CreateZExt(Builder.CreateTrunc(Ops[0], Int8Ty), Int32Ty,
                              "extbz");
  case RISCV::BI__builtin_riscv_cv_alu_exths:
    return Builder.CreateSExt(Builder.CreateTrunc(Ops[0], Int16Ty), Int32Ty,
                              "exths");
  case RISCV::BI__builtin_riscv_cv_alu_exthz:
    return Builder.CreateZExt(Builder.CreateTrunc(Ops[0], Int16Ty), Int32Ty,
                              "exthz");
  case RISCV::BI__builtin_riscv_cv_alu_sle:
    return Builder.CreateZExt(Builder.CreateICmpSLE(Ops[0], Ops[1]), Int32Ty,
                              "sle");
  case RISCV::BI__builtin_riscv_cv_alu_sleu:
    return Builder.CreateZExt(Builder.CreateICmpULE(Ops[0], Ops[1]), Int32Ty,
                              "sleu");
  case RISCV::BI__builtin_riscv_cv_alu_subN:
    ID = Intrinsic::riscv_cv_alu_subN;
    break;
  case RISCV::BI__builtin_riscv_cv_alu_subRN:
    ID = Intrinsic::riscv_cv_alu_subRN;
    break;
  case RISCV::BI__builtin_riscv_cv_alu_subuN:
    ID = Intrinsic::riscv_cv_alu_subuN;
    break;
  case RISCV::BI__builtin_riscv_cv_alu_subuRN:
    ID = Intrinsic::riscv_cv_alu_subuRN;
    break;

  // XAndesBFHCvt
  case RISCV::BI__builtin_riscv_nds_fcvt_s_bf16:
    return Builder.CreateFPExt(Ops[0], FloatTy);
  case RISCV::BI__builtin_riscv_nds_fcvt_bf16_s:
    return Builder.CreateFPTrunc(Ops[0], BFloatTy);

    // Vector builtins are handled from here.
#include "language/Core/Basic/riscv_vector_builtin_cg.inc"

    // SiFive Vector builtins are handled from here.
#include "language/Core/Basic/riscv_sifive_vector_builtin_cg.inc"

    // Andes Vector builtins are handled from here.
#include "language/Core/Basic/riscv_andes_vector_builtin_cg.inc"
  }

  assert(ID != Intrinsic::not_intrinsic);

  toolchain::Function *F = CGM.getIntrinsic(ID, IntrinsicTypes);
  return Builder.CreateCall(F, Ops, "");
}
