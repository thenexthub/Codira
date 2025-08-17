//===---- TargetInfo.cpp - Encapsulate target details -----------*- C++ -*-===//
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
// These classes wrap the information about a call or function
// definition used to handle ABI compliancy.
//
//===----------------------------------------------------------------------===//

#include "TargetInfo.h"
#include "ABIInfo.h"
#include "ABIInfoImpl.h"
#include "CodeGenFunction.h"
#include "language/Core/Basic/CodeGenOptions.h"
#include "language/Core/CodeGen/CGFunctionInfo.h"
#include "toolchain/ADT/StringExtras.h"
#include "toolchain/ADT/Twine.h"
#include "toolchain/IR/Function.h"
#include "toolchain/IR/Type.h"
#include "toolchain/Support/raw_ostream.h"

using namespace language::Core;
using namespace CodeGen;

LLVM_DUMP_METHOD void ABIArgInfo::dump() const {
  raw_ostream &OS = toolchain::errs();
  OS << "(ABIArgInfo Kind=";
  switch (TheKind) {
  case Direct:
    OS << "Direct Type=";
    if (toolchain::Type *Ty = getCoerceToType())
      Ty->print(OS);
    else
      OS << "null";
    break;
  case Extend:
    OS << "Extend";
    break;
  case Ignore:
    OS << "Ignore";
    break;
  case InAlloca:
    OS << "InAlloca Offset=" << getInAllocaFieldIndex();
    break;
  case Indirect:
    OS << "Indirect Align=" << getIndirectAlign().getQuantity()
       << " ByVal=" << getIndirectByVal()
       << " Realign=" << getIndirectRealign();
    break;
  case IndirectAliased:
    OS << "Indirect Align=" << getIndirectAlign().getQuantity()
       << " AadrSpace=" << getIndirectAddrSpace()
       << " Realign=" << getIndirectRealign();
    break;
  case Expand:
    OS << "Expand";
    break;
  case CoerceAndExpand:
    OS << "CoerceAndExpand Type=";
    getCoerceAndExpandType()->print(OS);
    break;
  }
  OS << ")\n";
}

TargetCodeGenInfo::TargetCodeGenInfo(std::unique_ptr<ABIInfo> Info)
    : Info(std::move(Info)) {}

TargetCodeGenInfo::~TargetCodeGenInfo() = default;

// If someone can figure out a general rule for this, that would be great.
// It's probably just doomed to be platform-dependent, though.
unsigned TargetCodeGenInfo::getSizeOfUnwindException() const {
  // Verified for:
  //   x86-64     FreeBSD, Linux, Darwin
  //   x86-32     FreeBSD, Linux, Darwin
  //   PowerPC    Linux
  //   ARM        Darwin (*not* EABI)
  //   AArch64    Linux
  return 32;
}

bool TargetCodeGenInfo::isNoProtoCallVariadic(const CallArgList &args,
                                     const FunctionNoProtoType *fnType) const {
  // The following conventions are known to require this to be false:
  //   x86_stdcall
  //   MIPS
  // For everything else, we just prefer false unless we opt out.
  return false;
}

void
TargetCodeGenInfo::getDependentLibraryOption(toolchain::StringRef Lib,
                                             toolchain::SmallString<24> &Opt) const {
  // This assumes the user is passing a library name like "rt" instead of a
  // filename like "librt.a/so", and that they don't care whether it's static or
  // dynamic.
  Opt = "-l";
  Opt += Lib;
}

unsigned TargetCodeGenInfo::getDeviceKernelCallingConv() const {
  if (getABIInfo().getContext().getLangOpts().OpenCL) {
    // Device kernels are called via an explicit runtime API with arguments,
    // such as set with clSetKernelArg() for OpenCL, not as normal
    // sub-functions. Return SPIR_KERNEL by default as the kernel calling
    // convention to ensure the fingerprint is fixed such way that each kernel
    // argument gets one matching argument in the produced kernel function
    // argument list to enable feasible implementation of clSetKernelArg() with
    // aggregates etc. In case we would use the default C calling conv here,
    // clSetKernelArg() might break depending on the target-specific
    // conventions; different targets might split structs passed as values
    // to multiple function arguments etc.
    return toolchain::CallingConv::SPIR_KERNEL;
  }
  toolchain_unreachable("Unknown kernel calling convention");
}

void TargetCodeGenInfo::setOCLKernelStubCallingConvention(
    const FunctionType *&FT) const {
  FT = getABIInfo().getContext().adjustFunctionType(
      FT, FT->getExtInfo().withCallingConv(CC_C));
}

toolchain::Constant *TargetCodeGenInfo::getNullPointer(const CodeGen::CodeGenModule &CGM,
    toolchain::PointerType *T, QualType QT) const {
  return toolchain::ConstantPointerNull::get(T);
}

LangAS TargetCodeGenInfo::getGlobalVarAddressSpace(CodeGenModule &CGM,
                                                   const VarDecl *D) const {
  assert(!CGM.getLangOpts().OpenCL &&
         !(CGM.getLangOpts().CUDA && CGM.getLangOpts().CUDAIsDevice) &&
         "Address space agnostic languages only");
  return D ? D->getType().getAddressSpace() : LangAS::Default;
}

toolchain::Value *TargetCodeGenInfo::performAddrSpaceCast(
    CodeGen::CodeGenFunction &CGF, toolchain::Value *Src, LangAS SrcAddr,
    toolchain::Type *DestTy, bool isNonNull) const {
  // Since target may map different address spaces in AST to the same address
  // space, an address space conversion may end up as a bitcast.
  if (auto *C = dyn_cast<toolchain::Constant>(Src))
    return performAddrSpaceCast(CGF.CGM, C, SrcAddr, DestTy);
  // Try to preserve the source's name to make IR more readable.
  return CGF.Builder.CreateAddrSpaceCast(
      Src, DestTy, Src->hasName() ? Src->getName() + ".ascast" : "");
}

toolchain::Constant *
TargetCodeGenInfo::performAddrSpaceCast(CodeGenModule &CGM, toolchain::Constant *Src,
                                        LangAS SrcAddr,
                                        toolchain::Type *DestTy) const {
  // Since target may map different address spaces in AST to the same address
  // space, an address space conversion may end up as a bitcast.
  return toolchain::ConstantExpr::getPointerCast(Src, DestTy);
}

toolchain::SyncScope::ID
TargetCodeGenInfo::getLLVMSyncScopeID(const LangOptions &LangOpts,
                                      SyncScope Scope,
                                      toolchain::AtomicOrdering Ordering,
                                      toolchain::LLVMContext &Ctx) const {
  return Ctx.getOrInsertSyncScopeID(""); /* default sync scope */
}

void TargetCodeGenInfo::addStackProbeTargetAttributes(
    const Decl *D, toolchain::GlobalValue *GV, CodeGen::CodeGenModule &CGM) const {
  if (toolchain::Function *Fn = dyn_cast_or_null<toolchain::Function>(GV)) {
    if (CGM.getCodeGenOpts().StackProbeSize != 4096)
      Fn->addFnAttr("stack-probe-size",
                    toolchain::utostr(CGM.getCodeGenOpts().StackProbeSize));
    if (CGM.getCodeGenOpts().NoStackArgProbe)
      Fn->addFnAttr("no-stack-arg-probe");
  }
}

/// Create an OpenCL kernel for an enqueued block.
///
/// The kernel has the same function type as the block invoke function. Its
/// name is the name of the block invoke function postfixed with "_kernel".
/// It simply calls the block invoke function then returns.
toolchain::Value *TargetCodeGenInfo::createEnqueuedBlockKernel(
    CodeGenFunction &CGF, toolchain::Function *Invoke, toolchain::Type *BlockTy) const {
  auto *InvokeFT = Invoke->getFunctionType();
  auto &C = CGF.getLLVMContext();
  std::string Name = Invoke->getName().str() + "_kernel";
  auto *FT = toolchain::FunctionType::get(toolchain::Type::getVoidTy(C),
                                     InvokeFT->params(), false);
  auto *F = toolchain::Function::Create(FT, toolchain::GlobalValue::ExternalLinkage, Name,
                                   &CGF.CGM.getModule());
  toolchain::CallingConv::ID KernelCC =
      CGF.getTypes().ClangCallConvToLLVMCallConv(CallingConv::CC_DeviceKernel);
  F->setCallingConv(KernelCC);

  toolchain::AttrBuilder KernelAttrs(C);

  // FIXME: This is missing setTargetAttributes
  CGF.CGM.addDefaultFunctionDefinitionAttributes(KernelAttrs);
  F->addFnAttrs(KernelAttrs);

  auto IP = CGF.Builder.saveIP();
  auto *BB = toolchain::BasicBlock::Create(C, "entry", F);
  auto &Builder = CGF.Builder;
  Builder.SetInsertPoint(BB);
  toolchain::SmallVector<toolchain::Value *, 2> Args(toolchain::make_pointer_range(F->args()));
  toolchain::CallInst *Call = Builder.CreateCall(Invoke, Args);
  Call->setCallingConv(Invoke->getCallingConv());

  Builder.CreateRetVoid();
  Builder.restoreIP(IP);
  return F;
}

void TargetCodeGenInfo::setBranchProtectionFnAttributes(
    const TargetInfo::BranchProtectionInfo &BPI, toolchain::Function &F) {
  // Called on already created and initialized function where attributes already
  // set from command line attributes but some might need to be removed as the
  // actual BPI is different.
  if (BPI.SignReturnAddr != LangOptions::SignReturnAddressScopeKind::None) {
    F.addFnAttr("sign-return-address", BPI.getSignReturnAddrStr());
    F.addFnAttr("sign-return-address-key", BPI.getSignKeyStr());
  } else {
    if (F.hasFnAttribute("sign-return-address"))
      F.removeFnAttr("sign-return-address");
    if (F.hasFnAttribute("sign-return-address-key"))
      F.removeFnAttr("sign-return-address-key");
  }

  auto AddRemoveAttributeAsSet = [&](bool Set, const StringRef &ModAttr) {
    if (Set)
      F.addFnAttr(ModAttr);
    else if (F.hasFnAttribute(ModAttr))
      F.removeFnAttr(ModAttr);
  };

  AddRemoveAttributeAsSet(BPI.BranchTargetEnforcement,
                          "branch-target-enforcement");
  AddRemoveAttributeAsSet(BPI.BranchProtectionPAuthLR,
                          "branch-protection-pauth-lr");
  AddRemoveAttributeAsSet(BPI.GuardedControlStack, "guarded-control-stack");
}

void TargetCodeGenInfo::initBranchProtectionFnAttributes(
    const TargetInfo::BranchProtectionInfo &BPI, toolchain::AttrBuilder &FuncAttrs) {
  // Only used for initializing attributes in the AttrBuilder, which will not
  // contain any of these attributes so no need to remove anything.
  if (BPI.SignReturnAddr != LangOptions::SignReturnAddressScopeKind::None) {
    FuncAttrs.addAttribute("sign-return-address", BPI.getSignReturnAddrStr());
    FuncAttrs.addAttribute("sign-return-address-key", BPI.getSignKeyStr());
  }
  if (BPI.BranchTargetEnforcement)
    FuncAttrs.addAttribute("branch-target-enforcement");
  if (BPI.BranchProtectionPAuthLR)
    FuncAttrs.addAttribute("branch-protection-pauth-lr");
  if (BPI.GuardedControlStack)
    FuncAttrs.addAttribute("guarded-control-stack");
}

void TargetCodeGenInfo::setPointerAuthFnAttributes(
    const PointerAuthOptions &Opts, toolchain::Function &F) {
  auto UpdateAttr = [&F](bool AttrShouldExist, StringRef AttrName) {
    if (AttrShouldExist && !F.hasFnAttribute(AttrName))
      F.addFnAttr(AttrName);
    if (!AttrShouldExist && F.hasFnAttribute(AttrName))
      F.removeFnAttr(AttrName);
  };
  UpdateAttr(Opts.ReturnAddresses, "ptrauth-returns");
  UpdateAttr((bool)Opts.FunctionPointers, "ptrauth-calls");
  UpdateAttr(Opts.AuthTraps, "ptrauth-auth-traps");
  UpdateAttr(Opts.IndirectGotos, "ptrauth-indirect-gotos");
  UpdateAttr(Opts.AArch64JumpTableHardening, "aarch64-jump-table-hardening");
}

void TargetCodeGenInfo::initPointerAuthFnAttributes(
    const PointerAuthOptions &Opts, toolchain::AttrBuilder &FuncAttrs) {
  if (Opts.ReturnAddresses)
    FuncAttrs.addAttribute("ptrauth-returns");
  if (Opts.FunctionPointers)
    FuncAttrs.addAttribute("ptrauth-calls");
  if (Opts.AuthTraps)
    FuncAttrs.addAttribute("ptrauth-auth-traps");
  if (Opts.IndirectGotos)
    FuncAttrs.addAttribute("ptrauth-indirect-gotos");
  if (Opts.AArch64JumpTableHardening)
    FuncAttrs.addAttribute("aarch64-jump-table-hardening");
}

namespace {
class DefaultTargetCodeGenInfo : public TargetCodeGenInfo {
public:
  DefaultTargetCodeGenInfo(CodeGen::CodeGenTypes &CGT)
      : TargetCodeGenInfo(std::make_unique<DefaultABIInfo>(CGT)) {}
};
} // namespace

std::unique_ptr<TargetCodeGenInfo>
CodeGen::createDefaultTargetCodeGenInfo(CodeGenModule &CGM) {
  return std::make_unique<DefaultTargetCodeGenInfo>(CGM.getTypes());
}
