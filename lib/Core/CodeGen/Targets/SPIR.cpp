//===- SPIR.cpp -----------------------------------------------------------===//
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
#include "HLSLBufferLayoutBuilder.h"
#include "TargetInfo.h"

using namespace language::Core;
using namespace language::Core::CodeGen;

//===----------------------------------------------------------------------===//
// Base ABI and target codegen info implementation common between SPIR and
// SPIR-V.
//===----------------------------------------------------------------------===//

namespace {
class CommonSPIRABIInfo : public DefaultABIInfo {
public:
  CommonSPIRABIInfo(CodeGenTypes &CGT) : DefaultABIInfo(CGT) { setCCs(); }

private:
  void setCCs();
};

class SPIRVABIInfo : public CommonSPIRABIInfo {
public:
  SPIRVABIInfo(CodeGenTypes &CGT) : CommonSPIRABIInfo(CGT) {}
  void computeInfo(CGFunctionInfo &FI) const override;

private:
  ABIArgInfo classifyReturnType(QualType RetTy) const;
  ABIArgInfo classifyKernelArgumentType(QualType Ty) const;
  ABIArgInfo classifyArgumentType(QualType Ty) const;
};
} // end anonymous namespace
namespace {
class CommonSPIRTargetCodeGenInfo : public TargetCodeGenInfo {
public:
  CommonSPIRTargetCodeGenInfo(CodeGen::CodeGenTypes &CGT)
      : TargetCodeGenInfo(std::make_unique<CommonSPIRABIInfo>(CGT)) {}
  CommonSPIRTargetCodeGenInfo(std::unique_ptr<ABIInfo> ABIInfo)
      : TargetCodeGenInfo(std::move(ABIInfo)) {}

  LangAS getASTAllocaAddressSpace() const override {
    return getLangASFromTargetAS(
        getABIInfo().getDataLayout().getAllocaAddrSpace());
  }

  unsigned getDeviceKernelCallingConv() const override;
  toolchain::Type *getOpenCLType(CodeGenModule &CGM, const Type *T) const override;
  toolchain::Type *
  getHLSLType(CodeGenModule &CGM, const Type *Ty,
              const SmallVector<int32_t> *Packoffsets = nullptr) const override;
  toolchain::Type *getSPIRVImageTypeFromHLSLResource(
      const HLSLAttributedResourceType::Attributes &attributes,
      QualType SampledType, CodeGenModule &CGM) const;
  void
  setOCLKernelStubCallingConvention(const FunctionType *&FT) const override;
};
class SPIRVTargetCodeGenInfo : public CommonSPIRTargetCodeGenInfo {
public:
  SPIRVTargetCodeGenInfo(CodeGen::CodeGenTypes &CGT)
      : CommonSPIRTargetCodeGenInfo(std::make_unique<SPIRVABIInfo>(CGT)) {}
  void setCUDAKernelCallingConvention(const FunctionType *&FT) const override;
  LangAS getGlobalVarAddressSpace(CodeGenModule &CGM,
                                  const VarDecl *D) const override;
  void setTargetAttributes(const Decl *D, toolchain::GlobalValue *GV,
                           CodeGen::CodeGenModule &M) const override;
  toolchain::SyncScope::ID getLLVMSyncScopeID(const LangOptions &LangOpts,
                                         SyncScope Scope,
                                         toolchain::AtomicOrdering Ordering,
                                         toolchain::LLVMContext &Ctx) const override;
  bool supportsLibCall() const override {
    return getABIInfo().getTarget().getTriple().getVendor() !=
           toolchain::Triple::AMD;
  }
};

inline StringRef mapClangSyncScopeToLLVM(SyncScope Scope) {
  switch (Scope) {
  case SyncScope::HIPSingleThread:
  case SyncScope::SingleScope:
    return "singlethread";
  case SyncScope::HIPWavefront:
  case SyncScope::OpenCLSubGroup:
  case SyncScope::WavefrontScope:
    return "subgroup";
  case SyncScope::HIPWorkgroup:
  case SyncScope::OpenCLWorkGroup:
  case SyncScope::WorkgroupScope:
    return "workgroup";
  case SyncScope::HIPAgent:
  case SyncScope::OpenCLDevice:
  case SyncScope::DeviceScope:
    return "device";
  case SyncScope::SystemScope:
  case SyncScope::HIPSystem:
  case SyncScope::OpenCLAllSVMDevices:
    return "";
  }
  return "";
}
} // End anonymous namespace.

void CommonSPIRABIInfo::setCCs() {
  assert(getRuntimeCC() == toolchain::CallingConv::C);
  RuntimeCC = toolchain::CallingConv::SPIR_FUNC;
}

ABIArgInfo SPIRVABIInfo::classifyReturnType(QualType RetTy) const {
  if (getTarget().getTriple().getVendor() != toolchain::Triple::AMD)
    return DefaultABIInfo::classifyReturnType(RetTy);
  if (!isAggregateTypeForABI(RetTy) || getRecordArgABI(RetTy, getCXXABI()))
    return DefaultABIInfo::classifyReturnType(RetTy);

  if (const RecordType *RT = RetTy->getAs<RecordType>()) {
    const RecordDecl *RD = RT->getOriginalDecl()->getDefinitionOrSelf();
    if (RD->hasFlexibleArrayMember())
      return DefaultABIInfo::classifyReturnType(RetTy);
  }

  // TODO: The AMDGPU ABI is non-trivial to represent in SPIR-V; in order to
  // avoid encoding various architecture specific bits here we return everything
  // as direct to retain type info for things like aggregates, for later perusal
  // when translating back to LLVM/lowering in the BE. This is also why we
  // disable flattening as the outcomes can mismatch between SPIR-V and AMDGPU.
  // This will be revisited / optimised in the future.
  return ABIArgInfo::getDirect(CGT.ConvertType(RetTy), 0u, nullptr, false);
}

ABIArgInfo SPIRVABIInfo::classifyKernelArgumentType(QualType Ty) const {
  if (getContext().getLangOpts().CUDAIsDevice) {
    // Coerce pointer arguments with default address space to CrossWorkGroup
    // pointers for HIPSPV/CUDASPV. When the language mode is HIP/CUDA, the
    // SPIRTargetInfo maps cuda_device to SPIR-V's CrossWorkGroup address space.
    toolchain::Type *LTy = CGT.ConvertType(Ty);
    auto DefaultAS = getContext().getTargetAddressSpace(LangAS::Default);
    auto GlobalAS = getContext().getTargetAddressSpace(LangAS::cuda_device);
    auto *PtrTy = toolchain::dyn_cast<toolchain::PointerType>(LTy);
    if (PtrTy && PtrTy->getAddressSpace() == DefaultAS) {
      LTy = toolchain::PointerType::get(PtrTy->getContext(), GlobalAS);
      return ABIArgInfo::getDirect(LTy, 0, nullptr, false);
    }

    if (isAggregateTypeForABI(Ty)) {
      if (getTarget().getTriple().getVendor() == toolchain::Triple::AMD)
        // TODO: The AMDGPU kernel ABI passes aggregates byref, which is not
        // currently expressible in SPIR-V; SPIR-V passes aggregates byval,
        // which the AMDGPU kernel ABI does not allow. Passing aggregates as
        // direct works around this impedance mismatch, as it retains type info
        // and can be correctly handled, post reverse-translation, by the AMDGPU
        // BE, which has to support this CC for legacy OpenCL purposes. It can
        // be brittle and does lead to performance degradation in certain
        // pathological cases. This will be revisited / optimised in the future,
        // once a way to deal with the byref/byval impedance mismatch is
        // identified.
        return ABIArgInfo::getDirect(LTy, 0, nullptr, false);
      // Force copying aggregate type in kernel arguments by value when
      // compiling CUDA targeting SPIR-V. This is required for the object
      // copied to be valid on the device.
      // This behavior follows the CUDA spec
      // https://docs.nvidia.com/cuda/cuda-c-programming-guide/index.html#global-function-argument-processing,
      // and matches the NVPTX implementation. TODO: hardcoding to 0 should be
      // revisited if HIPSPV / byval starts making use of the AS of an indirect
      // arg.
      return getNaturalAlignIndirect(Ty, /*AddrSpace=*/0, /*byval=*/true);
    }
  }
  return classifyArgumentType(Ty);
}

ABIArgInfo SPIRVABIInfo::classifyArgumentType(QualType Ty) const {
  if (getTarget().getTriple().getVendor() != toolchain::Triple::AMD)
    return DefaultABIInfo::classifyArgumentType(Ty);
  if (!isAggregateTypeForABI(Ty))
    return DefaultABIInfo::classifyArgumentType(Ty);

  // Records with non-trivial destructors/copy-constructors should not be
  // passed by value.
  if (auto RAA = getRecordArgABI(Ty, getCXXABI()))
    return getNaturalAlignIndirect(Ty, getDataLayout().getAllocaAddrSpace(),
                                   RAA == CGCXXABI::RAA_DirectInMemory);

  if (const RecordType *RT = Ty->getAs<RecordType>()) {
    const RecordDecl *RD = RT->getOriginalDecl()->getDefinitionOrSelf();
    if (RD->hasFlexibleArrayMember())
      return DefaultABIInfo::classifyArgumentType(Ty);
  }

  return ABIArgInfo::getDirect(CGT.ConvertType(Ty), 0u, nullptr, false);
}

void SPIRVABIInfo::computeInfo(CGFunctionInfo &FI) const {
  // The logic is same as in DefaultABIInfo with an exception on the kernel
  // arguments handling.
  toolchain::CallingConv::ID CC = FI.getCallingConvention();

  if (!getCXXABI().classifyReturnType(FI))
    FI.getReturnInfo() = classifyReturnType(FI.getReturnType());

  for (auto &I : FI.arguments()) {
    if (CC == toolchain::CallingConv::SPIR_KERNEL) {
      I.info = classifyKernelArgumentType(I.type);
    } else {
      I.info = classifyArgumentType(I.type);
    }
  }
}

namespace language::Core {
namespace CodeGen {
void computeSPIRKernelABIInfo(CodeGenModule &CGM, CGFunctionInfo &FI) {
  if (CGM.getTarget().getTriple().isSPIRV())
    SPIRVABIInfo(CGM.getTypes()).computeInfo(FI);
  else
    CommonSPIRABIInfo(CGM.getTypes()).computeInfo(FI);
}
}
}

unsigned CommonSPIRTargetCodeGenInfo::getDeviceKernelCallingConv() const {
  return toolchain::CallingConv::SPIR_KERNEL;
}

void SPIRVTargetCodeGenInfo::setCUDAKernelCallingConvention(
    const FunctionType *&FT) const {
  // Convert HIP kernels to SPIR-V kernels.
  if (getABIInfo().getContext().getLangOpts().HIP) {
    FT = getABIInfo().getContext().adjustFunctionType(
        FT, FT->getExtInfo().withCallingConv(CC_DeviceKernel));
    return;
  }
}

void CommonSPIRTargetCodeGenInfo::setOCLKernelStubCallingConvention(
    const FunctionType *&FT) const {
  FT = getABIInfo().getContext().adjustFunctionType(
      FT, FT->getExtInfo().withCallingConv(CC_SpirFunction));
}

LangAS
SPIRVTargetCodeGenInfo::getGlobalVarAddressSpace(CodeGenModule &CGM,
                                                 const VarDecl *D) const {
  assert(!CGM.getLangOpts().OpenCL &&
         !(CGM.getLangOpts().CUDA && CGM.getLangOpts().CUDAIsDevice) &&
         "Address space agnostic languages only");
  // If we're here it means that we're using the SPIRDefIsGen ASMap, hence for
  // the global AS we can rely on either cuda_device or sycl_global to be
  // correct; however, since this is not a CUDA Device context, we use
  // sycl_global to prevent confusion with the assertion.
  LangAS DefaultGlobalAS = getLangASFromTargetAS(
      CGM.getContext().getTargetAddressSpace(LangAS::sycl_global));
  if (!D)
    return DefaultGlobalAS;

  LangAS AddrSpace = D->getType().getAddressSpace();
  if (AddrSpace != LangAS::Default)
    return AddrSpace;

  return DefaultGlobalAS;
}

void SPIRVTargetCodeGenInfo::setTargetAttributes(
    const Decl *D, toolchain::GlobalValue *GV, CodeGen::CodeGenModule &M) const {
  if (!M.getLangOpts().HIP ||
      M.getTarget().getTriple().getVendor() != toolchain::Triple::AMD)
    return;
  if (GV->isDeclaration())
    return;

  auto F = dyn_cast<toolchain::Function>(GV);
  if (!F)
    return;

  auto FD = dyn_cast_or_null<FunctionDecl>(D);
  if (!FD)
    return;
  if (!FD->hasAttr<CUDAGlobalAttr>())
    return;

  unsigned N = M.getLangOpts().GPUMaxThreadsPerBlock;
  if (auto FlatWGS = FD->getAttr<AMDGPUFlatWorkGroupSizeAttr>())
    N = FlatWGS->getMax()->EvaluateKnownConstInt(M.getContext()).getExtValue();

  // We encode the maximum flat WG size in the first component of the 3D
  // max_work_group_size attribute, which will get reverse translated into the
  // original AMDGPU attribute when targeting AMDGPU.
  auto Int32Ty = toolchain::IntegerType::getInt32Ty(M.getLLVMContext());
  toolchain::Metadata *AttrMDArgs[] = {
      toolchain::ConstantAsMetadata::get(toolchain::ConstantInt::get(Int32Ty, N)),
      toolchain::ConstantAsMetadata::get(toolchain::ConstantInt::get(Int32Ty, 1)),
      toolchain::ConstantAsMetadata::get(toolchain::ConstantInt::get(Int32Ty, 1))};

  F->setMetadata("max_work_group_size",
                 toolchain::MDNode::get(M.getLLVMContext(), AttrMDArgs));
}

toolchain::SyncScope::ID
SPIRVTargetCodeGenInfo::getLLVMSyncScopeID(const LangOptions &, SyncScope Scope,
                                           toolchain::AtomicOrdering,
                                           toolchain::LLVMContext &Ctx) const {
  return Ctx.getOrInsertSyncScopeID(mapClangSyncScopeToLLVM(Scope));
}

/// Construct a SPIR-V target extension type for the given OpenCL image type.
static toolchain::Type *getSPIRVImageType(toolchain::LLVMContext &Ctx, StringRef BaseType,
                                     StringRef OpenCLName,
                                     unsigned AccessQualifier) {
  // These parameters compare to the operands of OpTypeImage (see
  // https://registry.khronos.org/SPIR-V/specs/unified1/SPIRV.html#OpTypeImage
  // for more details). The first 6 integer parameters all default to 0, and
  // will be changed to 1 only for the image type(s) that set the parameter to
  // one. The 7th integer parameter is the access qualifier, which is tacked on
  // at the end.
  SmallVector<unsigned, 7> IntParams = {0, 0, 0, 0, 0, 0};

  // Choose the dimension of the image--this corresponds to the Dim enum in
  // SPIR-V (first integer parameter of OpTypeImage).
  if (OpenCLName.starts_with("image2d"))
    IntParams[0] = 1;
  else if (OpenCLName.starts_with("image3d"))
    IntParams[0] = 2;
  else if (OpenCLName == "image1d_buffer")
    IntParams[0] = 5; // Buffer
  else
    assert(OpenCLName.starts_with("image1d") && "Unknown image type");

  // Set the other integer parameters of OpTypeImage if necessary. Note that the
  // OpenCL image types don't provide any information for the Sampled or
  // Image Format parameters.
  if (OpenCLName.contains("_depth"))
    IntParams[1] = 1;
  if (OpenCLName.contains("_array"))
    IntParams[2] = 1;
  if (OpenCLName.contains("_msaa"))
    IntParams[3] = 1;

  // Access qualifier
  IntParams.push_back(AccessQualifier);

  return toolchain::TargetExtType::get(Ctx, BaseType, {toolchain::Type::getVoidTy(Ctx)},
                                  IntParams);
}

toolchain::Type *CommonSPIRTargetCodeGenInfo::getOpenCLType(CodeGenModule &CGM,
                                                       const Type *Ty) const {
  toolchain::LLVMContext &Ctx = CGM.getLLVMContext();
  if (auto *PipeTy = dyn_cast<PipeType>(Ty))
    return toolchain::TargetExtType::get(Ctx, "spirv.Pipe", {},
                                    {!PipeTy->isReadOnly()});
  if (auto *BuiltinTy = dyn_cast<BuiltinType>(Ty)) {
    enum AccessQualifier : unsigned { AQ_ro = 0, AQ_wo = 1, AQ_rw = 2 };
    switch (BuiltinTy->getKind()) {
#define IMAGE_TYPE(ImgType, Id, SingletonId, Access, Suffix)                   \
    case BuiltinType::Id:                                                      \
      return getSPIRVImageType(Ctx, "spirv.Image", #ImgType, AQ_##Suffix);
#include "language/Core/Basic/OpenCLImageTypes.def"
    case BuiltinType::OCLSampler:
      return toolchain::TargetExtType::get(Ctx, "spirv.Sampler");
    case BuiltinType::OCLEvent:
      return toolchain::TargetExtType::get(Ctx, "spirv.Event");
    case BuiltinType::OCLClkEvent:
      return toolchain::TargetExtType::get(Ctx, "spirv.DeviceEvent");
    case BuiltinType::OCLQueue:
      return toolchain::TargetExtType::get(Ctx, "spirv.Queue");
    case BuiltinType::OCLReserveID:
      return toolchain::TargetExtType::get(Ctx, "spirv.ReserveId");
#define INTEL_SUBGROUP_AVC_TYPE(Name, Id)                                      \
    case BuiltinType::OCLIntelSubgroupAVC##Id:                                 \
      return toolchain::TargetExtType::get(Ctx, "spirv.Avc" #Id "INTEL");
#include "language/Core/Basic/OpenCLExtensionTypes.def"
    default:
      return nullptr;
    }
  }

  return nullptr;
}

// Gets a spirv.IntegralConstant or spirv.Literal. If IntegralType is present,
// returns an IntegralConstant, otherwise returns a Literal.
static toolchain::Type *getInlineSpirvConstant(CodeGenModule &CGM,
                                          toolchain::Type *IntegralType,
                                          toolchain::APInt Value) {
  toolchain::LLVMContext &Ctx = CGM.getLLVMContext();

  // Convert the APInt value to an array of uint32_t words
  toolchain::SmallVector<uint32_t> Words;

  while (Value.ugt(0)) {
    uint32_t Word = Value.trunc(32).getZExtValue();
    Value.lshrInPlace(32);

    Words.push_back(Word);
  }
  if (Words.size() == 0)
    Words.push_back(0);

  if (IntegralType)
    return toolchain::TargetExtType::get(Ctx, "spirv.IntegralConstant",
                                    {IntegralType}, Words);
  return toolchain::TargetExtType::get(Ctx, "spirv.Literal", {}, Words);
}

static toolchain::Type *getInlineSpirvType(CodeGenModule &CGM,
                                      const HLSLInlineSpirvType *SpirvType) {
  toolchain::LLVMContext &Ctx = CGM.getLLVMContext();

  toolchain::SmallVector<toolchain::Type *> Operands;

  for (auto &Operand : SpirvType->getOperands()) {
    using SpirvOperandKind = SpirvOperand::SpirvOperandKind;

    toolchain::Type *Result = nullptr;
    switch (Operand.getKind()) {
    case SpirvOperandKind::ConstantId: {
      toolchain::Type *IntegralType =
          CGM.getTypes().ConvertType(Operand.getResultType());

      Result = getInlineSpirvConstant(CGM, IntegralType, Operand.getValue());
      break;
    }
    case SpirvOperandKind::Literal: {
      Result = getInlineSpirvConstant(CGM, nullptr, Operand.getValue());
      break;
    }
    case SpirvOperandKind::TypeId: {
      QualType TypeOperand = Operand.getResultType();
      if (auto *RT = TypeOperand->getAs<RecordType>()) {
        auto *RD = RT->getOriginalDecl()->getDefinitionOrSelf();
        assert(RD->isCompleteDefinition() &&
               "Type completion should have been required in Sema");

        const FieldDecl *HandleField = RD->findFirstNamedDataMember();
        if (HandleField) {
          QualType ResourceType = HandleField->getType();
          if (ResourceType->getAs<HLSLAttributedResourceType>()) {
            TypeOperand = ResourceType;
          }
        }
      }
      Result = CGM.getTypes().ConvertType(TypeOperand);
      break;
    }
    default:
      toolchain_unreachable("HLSLInlineSpirvType had invalid operand!");
      break;
    }

    assert(Result);
    Operands.push_back(Result);
  }

  return toolchain::TargetExtType::get(Ctx, "spirv.Type", Operands,
                                  {SpirvType->getOpcode(), SpirvType->getSize(),
                                   SpirvType->getAlignment()});
}

toolchain::Type *CommonSPIRTargetCodeGenInfo::getHLSLType(
    CodeGenModule &CGM, const Type *Ty,
    const SmallVector<int32_t> *Packoffsets) const {
  toolchain::LLVMContext &Ctx = CGM.getLLVMContext();

  if (auto *SpirvType = dyn_cast<HLSLInlineSpirvType>(Ty))
    return getInlineSpirvType(CGM, SpirvType);

  auto *ResType = dyn_cast<HLSLAttributedResourceType>(Ty);
  if (!ResType)
    return nullptr;

  const HLSLAttributedResourceType::Attributes &ResAttrs = ResType->getAttrs();
  switch (ResAttrs.ResourceClass) {
  case toolchain::dxil::ResourceClass::UAV:
  case toolchain::dxil::ResourceClass::SRV: {
    // TypedBuffer and RawBuffer both need element type
    QualType ContainedTy = ResType->getContainedType();
    if (ContainedTy.isNull())
      return nullptr;

    assert(!ResAttrs.IsROV &&
           "Rasterizer order views not implemented for SPIR-V yet");

    if (!ResAttrs.RawBuffer) {
      // convert element type
      return getSPIRVImageTypeFromHLSLResource(ResAttrs, ContainedTy, CGM);
    }

    toolchain::Type *ElemType = CGM.getTypes().ConvertTypeForMem(ContainedTy);
    toolchain::ArrayType *RuntimeArrayType = toolchain::ArrayType::get(ElemType, 0);
    uint32_t StorageClass = /* StorageBuffer storage class */ 12;
    bool IsWritable = ResAttrs.ResourceClass == toolchain::dxil::ResourceClass::UAV;
    return toolchain::TargetExtType::get(Ctx, "spirv.VulkanBuffer",
                                    {RuntimeArrayType},
                                    {StorageClass, IsWritable});
  }
  case toolchain::dxil::ResourceClass::CBuffer: {
    QualType ContainedTy = ResType->getContainedType();
    if (ContainedTy.isNull() || !ContainedTy->isStructureType())
      return nullptr;

    toolchain::Type *BufferLayoutTy =
        HLSLBufferLayoutBuilder(CGM, "spirv.Layout")
            .createLayoutType(ContainedTy->getAsStructureType(), Packoffsets);
    uint32_t StorageClass = /* Uniform storage class */ 2;
    return toolchain::TargetExtType::get(Ctx, "spirv.VulkanBuffer", {BufferLayoutTy},
                                    {StorageClass, false});
    break;
  }
  case toolchain::dxil::ResourceClass::Sampler:
    return toolchain::TargetExtType::get(Ctx, "spirv.Sampler");
  }
  return nullptr;
}

toolchain::Type *CommonSPIRTargetCodeGenInfo::getSPIRVImageTypeFromHLSLResource(
    const HLSLAttributedResourceType::Attributes &attributes, QualType Ty,
    CodeGenModule &CGM) const {
  toolchain::LLVMContext &Ctx = CGM.getLLVMContext();

  Ty = Ty->getCanonicalTypeUnqualified();
  if (const VectorType *V = dyn_cast<VectorType>(Ty))
    Ty = V->getElementType();
  assert(!Ty->isVectorType() && "We still have a vector type.");

  toolchain::Type *SampledType = CGM.getTypes().ConvertTypeForMem(Ty);

  assert((SampledType->isIntegerTy() || SampledType->isFloatingPointTy()) &&
         "The element type for a SPIR-V resource must be a scalar integer or "
         "floating point type.");

  // These parameters correspond to the operands to the OpTypeImage SPIR-V
  // instruction. See
  // https://registry.khronos.org/SPIR-V/specs/unified1/SPIRV.html#OpTypeImage.
  SmallVector<unsigned, 6> IntParams(6, 0);

  const char *Name =
      Ty->isSignedIntegerType() ? "spirv.SignedImage" : "spirv.Image";

  // Dim
  // For now we assume everything is a buffer.
  IntParams[0] = 5;

  // Depth
  // HLSL does not indicate if it is a depth texture or not, so we use unknown.
  IntParams[1] = 2;

  // Arrayed
  IntParams[2] = 0;

  // MS
  IntParams[3] = 0;

  // Sampled
  IntParams[4] =
      attributes.ResourceClass == toolchain::dxil::ResourceClass::UAV ? 2 : 1;

  // Image format.
  // Setting to unknown for now.
  IntParams[5] = 0;

  toolchain::TargetExtType *ImageType =
      toolchain::TargetExtType::get(Ctx, Name, {SampledType}, IntParams);
  return ImageType;
}

std::unique_ptr<TargetCodeGenInfo>
CodeGen::createCommonSPIRTargetCodeGenInfo(CodeGenModule &CGM) {
  return std::make_unique<CommonSPIRTargetCodeGenInfo>(CGM.getTypes());
}

std::unique_ptr<TargetCodeGenInfo>
CodeGen::createSPIRVTargetCodeGenInfo(CodeGenModule &CGM) {
  return std::make_unique<SPIRVTargetCodeGenInfo>(CGM.getTypes());
}
