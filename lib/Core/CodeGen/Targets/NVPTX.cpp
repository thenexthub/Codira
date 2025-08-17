//===- NVPTX.cpp ----------------------------------------------------------===//
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

#include "ABIInfoImpl.h"
#include "TargetInfo.h"
#include "toolchain/ADT/STLExtras.h"
#include "toolchain/ADT/StringExtras.h"
#include "toolchain/IR/CallingConv.h"
#include "toolchain/IR/IntrinsicsNVPTX.h"

using namespace language::Core;
using namespace language::Core::CodeGen;

//===----------------------------------------------------------------------===//
// NVPTX ABI Implementation
//===----------------------------------------------------------------------===//

namespace {

class NVPTXTargetCodeGenInfo;

class NVPTXABIInfo : public ABIInfo {
  NVPTXTargetCodeGenInfo &CGInfo;

public:
  NVPTXABIInfo(CodeGenTypes &CGT, NVPTXTargetCodeGenInfo &Info)
      : ABIInfo(CGT), CGInfo(Info) {}

  ABIArgInfo classifyReturnType(QualType RetTy) const;
  ABIArgInfo classifyArgumentType(QualType Ty) const;

  void computeInfo(CGFunctionInfo &FI) const override;
  RValue EmitVAArg(CodeGenFunction &CGF, Address VAListAddr, QualType Ty,
                   AggValueSlot Slot) const override;
  bool isUnsupportedType(QualType T) const;
  ABIArgInfo coerceToIntArrayWithLimit(QualType Ty, unsigned MaxSize) const;
};

class NVPTXTargetCodeGenInfo : public TargetCodeGenInfo {
public:
  NVPTXTargetCodeGenInfo(CodeGenTypes &CGT)
      : TargetCodeGenInfo(std::make_unique<NVPTXABIInfo>(CGT, *this)) {}

  void setTargetAttributes(const Decl *D, toolchain::GlobalValue *GV,
                           CodeGen::CodeGenModule &M) const override;
  bool shouldEmitStaticExternCAliases() const override;

  toolchain::Constant *getNullPointer(const CodeGen::CodeGenModule &CGM,
                                 toolchain::PointerType *T,
                                 QualType QT) const override;

  toolchain::Type *getCUDADeviceBuiltinSurfaceDeviceType() const override {
    // On the device side, surface reference is represented as an object handle
    // in 64-bit integer.
    return toolchain::Type::getInt64Ty(getABIInfo().getVMContext());
  }

  toolchain::Type *getCUDADeviceBuiltinTextureDeviceType() const override {
    // On the device side, texture reference is represented as an object handle
    // in 64-bit integer.
    return toolchain::Type::getInt64Ty(getABIInfo().getVMContext());
  }

  bool emitCUDADeviceBuiltinSurfaceDeviceCopy(CodeGenFunction &CGF, LValue Dst,
                                              LValue Src) const override {
    emitBuiltinSurfTexDeviceCopy(CGF, Dst, Src);
    return true;
  }

  bool emitCUDADeviceBuiltinTextureDeviceCopy(CodeGenFunction &CGF, LValue Dst,
                                              LValue Src) const override {
    emitBuiltinSurfTexDeviceCopy(CGF, Dst, Src);
    return true;
  }

  unsigned getDeviceKernelCallingConv() const override {
    return toolchain::CallingConv::PTX_Kernel;
  }

  // Adds a NamedMDNode with GV, Name, and Operand as operands, and adds the
  // resulting MDNode to the nvvm.annotations MDNode.
  static void addNVVMMetadata(toolchain::GlobalValue *GV, StringRef Name,
                              int Operand);

  static void
  addGridConstantNVVMMetadata(toolchain::GlobalValue *GV,
                              const SmallVectorImpl<int> &GridConstantArgs);

private:
  static void emitBuiltinSurfTexDeviceCopy(CodeGenFunction &CGF, LValue Dst,
                                           LValue Src) {
    toolchain::Value *Handle = nullptr;
    toolchain::Constant *C =
        toolchain::dyn_cast<toolchain::Constant>(Src.getAddress().emitRawPointer(CGF));
    // Lookup `addrspacecast` through the constant pointer if any.
    if (auto *ASC = toolchain::dyn_cast_or_null<toolchain::AddrSpaceCastOperator>(C))
      C = toolchain::cast<toolchain::Constant>(ASC->getPointerOperand());
    if (auto *GV = toolchain::dyn_cast_or_null<toolchain::GlobalVariable>(C)) {
      // Load the handle from the specific global variable using
      // `nvvm.texsurf.handle.internal` intrinsic.
      Handle = CGF.EmitRuntimeCall(
          CGF.CGM.getIntrinsic(toolchain::Intrinsic::nvvm_texsurf_handle_internal,
                               {GV->getType()}),
          {GV}, "texsurf_handle");
    } else
      Handle = CGF.EmitLoadOfScalar(Src, SourceLocation());
    CGF.EmitStoreOfScalar(Handle, Dst);
  }
};

/// Checks if the type is unsupported directly by the current target.
bool NVPTXABIInfo::isUnsupportedType(QualType T) const {
  ASTContext &Context = getContext();
  if (!Context.getTargetInfo().hasFloat16Type() && T->isFloat16Type())
    return true;
  if (!Context.getTargetInfo().hasFloat128Type() &&
      (T->isFloat128Type() ||
       (T->isRealFloatingType() && Context.getTypeSize(T) == 128)))
    return true;
  if (const auto *EIT = T->getAs<BitIntType>())
    return EIT->getNumBits() >
           (Context.getTargetInfo().hasInt128Type() ? 128U : 64U);
  if (!Context.getTargetInfo().hasInt128Type() && T->isIntegerType() &&
      Context.getTypeSize(T) > 64U)
    return true;
  if (const auto *AT = T->getAsArrayTypeUnsafe())
    return isUnsupportedType(AT->getElementType());
  const auto *RT = T->getAs<RecordType>();
  if (!RT)
    return false;
  const RecordDecl *RD = RT->getOriginalDecl()->getDefinitionOrSelf();

  // If this is a C++ record, check the bases first.
  if (const CXXRecordDecl *CXXRD = dyn_cast<CXXRecordDecl>(RD))
    for (const CXXBaseSpecifier &I : CXXRD->bases())
      if (isUnsupportedType(I.getType()))
        return true;

  for (const FieldDecl *I : RD->fields())
    if (isUnsupportedType(I->getType()))
      return true;
  return false;
}

/// Coerce the given type into an array with maximum allowed size of elements.
ABIArgInfo NVPTXABIInfo::coerceToIntArrayWithLimit(QualType Ty,
                                                   unsigned MaxSize) const {
  // Alignment and Size are measured in bits.
  const uint64_t Size = getContext().getTypeSize(Ty);
  const uint64_t Alignment = getContext().getTypeAlign(Ty);
  const unsigned Div = std::min<unsigned>(MaxSize, Alignment);
  toolchain::Type *IntType = toolchain::Type::getIntNTy(getVMContext(), Div);
  const uint64_t NumElements = (Size + Div - 1) / Div;
  return ABIArgInfo::getDirect(toolchain::ArrayType::get(IntType, NumElements));
}

ABIArgInfo NVPTXABIInfo::classifyReturnType(QualType RetTy) const {
  if (RetTy->isVoidType())
    return ABIArgInfo::getIgnore();

  if (getContext().getLangOpts().OpenMP &&
      getContext().getLangOpts().OpenMPIsTargetDevice &&
      isUnsupportedType(RetTy))
    return coerceToIntArrayWithLimit(RetTy, 64);

  // note: this is different from default ABI
  if (!RetTy->isScalarType())
    return ABIArgInfo::getDirect();

  // Treat an enum type as its underlying type.
  if (const EnumType *EnumTy = RetTy->getAs<EnumType>())
    RetTy = EnumTy->getOriginalDecl()->getDefinitionOrSelf()->getIntegerType();

  return (isPromotableIntegerTypeForABI(RetTy) ? ABIArgInfo::getExtend(RetTy)
                                               : ABIArgInfo::getDirect());
}

ABIArgInfo NVPTXABIInfo::classifyArgumentType(QualType Ty) const {
  // Treat an enum type as its underlying type.
  if (const EnumType *EnumTy = Ty->getAs<EnumType>())
    Ty = EnumTy->getOriginalDecl()->getDefinitionOrSelf()->getIntegerType();

  // Return aggregates type as indirect by value
  if (isAggregateTypeForABI(Ty)) {
    // Under CUDA device compilation, tex/surf builtin types are replaced with
    // object types and passed directly.
    if (getContext().getLangOpts().CUDAIsDevice) {
      if (Ty->isCUDADeviceBuiltinSurfaceType())
        return ABIArgInfo::getDirect(
            CGInfo.getCUDADeviceBuiltinSurfaceDeviceType());
      if (Ty->isCUDADeviceBuiltinTextureType())
        return ABIArgInfo::getDirect(
            CGInfo.getCUDADeviceBuiltinTextureDeviceType());
    }
    return getNaturalAlignIndirect(
        Ty, /* AddrSpace */ getDataLayout().getAllocaAddrSpace(),
        /* byval */ true);
  }

  if (const auto *EIT = Ty->getAs<BitIntType>()) {
    if ((EIT->getNumBits() > 128) ||
        (!getContext().getTargetInfo().hasInt128Type() &&
         EIT->getNumBits() > 64))
      return getNaturalAlignIndirect(
          Ty, /* AddrSpace */ getDataLayout().getAllocaAddrSpace(),
          /* byval */ true);
  }

  return (isPromotableIntegerTypeForABI(Ty) ? ABIArgInfo::getExtend(Ty)
                                            : ABIArgInfo::getDirect());
}

void NVPTXABIInfo::computeInfo(CGFunctionInfo &FI) const {
  if (!getCXXABI().classifyReturnType(FI))
    FI.getReturnInfo() = classifyReturnType(FI.getReturnType());

  for (auto &&[ArgumentsCount, I] : toolchain::enumerate(FI.arguments()))
    I.info = ArgumentsCount < FI.getNumRequiredArgs()
                 ? classifyArgumentType(I.type)
                 : ABIArgInfo::getDirect();

  // Always honor user-specified calling convention.
  if (FI.getCallingConvention() != toolchain::CallingConv::C)
    return;

  FI.setEffectiveCallingConvention(getRuntimeCC());
}

RValue NVPTXABIInfo::EmitVAArg(CodeGenFunction &CGF, Address VAListAddr,
                               QualType Ty, AggValueSlot Slot) const {
  return emitVoidPtrVAArg(CGF, VAListAddr, Ty, /*IsIndirect=*/false,
                          getContext().getTypeInfoInChars(Ty),
                          CharUnits::fromQuantity(1),
                          /*AllowHigherAlign=*/true, Slot);
}

void NVPTXTargetCodeGenInfo::setTargetAttributes(
    const Decl *D, toolchain::GlobalValue *GV, CodeGen::CodeGenModule &M) const {
  if (GV->isDeclaration())
    return;
  const VarDecl *VD = dyn_cast_or_null<VarDecl>(D);
  if (VD) {
    if (M.getLangOpts().CUDA) {
      if (VD->getType()->isCUDADeviceBuiltinSurfaceType())
        addNVVMMetadata(GV, "surface", 1);
      else if (VD->getType()->isCUDADeviceBuiltinTextureType())
        addNVVMMetadata(GV, "texture", 1);
      return;
    }
  }

  const FunctionDecl *FD = dyn_cast_or_null<FunctionDecl>(D);
  if (!FD)
    return;

  toolchain::Function *F = cast<toolchain::Function>(GV);

  // Perform special handling in OpenCL/CUDA mode
  if (M.getLangOpts().OpenCL || M.getLangOpts().CUDA) {
    // Use function attributes to check for kernel functions
    // By default, all functions are device functions
    if (FD->hasAttr<DeviceKernelAttr>() || FD->hasAttr<CUDAGlobalAttr>()) {
      // OpenCL/CUDA kernel functions get kernel metadata
      // Create !{<func-ref>, metadata !"kernel", i32 1} node
      // And kernel functions are not subject to inlining
      F->addFnAttr(toolchain::Attribute::NoInline);
      if (FD->hasAttr<CUDAGlobalAttr>()) {
        SmallVector<int, 10> GCI;
        for (auto IV : toolchain::enumerate(FD->parameters()))
          if (IV.value()->hasAttr<CUDAGridConstantAttr>())
            // For some reason arg indices are 1-based in NVVM
            GCI.push_back(IV.index() + 1);
        // Create !{<func-ref>, metadata !"kernel", i32 1} node
        F->setCallingConv(toolchain::CallingConv::PTX_Kernel);
        addGridConstantNVVMMetadata(F, GCI);
      }
      if (CUDALaunchBoundsAttr *Attr = FD->getAttr<CUDALaunchBoundsAttr>())
        M.handleCUDALaunchBoundsAttr(F, Attr);
    }
  }
  // Attach kernel metadata directly if compiling for NVPTX.
  if (FD->hasAttr<DeviceKernelAttr>()) {
    F->setCallingConv(toolchain::CallingConv::PTX_Kernel);
  }
}

void NVPTXTargetCodeGenInfo::addNVVMMetadata(toolchain::GlobalValue *GV,
                                             StringRef Name, int Operand) {
  toolchain::Module *M = GV->getParent();
  toolchain::LLVMContext &Ctx = M->getContext();

  // Get "nvvm.annotations" metadata node
  toolchain::NamedMDNode *MD = M->getOrInsertNamedMetadata("nvvm.annotations");

  SmallVector<toolchain::Metadata *, 5> MDVals = {
      toolchain::ConstantAsMetadata::get(GV), toolchain::MDString::get(Ctx, Name),
      toolchain::ConstantAsMetadata::get(
          toolchain::ConstantInt::get(toolchain::Type::getInt32Ty(Ctx), Operand))};

  // Append metadata to nvvm.annotations
  MD->addOperand(toolchain::MDNode::get(Ctx, MDVals));
}

void NVPTXTargetCodeGenInfo::addGridConstantNVVMMetadata(
    toolchain::GlobalValue *GV, const SmallVectorImpl<int> &GridConstantArgs) {

  toolchain::Module *M = GV->getParent();
  toolchain::LLVMContext &Ctx = M->getContext();

  // Get "nvvm.annotations" metadata node
  toolchain::NamedMDNode *MD = M->getOrInsertNamedMetadata("nvvm.annotations");

  SmallVector<toolchain::Metadata *, 5> MDVals = {toolchain::ConstantAsMetadata::get(GV)};
  if (!GridConstantArgs.empty()) {
    SmallVector<toolchain::Metadata *, 10> GCM;
    for (int I : GridConstantArgs)
      GCM.push_back(toolchain::ConstantAsMetadata::get(
          toolchain::ConstantInt::get(toolchain::Type::getInt32Ty(Ctx), I)));
    MDVals.append({toolchain::MDString::get(Ctx, "grid_constant"),
                   toolchain::MDNode::get(Ctx, GCM)});
  }

  // Append metadata to nvvm.annotations
  MD->addOperand(toolchain::MDNode::get(Ctx, MDVals));
}

bool NVPTXTargetCodeGenInfo::shouldEmitStaticExternCAliases() const {
  return false;
}

toolchain::Constant *
NVPTXTargetCodeGenInfo::getNullPointer(const CodeGen::CodeGenModule &CGM,
                                       toolchain::PointerType *PT,
                                       QualType QT) const {
  auto &Ctx = CGM.getContext();
  if (PT->getAddressSpace() != Ctx.getTargetAddressSpace(LangAS::opencl_local))
    return toolchain::ConstantPointerNull::get(PT);

  auto NPT = toolchain::PointerType::get(
      PT->getContext(), Ctx.getTargetAddressSpace(LangAS::opencl_generic));
  return toolchain::ConstantExpr::getAddrSpaceCast(
      toolchain::ConstantPointerNull::get(NPT), PT);
}
} // namespace

void CodeGenModule::handleCUDALaunchBoundsAttr(toolchain::Function *F,
                                               const CUDALaunchBoundsAttr *Attr,
                                               int32_t *MaxThreadsVal,
                                               int32_t *MinBlocksVal,
                                               int32_t *MaxClusterRankVal) {
  toolchain::APSInt MaxThreads(32);
  MaxThreads = Attr->getMaxThreads()->EvaluateKnownConstInt(getContext());
  if (MaxThreads > 0) {
    if (MaxThreadsVal)
      *MaxThreadsVal = MaxThreads.getExtValue();
    if (F)
      F->addFnAttr("nvvm.maxntid", toolchain::utostr(MaxThreads.getExtValue()));
  }

  // min and max blocks is an optional argument for CUDALaunchBoundsAttr. If it
  // was not specified in __launch_bounds__ or if the user specified a 0 value,
  // we don't have to add a PTX directive.
  if (Attr->getMinBlocks()) {
    toolchain::APSInt MinBlocks(32);
    MinBlocks = Attr->getMinBlocks()->EvaluateKnownConstInt(getContext());
    if (MinBlocks > 0) {
      if (MinBlocksVal)
        *MinBlocksVal = MinBlocks.getExtValue();
      if (F)
        F->addFnAttr("nvvm.minctasm", toolchain::utostr(MinBlocks.getExtValue()));
    }
  }
  if (Attr->getMaxBlocks()) {
    toolchain::APSInt MaxBlocks(32);
    MaxBlocks = Attr->getMaxBlocks()->EvaluateKnownConstInt(getContext());
    if (MaxBlocks > 0) {
      if (MaxClusterRankVal)
        *MaxClusterRankVal = MaxBlocks.getExtValue();
      if (F)
        F->addFnAttr("nvvm.maxclusterrank",
                     toolchain::utostr(MaxBlocks.getExtValue()));
    }
  }
}

std::unique_ptr<TargetCodeGenInfo>
CodeGen::createNVPTXTargetCodeGenInfo(CodeGenModule &CGM) {
  return std::make_unique<NVPTXTargetCodeGenInfo>(CGM.getTypes());
}
