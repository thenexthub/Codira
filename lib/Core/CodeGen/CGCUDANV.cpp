//===----- CGCUDANV.cpp - Interface to NVIDIA CUDA Runtime ----------------===//
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
// This provides a class for CUDA code generation targeting the NVIDIA CUDA
// runtime library.
//
//===----------------------------------------------------------------------===//

#include "CGCUDARuntime.h"
#include "CGCXXABI.h"
#include "CodeGenFunction.h"
#include "CodeGenModule.h"
#include "language/Core/AST/CharUnits.h"
#include "language/Core/AST/Decl.h"
#include "language/Core/Basic/Cuda.h"
#include "language/Core/CodeGen/CodeGenABITypes.h"
#include "language/Core/CodeGen/ConstantInitBuilder.h"
#include "toolchain/ADT/StringRef.h"
#include "toolchain/Frontend/Offloading/Utility.h"
#include "toolchain/IR/BasicBlock.h"
#include "toolchain/IR/Constants.h"
#include "toolchain/IR/DerivedTypes.h"
#include "toolchain/IR/ReplaceConstant.h"
#include "toolchain/Support/Format.h"
#include "toolchain/Support/VirtualFileSystem.h"

using namespace language::Core;
using namespace CodeGen;

namespace {
constexpr unsigned CudaFatMagic = 0x466243b1;
constexpr unsigned HIPFatMagic = 0x48495046; // "HIPF"

class CGNVCUDARuntime : public CGCUDARuntime {

  /// The prefix used for function calls and section names (CUDA, HIP, LLVM)
  StringRef Prefix;

private:
  toolchain::IntegerType *IntTy, *SizeTy;
  toolchain::Type *VoidTy;
  toolchain::PointerType *PtrTy;

  /// Convenience reference to LLVM Context
  toolchain::LLVMContext &Context;
  /// Convenience reference to the current module
  toolchain::Module &TheModule;
  /// Keeps track of kernel launch stubs and handles emitted in this module
  struct KernelInfo {
    toolchain::Function *Kernel; // stub function to help launch kernel
    const Decl *D;
  };
  toolchain::SmallVector<KernelInfo, 16> EmittedKernels;
  // Map a kernel mangled name to a symbol for identifying kernel in host code
  // For CUDA, the symbol for identifying the kernel is the same as the device
  // stub function. For HIP, they are different.
  toolchain::DenseMap<StringRef, toolchain::GlobalValue *> KernelHandles;
  // Map a kernel handle to the kernel stub.
  toolchain::DenseMap<toolchain::GlobalValue *, toolchain::Function *> KernelStubs;
  struct VarInfo {
    toolchain::GlobalVariable *Var;
    const VarDecl *D;
    DeviceVarFlags Flags;
  };
  toolchain::SmallVector<VarInfo, 16> DeviceVars;
  /// Keeps track of variable containing handle of GPU binary. Populated by
  /// ModuleCtorFunction() and used to create corresponding cleanup calls in
  /// ModuleDtorFunction()
  toolchain::GlobalVariable *GpuBinaryHandle = nullptr;
  /// Whether we generate relocatable device code.
  bool RelocatableDeviceCode;
  /// Mangle context for device.
  std::unique_ptr<MangleContext> DeviceMC;

  toolchain::FunctionCallee getSetupArgumentFn() const;
  toolchain::FunctionCallee getLaunchFn() const;

  toolchain::FunctionType *getRegisterGlobalsFnTy() const;
  toolchain::FunctionType *getCallbackFnTy() const;
  toolchain::FunctionType *getRegisterLinkedBinaryFnTy() const;
  std::string addPrefixToName(StringRef FuncName) const;
  std::string addUnderscoredPrefixToName(StringRef FuncName) const;

  /// Creates a function to register all kernel stubs generated in this module.
  toolchain::Function *makeRegisterGlobalsFn();

  /// Helper function that generates a constant string and returns a pointer to
  /// the start of the string.  The result of this function can be used anywhere
  /// where the C code specifies const char*.
  toolchain::Constant *makeConstantString(const std::string &Str,
                                     const std::string &Name = "") {
    return CGM.GetAddrOfConstantCString(Str, Name.c_str()).getPointer();
  }

  /// Helper function which generates an initialized constant array from Str,
  /// and optionally sets section name and alignment. AddNull specifies whether
  /// the array should nave NUL termination.
  toolchain::Constant *makeConstantArray(StringRef Str,
                                    StringRef Name = "",
                                    StringRef SectionName = "",
                                    unsigned Alignment = 0,
                                    bool AddNull = false) {
    toolchain::Constant *Value =
        toolchain::ConstantDataArray::getString(Context, Str, AddNull);
    auto *GV = new toolchain::GlobalVariable(
        TheModule, Value->getType(), /*isConstant=*/true,
        toolchain::GlobalValue::PrivateLinkage, Value, Name);
    if (!SectionName.empty()) {
      GV->setSection(SectionName);
      // Mark the address as used which make sure that this section isn't
      // merged and we will really have it in the object file.
      GV->setUnnamedAddr(toolchain::GlobalValue::UnnamedAddr::None);
    }
    if (Alignment)
      GV->setAlignment(toolchain::Align(Alignment));
    return GV;
  }

  /// Helper function that generates an empty dummy function returning void.
  toolchain::Function *makeDummyFunction(toolchain::FunctionType *FnTy) {
    assert(FnTy->getReturnType()->isVoidTy() &&
           "Can only generate dummy functions returning void!");
    toolchain::Function *DummyFunc = toolchain::Function::Create(
        FnTy, toolchain::GlobalValue::InternalLinkage, "dummy", &TheModule);

    toolchain::BasicBlock *DummyBlock =
        toolchain::BasicBlock::Create(Context, "", DummyFunc);
    CGBuilderTy FuncBuilder(CGM, Context);
    FuncBuilder.SetInsertPoint(DummyBlock);
    FuncBuilder.CreateRetVoid();

    return DummyFunc;
  }

  Address prepareKernelArgs(CodeGenFunction &CGF, FunctionArgList &Args);
  Address prepareKernelArgsLLVMOffload(CodeGenFunction &CGF,
                                       FunctionArgList &Args);
  void emitDeviceStubBodyLegacy(CodeGenFunction &CGF, FunctionArgList &Args);
  void emitDeviceStubBodyNew(CodeGenFunction &CGF, FunctionArgList &Args);
  std::string getDeviceSideName(const NamedDecl *ND) override;

  void registerDeviceVar(const VarDecl *VD, toolchain::GlobalVariable &Var,
                         bool Extern, bool Constant) {
    DeviceVars.push_back({&Var,
                          VD,
                          {DeviceVarFlags::Variable, Extern, Constant,
                           VD->hasAttr<HIPManagedAttr>(),
                           /*Normalized*/ false, 0}});
  }
  void registerDeviceSurf(const VarDecl *VD, toolchain::GlobalVariable &Var,
                          bool Extern, int Type) {
    DeviceVars.push_back({&Var,
                          VD,
                          {DeviceVarFlags::Surface, Extern, /*Constant*/ false,
                           /*Managed*/ false,
                           /*Normalized*/ false, Type}});
  }
  void registerDeviceTex(const VarDecl *VD, toolchain::GlobalVariable &Var,
                         bool Extern, int Type, bool Normalized) {
    DeviceVars.push_back({&Var,
                          VD,
                          {DeviceVarFlags::Texture, Extern, /*Constant*/ false,
                           /*Managed*/ false, Normalized, Type}});
  }

  /// Creates module constructor function
  toolchain::Function *makeModuleCtorFunction();
  /// Creates module destructor function
  toolchain::Function *makeModuleDtorFunction();
  /// Transform managed variables for device compilation.
  void transformManagedVars();
  /// Create offloading entries to register globals in RDC mode.
  void createOffloadingEntries();

public:
  CGNVCUDARuntime(CodeGenModule &CGM);

  toolchain::GlobalValue *getKernelHandle(toolchain::Function *F, GlobalDecl GD) override;
  toolchain::Function *getKernelStub(toolchain::GlobalValue *Handle) override {
    auto Loc = KernelStubs.find(Handle);
    assert(Loc != KernelStubs.end());
    return Loc->second;
  }
  void emitDeviceStub(CodeGenFunction &CGF, FunctionArgList &Args) override;
  void handleVarRegistration(const VarDecl *VD,
                             toolchain::GlobalVariable &Var) override;
  void
  internalizeDeviceSideVar(const VarDecl *D,
                           toolchain::GlobalValue::LinkageTypes &Linkage) override;

  toolchain::Function *finalizeModule() override;
};

} // end anonymous namespace

std::string CGNVCUDARuntime::addPrefixToName(StringRef FuncName) const {
  return (Prefix + FuncName).str();
}
std::string
CGNVCUDARuntime::addUnderscoredPrefixToName(StringRef FuncName) const {
  return ("__" + Prefix + FuncName).str();
}

static std::unique_ptr<MangleContext> InitDeviceMC(CodeGenModule &CGM) {
  // If the host and device have different C++ ABIs, mark it as the device
  // mangle context so that the mangling needs to retrieve the additional
  // device lambda mangling number instead of the regular host one.
  if (CGM.getContext().getAuxTargetInfo() &&
      CGM.getContext().getTargetInfo().getCXXABI().isMicrosoft() &&
      CGM.getContext().getAuxTargetInfo()->getCXXABI().isItaniumFamily()) {
    return std::unique_ptr<MangleContext>(
        CGM.getContext().createDeviceMangleContext(
            *CGM.getContext().getAuxTargetInfo()));
  }

  return std::unique_ptr<MangleContext>(CGM.getContext().createMangleContext(
      CGM.getContext().getAuxTargetInfo()));
}

CGNVCUDARuntime::CGNVCUDARuntime(CodeGenModule &CGM)
    : CGCUDARuntime(CGM), Context(CGM.getLLVMContext()),
      TheModule(CGM.getModule()),
      RelocatableDeviceCode(CGM.getLangOpts().GPURelocatableDeviceCode),
      DeviceMC(InitDeviceMC(CGM)) {
  IntTy = CGM.IntTy;
  SizeTy = CGM.SizeTy;
  VoidTy = CGM.VoidTy;
  PtrTy = CGM.UnqualPtrTy;

  if (CGM.getLangOpts().OffloadViaLLVM)
    Prefix = "toolchain";
  else if (CGM.getLangOpts().HIP)
    Prefix = "hip";
  else
    Prefix = "cuda";
}

toolchain::FunctionCallee CGNVCUDARuntime::getSetupArgumentFn() const {
  // cudaError_t cudaSetupArgument(void *, size_t, size_t)
  toolchain::Type *Params[] = {PtrTy, SizeTy, SizeTy};
  return CGM.CreateRuntimeFunction(
      toolchain::FunctionType::get(IntTy, Params, false),
      addPrefixToName("SetupArgument"));
}

toolchain::FunctionCallee CGNVCUDARuntime::getLaunchFn() const {
  if (CGM.getLangOpts().HIP) {
    // hipError_t hipLaunchByPtr(char *);
    return CGM.CreateRuntimeFunction(
        toolchain::FunctionType::get(IntTy, PtrTy, false), "hipLaunchByPtr");
  }
  // cudaError_t cudaLaunch(char *);
  return CGM.CreateRuntimeFunction(toolchain::FunctionType::get(IntTy, PtrTy, false),
                                   "cudaLaunch");
}

toolchain::FunctionType *CGNVCUDARuntime::getRegisterGlobalsFnTy() const {
  return toolchain::FunctionType::get(VoidTy, PtrTy, false);
}

toolchain::FunctionType *CGNVCUDARuntime::getCallbackFnTy() const {
  return toolchain::FunctionType::get(VoidTy, PtrTy, false);
}

toolchain::FunctionType *CGNVCUDARuntime::getRegisterLinkedBinaryFnTy() const {
  toolchain::Type *Params[] = {toolchain::PointerType::getUnqual(Context), PtrTy, PtrTy,
                          toolchain::PointerType::getUnqual(Context)};
  return toolchain::FunctionType::get(VoidTy, Params, false);
}

std::string CGNVCUDARuntime::getDeviceSideName(const NamedDecl *ND) {
  GlobalDecl GD;
  // D could be either a kernel or a variable.
  if (auto *FD = dyn_cast<FunctionDecl>(ND))
    GD = GlobalDecl(FD, KernelReferenceKind::Kernel);
  else
    GD = GlobalDecl(ND);
  std::string DeviceSideName;
  MangleContext *MC;
  if (CGM.getLangOpts().CUDAIsDevice)
    MC = &CGM.getCXXABI().getMangleContext();
  else
    MC = DeviceMC.get();
  if (MC->shouldMangleDeclName(ND)) {
    SmallString<256> Buffer;
    toolchain::raw_svector_ostream Out(Buffer);
    MC->mangleName(GD, Out);
    DeviceSideName = std::string(Out.str());
  } else
    DeviceSideName = std::string(ND->getIdentifier()->getName());

  // Make unique name for device side static file-scope variable for HIP.
  if (CGM.getContext().shouldExternalize(ND) &&
      CGM.getLangOpts().GPURelocatableDeviceCode) {
    SmallString<256> Buffer;
    toolchain::raw_svector_ostream Out(Buffer);
    Out << DeviceSideName;
    CGM.printPostfixForExternalizedDecl(Out, ND);
    DeviceSideName = std::string(Out.str());
  }
  return DeviceSideName;
}

void CGNVCUDARuntime::emitDeviceStub(CodeGenFunction &CGF,
                                     FunctionArgList &Args) {
  EmittedKernels.push_back({CGF.CurFn, CGF.CurFuncDecl});
  if (auto *GV =
          dyn_cast<toolchain::GlobalVariable>(KernelHandles[CGF.CurFn->getName()])) {
    GV->setLinkage(CGF.CurFn->getLinkage());
    GV->setInitializer(CGF.CurFn);
  }
  if (CudaFeatureEnabled(CGM.getTarget().getSDKVersion(),
                         CudaFeature::CUDA_USES_NEW_LAUNCH) ||
      (CGF.getLangOpts().HIP && CGF.getLangOpts().HIPUseNewLaunchAPI) ||
      (CGF.getLangOpts().OffloadViaLLVM))
    emitDeviceStubBodyNew(CGF, Args);
  else
    emitDeviceStubBodyLegacy(CGF, Args);
}

/// CUDA passes the arguments with a level of indirection. For example, a
/// (void*, short, void*) is passed as {void **, short *, void **} to the launch
/// function. For the LLVM/offload launch we flatten the arguments into the
/// struct directly. In addition, we include the size of the arguments, thus
/// pass {sizeof({void *, short, void *}), ptr to {void *, short, void *},
/// nullptr}. The last nullptr needs to be initialized to an array of pointers
/// pointing to the arguments if we want to offload to the host.
Address CGNVCUDARuntime::prepareKernelArgsLLVMOffload(CodeGenFunction &CGF,
                                                      FunctionArgList &Args) {
  SmallVector<toolchain::Type *> ArgTypes, KernelLaunchParamsTypes;
  for (auto &Arg : Args)
    ArgTypes.push_back(CGF.ConvertTypeForMem(Arg->getType()));
  toolchain::StructType *KernelArgsTy = toolchain::StructType::create(ArgTypes);

  auto *Int64Ty = CGF.Builder.getInt64Ty();
  KernelLaunchParamsTypes.push_back(Int64Ty);
  KernelLaunchParamsTypes.push_back(PtrTy);
  KernelLaunchParamsTypes.push_back(PtrTy);

  toolchain::StructType *KernelLaunchParamsTy =
      toolchain::StructType::create(KernelLaunchParamsTypes);
  Address KernelArgs = CGF.CreateTempAllocaWithoutCast(
      KernelArgsTy, CharUnits::fromQuantity(16), "kernel_args");
  Address KernelLaunchParams = CGF.CreateTempAllocaWithoutCast(
      KernelLaunchParamsTy, CharUnits::fromQuantity(16),
      "kernel_launch_params");

  auto KernelArgsSize = CGM.getDataLayout().getTypeAllocSize(KernelArgsTy);
  CGF.Builder.CreateStore(toolchain::ConstantInt::get(Int64Ty, KernelArgsSize),
                          CGF.Builder.CreateStructGEP(KernelLaunchParams, 0));
  CGF.Builder.CreateStore(KernelArgs.emitRawPointer(CGF),
                          CGF.Builder.CreateStructGEP(KernelLaunchParams, 1));
  CGF.Builder.CreateStore(toolchain::Constant::getNullValue(PtrTy),
                          CGF.Builder.CreateStructGEP(KernelLaunchParams, 2));

  for (unsigned i = 0; i < Args.size(); ++i) {
    auto *ArgVal = CGF.Builder.CreateLoad(CGF.GetAddrOfLocalVar(Args[i]));
    CGF.Builder.CreateStore(ArgVal, CGF.Builder.CreateStructGEP(KernelArgs, i));
  }

  return KernelLaunchParams;
}

Address CGNVCUDARuntime::prepareKernelArgs(CodeGenFunction &CGF,
                                           FunctionArgList &Args) {
  // Calculate amount of space we will need for all arguments.  If we have no
  // args, allocate a single pointer so we still have a valid pointer to the
  // argument array that we can pass to runtime, even if it will be unused.
  Address KernelArgs = CGF.CreateTempAlloca(
      PtrTy, CharUnits::fromQuantity(16), "kernel_args",
      toolchain::ConstantInt::get(SizeTy, std::max<size_t>(1, Args.size())));
  // Store pointers to the arguments in a locally allocated launch_args.
  for (unsigned i = 0; i < Args.size(); ++i) {
    toolchain::Value *VarPtr = CGF.GetAddrOfLocalVar(Args[i]).emitRawPointer(CGF);
    toolchain::Value *VoidVarPtr = CGF.Builder.CreatePointerCast(VarPtr, PtrTy);
    CGF.Builder.CreateDefaultAlignedStore(
        VoidVarPtr, CGF.Builder.CreateConstGEP1_32(
                        PtrTy, KernelArgs.emitRawPointer(CGF), i));
  }
  return KernelArgs;
}

// CUDA 9.0+ uses new way to launch kernels. Parameters are packed in a local
// array and kernels are launched using cudaLaunchKernel().
void CGNVCUDARuntime::emitDeviceStubBodyNew(CodeGenFunction &CGF,
                                            FunctionArgList &Args) {
  // Build the shadow stack entry at the very start of the function.
  Address KernelArgs = CGF.getLangOpts().OffloadViaLLVM
                           ? prepareKernelArgsLLVMOffload(CGF, Args)
                           : prepareKernelArgs(CGF, Args);

  toolchain::BasicBlock *EndBlock = CGF.createBasicBlock("setup.end");

  // Lookup cudaLaunchKernel/hipLaunchKernel function.
  // HIP kernel launching API name depends on -fgpu-default-stream option. For
  // the default value 'legacy', it is hipLaunchKernel. For 'per-thread',
  // it is hipLaunchKernel_spt.
  // cudaError_t cudaLaunchKernel(const void *func, dim3 gridDim, dim3 blockDim,
  //                              void **args, size_t sharedMem,
  //                              cudaStream_t stream);
  // hipError_t hipLaunchKernel[_spt](const void *func, dim3 gridDim,
  //                                  dim3 blockDim, void **args,
  //                                  size_t sharedMem, hipStream_t stream);
  TranslationUnitDecl *TUDecl = CGM.getContext().getTranslationUnitDecl();
  DeclContext *DC = TranslationUnitDecl::castToDeclContext(TUDecl);
  std::string KernelLaunchAPI = "LaunchKernel";
  if (CGF.getLangOpts().GPUDefaultStream ==
      LangOptions::GPUDefaultStreamKind::PerThread) {
    if (CGF.getLangOpts().HIP)
      KernelLaunchAPI = KernelLaunchAPI + "_spt";
    else if (CGF.getLangOpts().CUDA)
      KernelLaunchAPI = KernelLaunchAPI + "_ptsz";
  }
  auto LaunchKernelName = addPrefixToName(KernelLaunchAPI);
  const IdentifierInfo &cudaLaunchKernelII =
      CGM.getContext().Idents.get(LaunchKernelName);
  FunctionDecl *cudaLaunchKernelFD = nullptr;
  for (auto *Result : DC->lookup(&cudaLaunchKernelII)) {
    if (FunctionDecl *FD = dyn_cast<FunctionDecl>(Result))
      cudaLaunchKernelFD = FD;
  }

  if (cudaLaunchKernelFD == nullptr) {
    CGM.Error(CGF.CurFuncDecl->getLocation(),
              "Can't find declaration for " + LaunchKernelName);
    return;
  }
  // Create temporary dim3 grid_dim, block_dim.
  ParmVarDecl *GridDimParam = cudaLaunchKernelFD->getParamDecl(1);
  QualType Dim3Ty = GridDimParam->getType();
  Address GridDim =
      CGF.CreateMemTemp(Dim3Ty, CharUnits::fromQuantity(8), "grid_dim");
  Address BlockDim =
      CGF.CreateMemTemp(Dim3Ty, CharUnits::fromQuantity(8), "block_dim");
  Address ShmemSize =
      CGF.CreateTempAlloca(SizeTy, CGM.getSizeAlign(), "shmem_size");
  Address Stream = CGF.CreateTempAlloca(PtrTy, CGM.getPointerAlign(), "stream");
  toolchain::FunctionCallee cudaPopConfigFn = CGM.CreateRuntimeFunction(
      toolchain::FunctionType::get(IntTy,
                              {/*gridDim=*/GridDim.getType(),
                               /*blockDim=*/BlockDim.getType(),
                               /*ShmemSize=*/ShmemSize.getType(),
                               /*Stream=*/Stream.getType()},
                              /*isVarArg=*/false),
      addUnderscoredPrefixToName("PopCallConfiguration"));

  CGF.EmitRuntimeCallOrInvoke(cudaPopConfigFn, {GridDim.emitRawPointer(CGF),
                                                BlockDim.emitRawPointer(CGF),
                                                ShmemSize.emitRawPointer(CGF),
                                                Stream.emitRawPointer(CGF)});

  // Emit the call to cudaLaunch
  toolchain::Value *Kernel =
      CGF.Builder.CreatePointerCast(KernelHandles[CGF.CurFn->getName()], PtrTy);
  CallArgList LaunchKernelArgs;
  LaunchKernelArgs.add(RValue::get(Kernel),
                       cudaLaunchKernelFD->getParamDecl(0)->getType());
  LaunchKernelArgs.add(RValue::getAggregate(GridDim), Dim3Ty);
  LaunchKernelArgs.add(RValue::getAggregate(BlockDim), Dim3Ty);
  LaunchKernelArgs.add(RValue::get(KernelArgs, CGF),
                       cudaLaunchKernelFD->getParamDecl(3)->getType());
  LaunchKernelArgs.add(RValue::get(CGF.Builder.CreateLoad(ShmemSize)),
                       cudaLaunchKernelFD->getParamDecl(4)->getType());
  LaunchKernelArgs.add(RValue::get(CGF.Builder.CreateLoad(Stream)),
                       cudaLaunchKernelFD->getParamDecl(5)->getType());

  QualType QT = cudaLaunchKernelFD->getType();
  QualType CQT = QT.getCanonicalType();
  toolchain::Type *Ty = CGM.getTypes().ConvertType(CQT);
  toolchain::FunctionType *FTy = cast<toolchain::FunctionType>(Ty);

  const CGFunctionInfo &FI =
      CGM.getTypes().arrangeFunctionDeclaration(cudaLaunchKernelFD);
  toolchain::FunctionCallee cudaLaunchKernelFn =
      CGM.CreateRuntimeFunction(FTy, LaunchKernelName);
  CGF.EmitCall(FI, CGCallee::forDirect(cudaLaunchKernelFn), ReturnValueSlot(),
               LaunchKernelArgs);

  // To prevent CUDA device stub functions from being merged by ICF in MSVC
  // environment, create an unique global variable for each kernel and write to
  // the variable in the device stub.
  if (CGM.getContext().getTargetInfo().getCXXABI().isMicrosoft() &&
      !CGF.getLangOpts().HIP) {
    toolchain::Function *KernelFunction = toolchain::cast<toolchain::Function>(Kernel);
    std::string GlobalVarName = (KernelFunction->getName() + ".id").str();

    toolchain::GlobalVariable *HandleVar =
        CGM.getModule().getNamedGlobal(GlobalVarName);
    if (!HandleVar) {
      HandleVar = new toolchain::GlobalVariable(
          CGM.getModule(), CGM.Int8Ty,
          /*Constant=*/false, KernelFunction->getLinkage(),
          toolchain::ConstantInt::get(CGM.Int8Ty, 0), GlobalVarName);
      HandleVar->setDSOLocal(KernelFunction->isDSOLocal());
      HandleVar->setVisibility(KernelFunction->getVisibility());
      if (KernelFunction->hasComdat())
        HandleVar->setComdat(CGM.getModule().getOrInsertComdat(GlobalVarName));
    }

    CGF.Builder.CreateAlignedStore(toolchain::ConstantInt::get(CGM.Int8Ty, 1),
                                   HandleVar, CharUnits::One(),
                                   /*IsVolatile=*/true);
  }

  CGF.EmitBranch(EndBlock);

  CGF.EmitBlock(EndBlock);
}

void CGNVCUDARuntime::emitDeviceStubBodyLegacy(CodeGenFunction &CGF,
                                               FunctionArgList &Args) {
  // Emit a call to cudaSetupArgument for each arg in Args.
  toolchain::FunctionCallee cudaSetupArgFn = getSetupArgumentFn();
  toolchain::BasicBlock *EndBlock = CGF.createBasicBlock("setup.end");
  CharUnits Offset = CharUnits::Zero();
  for (const VarDecl *A : Args) {
    auto TInfo = CGM.getContext().getTypeInfoInChars(A->getType());
    Offset = Offset.alignTo(TInfo.Align);
    toolchain::Value *Args[] = {
        CGF.Builder.CreatePointerCast(
            CGF.GetAddrOfLocalVar(A).emitRawPointer(CGF), PtrTy),
        toolchain::ConstantInt::get(SizeTy, TInfo.Width.getQuantity()),
        toolchain::ConstantInt::get(SizeTy, Offset.getQuantity()),
    };
    toolchain::CallBase *CB = CGF.EmitRuntimeCallOrInvoke(cudaSetupArgFn, Args);
    toolchain::Constant *Zero = toolchain::ConstantInt::get(IntTy, 0);
    toolchain::Value *CBZero = CGF.Builder.CreateICmpEQ(CB, Zero);
    toolchain::BasicBlock *NextBlock = CGF.createBasicBlock("setup.next");
    CGF.Builder.CreateCondBr(CBZero, NextBlock, EndBlock);
    CGF.EmitBlock(NextBlock);
    Offset += TInfo.Width;
  }

  // Emit the call to cudaLaunch
  toolchain::FunctionCallee cudaLaunchFn = getLaunchFn();
  toolchain::Value *Arg =
      CGF.Builder.CreatePointerCast(KernelHandles[CGF.CurFn->getName()], PtrTy);
  CGF.EmitRuntimeCallOrInvoke(cudaLaunchFn, Arg);
  CGF.EmitBranch(EndBlock);

  CGF.EmitBlock(EndBlock);
}

// Replace the original variable Var with the address loaded from variable
// ManagedVar populated by HIP runtime.
static void replaceManagedVar(toolchain::GlobalVariable *Var,
                              toolchain::GlobalVariable *ManagedVar) {
  SmallVector<SmallVector<toolchain::User *, 8>, 8> WorkList;
  for (auto &&VarUse : Var->uses()) {
    WorkList.push_back({VarUse.getUser()});
  }
  while (!WorkList.empty()) {
    auto &&WorkItem = WorkList.pop_back_val();
    auto *U = WorkItem.back();
    if (isa<toolchain::ConstantExpr>(U)) {
      for (auto &&UU : U->uses()) {
        WorkItem.push_back(UU.getUser());
        WorkList.push_back(WorkItem);
        WorkItem.pop_back();
      }
      continue;
    }
    if (auto *I = dyn_cast<toolchain::Instruction>(U)) {
      toolchain::Value *OldV = Var;
      toolchain::Instruction *NewV = new toolchain::LoadInst(
          Var->getType(), ManagedVar, "ld.managed", false,
          toolchain::Align(Var->getAlignment()), I->getIterator());
      WorkItem.pop_back();
      // Replace constant expressions directly or indirectly using the managed
      // variable with instructions.
      for (auto &&Op : WorkItem) {
        auto *CE = cast<toolchain::ConstantExpr>(Op);
        auto *NewInst = CE->getAsInstruction();
        NewInst->insertBefore(*I->getParent(), I->getIterator());
        NewInst->replaceUsesOfWith(OldV, NewV);
        OldV = CE;
        NewV = NewInst;
      }
      I->replaceUsesOfWith(OldV, NewV);
    } else {
      toolchain_unreachable("Invalid use of managed variable");
    }
  }
}

/// Creates a function that sets up state on the host side for CUDA objects that
/// have a presence on both the host and device sides. Specifically, registers
/// the host side of kernel functions and device global variables with the CUDA
/// runtime.
/// \code
/// void __cuda_register_globals(void** GpuBinaryHandle) {
///    __cudaRegisterFunction(GpuBinaryHandle,Kernel0,...);
///    ...
///    __cudaRegisterFunction(GpuBinaryHandle,KernelM,...);
///    __cudaRegisterVar(GpuBinaryHandle, GlobalVar0, ...);
///    ...
///    __cudaRegisterVar(GpuBinaryHandle, GlobalVarN, ...);
/// }
/// \endcode
toolchain::Function *CGNVCUDARuntime::makeRegisterGlobalsFn() {
  // No need to register anything
  if (EmittedKernels.empty() && DeviceVars.empty())
    return nullptr;

  toolchain::Function *RegisterKernelsFunc = toolchain::Function::Create(
      getRegisterGlobalsFnTy(), toolchain::GlobalValue::InternalLinkage,
      addUnderscoredPrefixToName("_register_globals"), &TheModule);
  toolchain::BasicBlock *EntryBB =
      toolchain::BasicBlock::Create(Context, "entry", RegisterKernelsFunc);
  CGBuilderTy Builder(CGM, Context);
  Builder.SetInsertPoint(EntryBB);

  // void __cudaRegisterFunction(void **, const char *, char *, const char *,
  //                             int, uint3*, uint3*, dim3*, dim3*, int*)
  toolchain::Type *RegisterFuncParams[] = {
      PtrTy, PtrTy, PtrTy, PtrTy, IntTy,
      PtrTy, PtrTy, PtrTy, PtrTy, toolchain::PointerType::getUnqual(Context)};
  toolchain::FunctionCallee RegisterFunc = CGM.CreateRuntimeFunction(
      toolchain::FunctionType::get(IntTy, RegisterFuncParams, false),
      addUnderscoredPrefixToName("RegisterFunction"));

  // Extract GpuBinaryHandle passed as the first argument passed to
  // __cuda_register_globals() and generate __cudaRegisterFunction() call for
  // each emitted kernel.
  toolchain::Argument &GpuBinaryHandlePtr = *RegisterKernelsFunc->arg_begin();
  for (auto &&I : EmittedKernels) {
    toolchain::Constant *KernelName =
        makeConstantString(getDeviceSideName(cast<NamedDecl>(I.D)));
    toolchain::Constant *NullPtr = toolchain::ConstantPointerNull::get(PtrTy);
    toolchain::Value *Args[] = {
        &GpuBinaryHandlePtr,
        KernelHandles[I.Kernel->getName()],
        KernelName,
        KernelName,
        toolchain::ConstantInt::get(IntTy, -1),
        NullPtr,
        NullPtr,
        NullPtr,
        NullPtr,
        toolchain::ConstantPointerNull::get(toolchain::PointerType::getUnqual(Context))};
    Builder.CreateCall(RegisterFunc, Args);
  }

  toolchain::Type *VarSizeTy = IntTy;
  // For HIP or CUDA 9.0+, device variable size is type of `size_t`.
  if (CGM.getLangOpts().HIP ||
      ToCudaVersion(CGM.getTarget().getSDKVersion()) >= CudaVersion::CUDA_90)
    VarSizeTy = SizeTy;

  // void __cudaRegisterVar(void **, char *, char *, const char *,
  //                        int, int, int, int)
  toolchain::Type *RegisterVarParams[] = {PtrTy, PtrTy,     PtrTy, PtrTy,
                                     IntTy, VarSizeTy, IntTy, IntTy};
  toolchain::FunctionCallee RegisterVar = CGM.CreateRuntimeFunction(
      toolchain::FunctionType::get(VoidTy, RegisterVarParams, false),
      addUnderscoredPrefixToName("RegisterVar"));
  // void __hipRegisterManagedVar(void **, char *, char *, const char *,
  //                              size_t, unsigned)
  toolchain::Type *RegisterManagedVarParams[] = {PtrTy, PtrTy,     PtrTy,
                                            PtrTy, VarSizeTy, IntTy};
  toolchain::FunctionCallee RegisterManagedVar = CGM.CreateRuntimeFunction(
      toolchain::FunctionType::get(VoidTy, RegisterManagedVarParams, false),
      addUnderscoredPrefixToName("RegisterManagedVar"));
  // void __cudaRegisterSurface(void **, const struct surfaceReference *,
  //                            const void **, const char *, int, int);
  toolchain::FunctionCallee RegisterSurf = CGM.CreateRuntimeFunction(
      toolchain::FunctionType::get(
          VoidTy, {PtrTy, PtrTy, PtrTy, PtrTy, IntTy, IntTy}, false),
      addUnderscoredPrefixToName("RegisterSurface"));
  // void __cudaRegisterTexture(void **, const struct textureReference *,
  //                            const void **, const char *, int, int, int)
  toolchain::FunctionCallee RegisterTex = CGM.CreateRuntimeFunction(
      toolchain::FunctionType::get(
          VoidTy, {PtrTy, PtrTy, PtrTy, PtrTy, IntTy, IntTy, IntTy}, false),
      addUnderscoredPrefixToName("RegisterTexture"));
  for (auto &&Info : DeviceVars) {
    toolchain::GlobalVariable *Var = Info.Var;
    assert((!Var->isDeclaration() || Info.Flags.isManaged()) &&
           "External variables should not show up here, except HIP managed "
           "variables");
    toolchain::Constant *VarName = makeConstantString(getDeviceSideName(Info.D));
    switch (Info.Flags.getKind()) {
    case DeviceVarFlags::Variable: {
      uint64_t VarSize =
          CGM.getDataLayout().getTypeAllocSize(Var->getValueType());
      if (Info.Flags.isManaged()) {
        assert(Var->getName().ends_with(".managed") &&
               "HIP managed variables not transformed");
        auto *ManagedVar = CGM.getModule().getNamedGlobal(
            Var->getName().drop_back(StringRef(".managed").size()));
        toolchain::Value *Args[] = {
            &GpuBinaryHandlePtr,
            ManagedVar,
            Var,
            VarName,
            toolchain::ConstantInt::get(VarSizeTy, VarSize),
            toolchain::ConstantInt::get(IntTy, Var->getAlignment())};
        if (!Var->isDeclaration())
          Builder.CreateCall(RegisterManagedVar, Args);
      } else {
        toolchain::Value *Args[] = {
            &GpuBinaryHandlePtr,
            Var,
            VarName,
            VarName,
            toolchain::ConstantInt::get(IntTy, Info.Flags.isExtern()),
            toolchain::ConstantInt::get(VarSizeTy, VarSize),
            toolchain::ConstantInt::get(IntTy, Info.Flags.isConstant()),
            toolchain::ConstantInt::get(IntTy, 0)};
        Builder.CreateCall(RegisterVar, Args);
      }
      break;
    }
    case DeviceVarFlags::Surface:
      Builder.CreateCall(
          RegisterSurf,
          {&GpuBinaryHandlePtr, Var, VarName, VarName,
           toolchain::ConstantInt::get(IntTy, Info.Flags.getSurfTexType()),
           toolchain::ConstantInt::get(IntTy, Info.Flags.isExtern())});
      break;
    case DeviceVarFlags::Texture:
      Builder.CreateCall(
          RegisterTex,
          {&GpuBinaryHandlePtr, Var, VarName, VarName,
           toolchain::ConstantInt::get(IntTy, Info.Flags.getSurfTexType()),
           toolchain::ConstantInt::get(IntTy, Info.Flags.isNormalized()),
           toolchain::ConstantInt::get(IntTy, Info.Flags.isExtern())});
      break;
    }
  }

  Builder.CreateRetVoid();
  return RegisterKernelsFunc;
}

/// Creates a global constructor function for the module:
///
/// For CUDA:
/// \code
/// void __cuda_module_ctor() {
///     Handle = __cudaRegisterFatBinary(GpuBinaryBlob);
///     __cuda_register_globals(Handle);
/// }
/// \endcode
///
/// For HIP:
/// \code
/// void __hip_module_ctor() {
///     if (__hip_gpubin_handle == 0) {
///         __hip_gpubin_handle  = __hipRegisterFatBinary(GpuBinaryBlob);
///         __hip_register_globals(__hip_gpubin_handle);
///     }
/// }
/// \endcode
toolchain::Function *CGNVCUDARuntime::makeModuleCtorFunction() {
  bool IsHIP = CGM.getLangOpts().HIP;
  bool IsCUDA = CGM.getLangOpts().CUDA;
  // No need to generate ctors/dtors if there is no GPU binary.
  StringRef CudaGpuBinaryFileName = CGM.getCodeGenOpts().CudaGpuBinaryFileName;
  if (CudaGpuBinaryFileName.empty() && !IsHIP)
    return nullptr;
  if ((IsHIP || (IsCUDA && !RelocatableDeviceCode)) && EmittedKernels.empty() &&
      DeviceVars.empty())
    return nullptr;

  // void __{cuda|hip}_register_globals(void* handle);
  toolchain::Function *RegisterGlobalsFunc = makeRegisterGlobalsFn();
  // We always need a function to pass in as callback. Create a dummy
  // implementation if we don't need to register anything.
  if (RelocatableDeviceCode && !RegisterGlobalsFunc)
    RegisterGlobalsFunc = makeDummyFunction(getRegisterGlobalsFnTy());

  // void ** __{cuda|hip}RegisterFatBinary(void *);
  toolchain::FunctionCallee RegisterFatbinFunc = CGM.CreateRuntimeFunction(
      toolchain::FunctionType::get(PtrTy, PtrTy, false),
      addUnderscoredPrefixToName("RegisterFatBinary"));
  // struct { int magic, int version, void * gpu_binary, void * dont_care };
  toolchain::StructType *FatbinWrapperTy =
      toolchain::StructType::get(IntTy, IntTy, PtrTy, PtrTy);

  // Register GPU binary with the CUDA runtime, store returned handle in a
  // global variable and save a reference in GpuBinaryHandle to be cleaned up
  // in destructor on exit. Then associate all known kernels with the GPU binary
  // handle so CUDA runtime can figure out what to call on the GPU side.
  std::unique_ptr<toolchain::MemoryBuffer> CudaGpuBinary = nullptr;
  if (!CudaGpuBinaryFileName.empty()) {
    auto VFS = CGM.getFileSystem();
    auto CudaGpuBinaryOrErr =
        VFS->getBufferForFile(CudaGpuBinaryFileName, -1, false);
    if (std::error_code EC = CudaGpuBinaryOrErr.getError()) {
      CGM.getDiags().Report(diag::err_cannot_open_file)
          << CudaGpuBinaryFileName << EC.message();
      return nullptr;
    }
    CudaGpuBinary = std::move(CudaGpuBinaryOrErr.get());
  }

  toolchain::Function *ModuleCtorFunc = toolchain::Function::Create(
      toolchain::FunctionType::get(VoidTy, false),
      toolchain::GlobalValue::InternalLinkage,
      addUnderscoredPrefixToName("_module_ctor"), &TheModule);
  toolchain::BasicBlock *CtorEntryBB =
      toolchain::BasicBlock::Create(Context, "entry", ModuleCtorFunc);
  CGBuilderTy CtorBuilder(CGM, Context);

  CtorBuilder.SetInsertPoint(CtorEntryBB);

  const char *FatbinConstantName;
  const char *FatbinSectionName;
  const char *ModuleIDSectionName;
  StringRef ModuleIDPrefix;
  toolchain::Constant *FatBinStr;
  unsigned FatMagic;
  if (IsHIP) {
    FatbinConstantName = ".hip_fatbin";
    FatbinSectionName = ".hipFatBinSegment";

    ModuleIDSectionName = "__hip_module_id";
    ModuleIDPrefix = "__hip_";

    if (CudaGpuBinary) {
      // If fatbin is available from early finalization, create a string
      // literal containing the fat binary loaded from the given file.
      const unsigned HIPCodeObjectAlign = 4096;
      FatBinStr = makeConstantArray(std::string(CudaGpuBinary->getBuffer()), "",
                                    FatbinConstantName, HIPCodeObjectAlign);
    } else {
      // If fatbin is not available, create an external symbol
      // __hip_fatbin in section .hip_fatbin. The external symbol is supposed
      // to contain the fat binary but will be populated somewhere else,
      // e.g. by lld through link script.
      FatBinStr = new toolchain::GlobalVariable(
          CGM.getModule(), CGM.Int8Ty,
          /*isConstant=*/true, toolchain::GlobalValue::ExternalLinkage, nullptr,
          "__hip_fatbin" + (CGM.getLangOpts().CUID.empty()
                                ? ""
                                : "_" + CGM.getContext().getCUIDHash()),
          nullptr, toolchain::GlobalVariable::NotThreadLocal);
      cast<toolchain::GlobalVariable>(FatBinStr)->setSection(FatbinConstantName);
    }

    FatMagic = HIPFatMagic;
  } else {
    if (RelocatableDeviceCode)
      FatbinConstantName = CGM.getTriple().isMacOSX()
                               ? "__NV_CUDA,__nv_relfatbin"
                               : "__nv_relfatbin";
    else
      FatbinConstantName =
          CGM.getTriple().isMacOSX() ? "__NV_CUDA,__nv_fatbin" : ".nv_fatbin";
    // NVIDIA's cuobjdump looks for fatbins in this section.
    FatbinSectionName =
        CGM.getTriple().isMacOSX() ? "__NV_CUDA,__fatbin" : ".nvFatBinSegment";

    ModuleIDSectionName = CGM.getTriple().isMacOSX()
                              ? "__NV_CUDA,__nv_module_id"
                              : "__nv_module_id";
    ModuleIDPrefix = "__nv_";

    // For CUDA, create a string literal containing the fat binary loaded from
    // the given file.
    FatBinStr = makeConstantArray(std::string(CudaGpuBinary->getBuffer()), "",
                                  FatbinConstantName, 8);
    FatMagic = CudaFatMagic;
  }

  // Create initialized wrapper structure that points to the loaded GPU binary
  ConstantInitBuilder Builder(CGM);
  auto Values = Builder.beginStruct(FatbinWrapperTy);
  // Fatbin wrapper magic.
  Values.addInt(IntTy, FatMagic);
  // Fatbin version.
  Values.addInt(IntTy, 1);
  // Data.
  Values.add(FatBinStr);
  // Unused in fatbin v1.
  Values.add(toolchain::ConstantPointerNull::get(PtrTy));
  toolchain::GlobalVariable *FatbinWrapper = Values.finishAndCreateGlobal(
      addUnderscoredPrefixToName("_fatbin_wrapper"), CGM.getPointerAlign(),
      /*constant*/ true);
  FatbinWrapper->setSection(FatbinSectionName);

  // There is only one HIP fat binary per linked module, however there are
  // multiple constructor functions. Make sure the fat binary is registered
  // only once. The constructor functions are executed by the dynamic loader
  // before the program gains control. The dynamic loader cannot execute the
  // constructor functions concurrently since doing that would not guarantee
  // thread safety of the loaded program. Therefore we can assume sequential
  // execution of constructor functions here.
  if (IsHIP) {
    auto Linkage = RelocatableDeviceCode ? toolchain::GlobalValue::ExternalLinkage
                                         : toolchain::GlobalValue::InternalLinkage;
    toolchain::BasicBlock *IfBlock =
        toolchain::BasicBlock::Create(Context, "if", ModuleCtorFunc);
    toolchain::BasicBlock *ExitBlock =
        toolchain::BasicBlock::Create(Context, "exit", ModuleCtorFunc);
    // The name, size, and initialization pattern of this variable is part
    // of HIP ABI.
    GpuBinaryHandle = new toolchain::GlobalVariable(
        TheModule, PtrTy, /*isConstant=*/false, Linkage,
        /*Initializer=*/
        !RelocatableDeviceCode ? toolchain::ConstantPointerNull::get(PtrTy)
                               : nullptr,
        "__hip_gpubin_handle" + (CGM.getLangOpts().CUID.empty()
                                     ? ""
                                     : "_" + CGM.getContext().getCUIDHash()));
    GpuBinaryHandle->setAlignment(CGM.getPointerAlign().getAsAlign());
    // Prevent the weak symbol in different shared libraries being merged.
    if (Linkage != toolchain::GlobalValue::InternalLinkage)
      GpuBinaryHandle->setVisibility(toolchain::GlobalValue::HiddenVisibility);
    Address GpuBinaryAddr(
        GpuBinaryHandle, PtrTy,
        CharUnits::fromQuantity(GpuBinaryHandle->getAlignment()));
    {
      auto *HandleValue = CtorBuilder.CreateLoad(GpuBinaryAddr);
      toolchain::Constant *Zero =
          toolchain::Constant::getNullValue(HandleValue->getType());
      toolchain::Value *EQZero = CtorBuilder.CreateICmpEQ(HandleValue, Zero);
      CtorBuilder.CreateCondBr(EQZero, IfBlock, ExitBlock);
    }
    {
      CtorBuilder.SetInsertPoint(IfBlock);
      // GpuBinaryHandle = __hipRegisterFatBinary(&FatbinWrapper);
      toolchain::CallInst *RegisterFatbinCall =
          CtorBuilder.CreateCall(RegisterFatbinFunc, FatbinWrapper);
      CtorBuilder.CreateStore(RegisterFatbinCall, GpuBinaryAddr);
      CtorBuilder.CreateBr(ExitBlock);
    }
    {
      CtorBuilder.SetInsertPoint(ExitBlock);
      // Call __hip_register_globals(GpuBinaryHandle);
      if (RegisterGlobalsFunc) {
        auto *HandleValue = CtorBuilder.CreateLoad(GpuBinaryAddr);
        CtorBuilder.CreateCall(RegisterGlobalsFunc, HandleValue);
      }
    }
  } else if (!RelocatableDeviceCode) {
    // Register binary with CUDA runtime. This is substantially different in
    // default mode vs. separate compilation!
    // GpuBinaryHandle = __cudaRegisterFatBinary(&FatbinWrapper);
    toolchain::CallInst *RegisterFatbinCall =
        CtorBuilder.CreateCall(RegisterFatbinFunc, FatbinWrapper);
    GpuBinaryHandle = new toolchain::GlobalVariable(
        TheModule, PtrTy, false, toolchain::GlobalValue::InternalLinkage,
        toolchain::ConstantPointerNull::get(PtrTy), "__cuda_gpubin_handle");
    GpuBinaryHandle->setAlignment(CGM.getPointerAlign().getAsAlign());
    CtorBuilder.CreateAlignedStore(RegisterFatbinCall, GpuBinaryHandle,
                                   CGM.getPointerAlign());

    // Call __cuda_register_globals(GpuBinaryHandle);
    if (RegisterGlobalsFunc)
      CtorBuilder.CreateCall(RegisterGlobalsFunc, RegisterFatbinCall);

    // Call __cudaRegisterFatBinaryEnd(Handle) if this CUDA version needs it.
    if (CudaFeatureEnabled(CGM.getTarget().getSDKVersion(),
                           CudaFeature::CUDA_USES_FATBIN_REGISTER_END)) {
      // void __cudaRegisterFatBinaryEnd(void **);
      toolchain::FunctionCallee RegisterFatbinEndFunc = CGM.CreateRuntimeFunction(
          toolchain::FunctionType::get(VoidTy, PtrTy, false),
          "__cudaRegisterFatBinaryEnd");
      CtorBuilder.CreateCall(RegisterFatbinEndFunc, RegisterFatbinCall);
    }
  } else {
    // Generate a unique module ID.
    SmallString<64> ModuleID;
    toolchain::raw_svector_ostream OS(ModuleID);
    OS << ModuleIDPrefix << toolchain::format("%" PRIx64, FatbinWrapper->getGUID());
    toolchain::Constant *ModuleIDConstant = makeConstantArray(
        std::string(ModuleID), "", ModuleIDSectionName, 32, /*AddNull=*/true);

    // Create an alias for the FatbinWrapper that nvcc will look for.
    toolchain::GlobalAlias::create(toolchain::GlobalValue::ExternalLinkage,
                              Twine("__fatbinwrap") + ModuleID, FatbinWrapper);

    // void __cudaRegisterLinkedBinary%ModuleID%(void (*)(void *), void *,
    // void *, void (*)(void **))
    SmallString<128> RegisterLinkedBinaryName("__cudaRegisterLinkedBinary");
    RegisterLinkedBinaryName += ModuleID;
    toolchain::FunctionCallee RegisterLinkedBinaryFunc = CGM.CreateRuntimeFunction(
        getRegisterLinkedBinaryFnTy(), RegisterLinkedBinaryName);

    assert(RegisterGlobalsFunc && "Expecting at least dummy function!");
    toolchain::Value *Args[] = {RegisterGlobalsFunc, FatbinWrapper, ModuleIDConstant,
                           makeDummyFunction(getCallbackFnTy())};
    CtorBuilder.CreateCall(RegisterLinkedBinaryFunc, Args);
  }

  // Create destructor and register it with atexit() the way NVCC does it. Doing
  // it during regular destructor phase worked in CUDA before 9.2 but results in
  // double-free in 9.2.
  if (toolchain::Function *CleanupFn = makeModuleDtorFunction()) {
    // extern "C" int atexit(void (*f)(void));
    toolchain::FunctionType *AtExitTy =
        toolchain::FunctionType::get(IntTy, CleanupFn->getType(), false);
    toolchain::FunctionCallee AtExitFunc =
        CGM.CreateRuntimeFunction(AtExitTy, "atexit", toolchain::AttributeList(),
                                  /*Local=*/true);
    CtorBuilder.CreateCall(AtExitFunc, CleanupFn);
  }

  CtorBuilder.CreateRetVoid();
  return ModuleCtorFunc;
}

/// Creates a global destructor function that unregisters the GPU code blob
/// registered by constructor.
///
/// For CUDA:
/// \code
/// void __cuda_module_dtor() {
///     __cudaUnregisterFatBinary(Handle);
/// }
/// \endcode
///
/// For HIP:
/// \code
/// void __hip_module_dtor() {
///     if (__hip_gpubin_handle) {
///         __hipUnregisterFatBinary(__hip_gpubin_handle);
///         __hip_gpubin_handle = 0;
///     }
/// }
/// \endcode
toolchain::Function *CGNVCUDARuntime::makeModuleDtorFunction() {
  // No need for destructor if we don't have a handle to unregister.
  if (!GpuBinaryHandle)
    return nullptr;

  // void __cudaUnregisterFatBinary(void ** handle);
  toolchain::FunctionCallee UnregisterFatbinFunc = CGM.CreateRuntimeFunction(
      toolchain::FunctionType::get(VoidTy, PtrTy, false),
      addUnderscoredPrefixToName("UnregisterFatBinary"));

  toolchain::Function *ModuleDtorFunc = toolchain::Function::Create(
      toolchain::FunctionType::get(VoidTy, false),
      toolchain::GlobalValue::InternalLinkage,
      addUnderscoredPrefixToName("_module_dtor"), &TheModule);

  toolchain::BasicBlock *DtorEntryBB =
      toolchain::BasicBlock::Create(Context, "entry", ModuleDtorFunc);
  CGBuilderTy DtorBuilder(CGM, Context);
  DtorBuilder.SetInsertPoint(DtorEntryBB);

  Address GpuBinaryAddr(
      GpuBinaryHandle, GpuBinaryHandle->getValueType(),
      CharUnits::fromQuantity(GpuBinaryHandle->getAlignment()));
  auto *HandleValue = DtorBuilder.CreateLoad(GpuBinaryAddr);
  // There is only one HIP fat binary per linked module, however there are
  // multiple destructor functions. Make sure the fat binary is unregistered
  // only once.
  if (CGM.getLangOpts().HIP) {
    toolchain::BasicBlock *IfBlock =
        toolchain::BasicBlock::Create(Context, "if", ModuleDtorFunc);
    toolchain::BasicBlock *ExitBlock =
        toolchain::BasicBlock::Create(Context, "exit", ModuleDtorFunc);
    toolchain::Constant *Zero = toolchain::Constant::getNullValue(HandleValue->getType());
    toolchain::Value *NEZero = DtorBuilder.CreateICmpNE(HandleValue, Zero);
    DtorBuilder.CreateCondBr(NEZero, IfBlock, ExitBlock);

    DtorBuilder.SetInsertPoint(IfBlock);
    DtorBuilder.CreateCall(UnregisterFatbinFunc, HandleValue);
    DtorBuilder.CreateStore(Zero, GpuBinaryAddr);
    DtorBuilder.CreateBr(ExitBlock);

    DtorBuilder.SetInsertPoint(ExitBlock);
  } else {
    DtorBuilder.CreateCall(UnregisterFatbinFunc, HandleValue);
  }
  DtorBuilder.CreateRetVoid();
  return ModuleDtorFunc;
}

CGCUDARuntime *CodeGen::CreateNVCUDARuntime(CodeGenModule &CGM) {
  return new CGNVCUDARuntime(CGM);
}

void CGNVCUDARuntime::internalizeDeviceSideVar(
    const VarDecl *D, toolchain::GlobalValue::LinkageTypes &Linkage) {
  // For -fno-gpu-rdc, host-side shadows of external declarations of device-side
  // global variables become internal definitions. These have to be internal in
  // order to prevent name conflicts with global host variables with the same
  // name in a different TUs.
  //
  // For -fgpu-rdc, the shadow variables should not be internalized because
  // they may be accessed by different TU.
  if (CGM.getLangOpts().GPURelocatableDeviceCode)
    return;

  // __shared__ variables are odd. Shadows do get created, but
  // they are not registered with the CUDA runtime, so they
  // can't really be used to access their device-side
  // counterparts. It's not clear yet whether it's nvcc's bug or
  // a feature, but we've got to do the same for compatibility.
  if (D->hasAttr<CUDADeviceAttr>() || D->hasAttr<CUDAConstantAttr>() ||
      D->hasAttr<CUDASharedAttr>() ||
      D->getType()->isCUDADeviceBuiltinSurfaceType() ||
      D->getType()->isCUDADeviceBuiltinTextureType()) {
    Linkage = toolchain::GlobalValue::InternalLinkage;
  }
}

void CGNVCUDARuntime::handleVarRegistration(const VarDecl *D,
                                            toolchain::GlobalVariable &GV) {
  if (D->hasAttr<CUDADeviceAttr>() || D->hasAttr<CUDAConstantAttr>()) {
    // Shadow variables and their properties must be registered with CUDA
    // runtime. Skip Extern global variables, which will be registered in
    // the TU where they are defined.
    //
    // Don't register a C++17 inline variable. The local symbol can be
    // discarded and referencing a discarded local symbol from outside the
    // comdat (__cuda_register_globals) is disallowed by the ELF spec.
    //
    // HIP managed variables need to be always recorded in device and host
    // compilations for transformation.
    //
    // HIP managed variables and variables in CUDADeviceVarODRUsedByHost are
    // added to toolchain.compiler-used, therefore they are safe to be registered.
    if ((!D->hasExternalStorage() && !D->isInline()) ||
        CGM.getContext().CUDADeviceVarODRUsedByHost.contains(D) ||
        D->hasAttr<HIPManagedAttr>()) {
      registerDeviceVar(D, GV, !D->hasDefinition(),
                        D->hasAttr<CUDAConstantAttr>());
    }
  } else if (D->getType()->isCUDADeviceBuiltinSurfaceType() ||
             D->getType()->isCUDADeviceBuiltinTextureType()) {
    // Builtin surfaces and textures and their template arguments are
    // also registered with CUDA runtime.
    const auto *TD = cast<ClassTemplateSpecializationDecl>(
                         D->getType()->castAs<RecordType>()->getOriginalDecl())
                         ->getDefinitionOrSelf();
    const TemplateArgumentList &Args = TD->getTemplateArgs();
    if (TD->hasAttr<CUDADeviceBuiltinSurfaceTypeAttr>()) {
      assert(Args.size() == 2 &&
             "Unexpected number of template arguments of CUDA device "
             "builtin surface type.");
      auto SurfType = Args[1].getAsIntegral();
      if (!D->hasExternalStorage())
        registerDeviceSurf(D, GV, !D->hasDefinition(), SurfType.getSExtValue());
    } else {
      assert(Args.size() == 3 &&
             "Unexpected number of template arguments of CUDA device "
             "builtin texture type.");
      auto TexType = Args[1].getAsIntegral();
      auto Normalized = Args[2].getAsIntegral();
      if (!D->hasExternalStorage())
        registerDeviceTex(D, GV, !D->hasDefinition(), TexType.getSExtValue(),
                          Normalized.getZExtValue());
    }
  }
}

// Transform managed variables to pointers to managed variables in device code.
// Each use of the original managed variable is replaced by a load from the
// transformed managed variable. The transformed managed variable contains
// the address of managed memory which will be allocated by the runtime.
void CGNVCUDARuntime::transformManagedVars() {
  for (auto &&Info : DeviceVars) {
    toolchain::GlobalVariable *Var = Info.Var;
    if (Info.Flags.getKind() == DeviceVarFlags::Variable &&
        Info.Flags.isManaged()) {
      auto *ManagedVar = new toolchain::GlobalVariable(
          CGM.getModule(), Var->getType(),
          /*isConstant=*/false, Var->getLinkage(),
          /*Init=*/Var->isDeclaration()
              ? nullptr
              : toolchain::ConstantPointerNull::get(Var->getType()),
          /*Name=*/"", /*InsertBefore=*/nullptr,
          toolchain::GlobalVariable::NotThreadLocal,
          CGM.getContext().getTargetAddressSpace(CGM.getLangOpts().CUDAIsDevice
                                                     ? LangAS::cuda_device
                                                     : LangAS::Default));
      ManagedVar->setDSOLocal(Var->isDSOLocal());
      ManagedVar->setVisibility(Var->getVisibility());
      ManagedVar->setExternallyInitialized(true);
      replaceManagedVar(Var, ManagedVar);
      ManagedVar->takeName(Var);
      Var->setName(Twine(ManagedVar->getName()) + ".managed");
      // Keep managed variables even if they are not used in device code since
      // they need to be allocated by the runtime.
      if (CGM.getLangOpts().CUDAIsDevice && !Var->isDeclaration()) {
        assert(!ManagedVar->isDeclaration());
        CGM.addCompilerUsedGlobal(Var);
        CGM.addCompilerUsedGlobal(ManagedVar);
      }
    }
  }
}

// Creates offloading entries for all the kernels and globals that must be
// registered. The linker will provide a pointer to this section so we can
// register the symbols with the linked device image.
void CGNVCUDARuntime::createOffloadingEntries() {
  toolchain::object::OffloadKind Kind = CGM.getLangOpts().HIP
                                       ? toolchain::object::OffloadKind::OFK_HIP
                                       : toolchain::object::OffloadKind::OFK_Cuda;
  // For now, just spoof this as OpenMP because that's the runtime it uses.
  if (CGM.getLangOpts().OffloadViaLLVM)
    Kind = toolchain::object::OffloadKind::OFK_OpenMP;

  toolchain::Module &M = CGM.getModule();
  for (KernelInfo &I : EmittedKernels)
    toolchain::offloading::emitOffloadingEntry(
        M, Kind, KernelHandles[I.Kernel->getName()],
        getDeviceSideName(cast<NamedDecl>(I.D)), /*Flags=*/0, /*Data=*/0,
        toolchain::offloading::OffloadGlobalEntry);

  for (VarInfo &I : DeviceVars) {
    uint64_t VarSize =
        CGM.getDataLayout().getTypeAllocSize(I.Var->getValueType());
    int32_t Flags =
        (I.Flags.isExtern()
             ? static_cast<int32_t>(toolchain::offloading::OffloadGlobalExtern)
             : 0) |
        (I.Flags.isConstant()
             ? static_cast<int32_t>(toolchain::offloading::OffloadGlobalConstant)
             : 0) |
        (I.Flags.isNormalized()
             ? static_cast<int32_t>(toolchain::offloading::OffloadGlobalNormalized)
             : 0);
    if (I.Flags.getKind() == DeviceVarFlags::Variable) {
      if (I.Flags.isManaged()) {
        assert(I.Var->getName().ends_with(".managed") &&
               "HIP managed variables not transformed");

        auto *ManagedVar = M.getNamedGlobal(
            I.Var->getName().drop_back(StringRef(".managed").size()));
        toolchain::offloading::emitOffloadingEntry(
            M, Kind, I.Var, getDeviceSideName(I.D), VarSize,
            toolchain::offloading::OffloadGlobalManagedEntry | Flags,
            /*Data=*/I.Var->getAlignment(), ManagedVar);
      } else {
        toolchain::offloading::emitOffloadingEntry(
            M, Kind, I.Var, getDeviceSideName(I.D), VarSize,
            toolchain::offloading::OffloadGlobalEntry | Flags,
            /*Data=*/0);
      }
    } else if (I.Flags.getKind() == DeviceVarFlags::Surface) {
      toolchain::offloading::emitOffloadingEntry(
          M, Kind, I.Var, getDeviceSideName(I.D), VarSize,
          toolchain::offloading::OffloadGlobalSurfaceEntry | Flags,
          I.Flags.getSurfTexType());
    } else if (I.Flags.getKind() == DeviceVarFlags::Texture) {
      toolchain::offloading::emitOffloadingEntry(
          M, Kind, I.Var, getDeviceSideName(I.D), VarSize,
          toolchain::offloading::OffloadGlobalTextureEntry | Flags,
          I.Flags.getSurfTexType());
    }
  }
}

// Returns module constructor to be added.
toolchain::Function *CGNVCUDARuntime::finalizeModule() {
  transformManagedVars();
  if (CGM.getLangOpts().CUDAIsDevice) {
    // Mark ODR-used device variables as compiler used to prevent it from being
    // eliminated by optimization. This is necessary for device variables
    // ODR-used by host functions. Sema correctly marks them as ODR-used no
    // matter whether they are ODR-used by device or host functions.
    //
    // We do not need to do this if the variable has used attribute since it
    // has already been added.
    //
    // Static device variables have been externalized at this point, therefore
    // variables with LLVM private or internal linkage need not be added.
    for (auto &&Info : DeviceVars) {
      auto Kind = Info.Flags.getKind();
      if (!Info.Var->isDeclaration() &&
          !toolchain::GlobalValue::isLocalLinkage(Info.Var->getLinkage()) &&
          (Kind == DeviceVarFlags::Variable ||
           Kind == DeviceVarFlags::Surface ||
           Kind == DeviceVarFlags::Texture) &&
          Info.D->isUsed() && !Info.D->hasAttr<UsedAttr>()) {
        CGM.addCompilerUsedGlobal(Info.Var);
      }
    }
    return nullptr;
  }
  if (CGM.getLangOpts().OffloadViaLLVM ||
      (CGM.getLangOpts().OffloadingNewDriver &&
       (CGM.getLangOpts().HIP || RelocatableDeviceCode)))
    createOffloadingEntries();
  else
    return makeModuleCtorFunction();

  return nullptr;
}

toolchain::GlobalValue *CGNVCUDARuntime::getKernelHandle(toolchain::Function *F,
                                                    GlobalDecl GD) {
  auto Loc = KernelHandles.find(F->getName());
  if (Loc != KernelHandles.end()) {
    auto OldHandle = Loc->second;
    if (KernelStubs[OldHandle] == F)
      return OldHandle;

    // We've found the function name, but F itself has changed, so we need to
    // update the references.
    if (CGM.getLangOpts().HIP) {
      // For HIP compilation the handle itself does not change, so we only need
      // to update the Stub value.
      KernelStubs[OldHandle] = F;
      return OldHandle;
    }
    // For non-HIP compilation, erase the old Stub and fall-through to creating
    // new entries.
    KernelStubs.erase(OldHandle);
  }

  if (!CGM.getLangOpts().HIP) {
    KernelHandles[F->getName()] = F;
    KernelStubs[F] = F;
    return F;
  }

  auto *Var = new toolchain::GlobalVariable(
      TheModule, F->getType(), /*isConstant=*/true, F->getLinkage(),
      /*Initializer=*/nullptr,
      CGM.getMangledName(
          GD.getWithKernelReferenceKind(KernelReferenceKind::Kernel)));
  Var->setAlignment(CGM.getPointerAlign().getAsAlign());
  Var->setDSOLocal(F->isDSOLocal());
  Var->setVisibility(F->getVisibility());
  auto *FD = cast<FunctionDecl>(GD.getDecl());
  auto *FT = FD->getPrimaryTemplate();
  if (!FT || FT->isThisDeclarationADefinition())
    CGM.maybeSetTrivialComdat(*FD, *Var);
  KernelHandles[F->getName()] = Var;
  KernelStubs[Var] = F;
  return Var;
}
