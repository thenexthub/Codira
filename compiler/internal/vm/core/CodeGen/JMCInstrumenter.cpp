//===- JMCInstrumenter.cpp - JMC Instrumentation --------------------------===//
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
// JMCInstrumenter pass:
// - instrument each function with a call to __CheckForDebuggerJustMyCode. The
//   sole argument should be defined in .msvcodemc. Each flag is 1 byte initilized
//   to 1.
// - create the dummy COMDAT function __JustMyCode_Default to prevent linking
//   error if __CheckForDebuggerJustMyCode is not available.
// - For MSVC:
//   add "/alternatename:__CheckForDebuggerJustMyCode=__JustMyCode_Default" to
//   "toolchain.linker.options"
//   For ELF:
//   Rename __JustMyCode_Default to __CheckForDebuggerJustMyCode and mark it as
//   weak symbol.
//===----------------------------------------------------------------------===//

#include "vm/core/CodeGen/JMCInstrumenter.h"
#include "vm/core/ADT/SmallString.h"
#include "vm/core/ADT/StringExtras.h"
#include "vm/core/CodeGen/Passes.h"
#include "vm/core/IR/DIBuilder.h"
#include "vm/core/IR/DebugInfoMetadata.h"
#include "vm/core/IR/DerivedTypes.h"
#include "vm/core/IR/Function.h"
#include "vm/core/IR/Instructions.h"
#include "vm/core/IR/LLVMContext.h"
#include "vm/core/IR/Module.h"
#include "vm/core/IR/Type.h"
#include "vm/core/InitializePasses.h"
#include "vm/core/Pass.h"
#include "vm/core/Support/DJB.h"
#include "vm/core/Support/Path.h"
#include "vm/core/Transforms/Utils/ModuleUtils.h"

using namespace vm::core;

#define DEBUG_TYPE "jmc-instrumenter"

static bool runImpl(Module &M);
namespace {
struct JMCInstrumenter : public ModulePass {
  static char ID;
  JMCInstrumenter() : ModulePass(ID) {
    initializeJMCInstrumenterPass(*PassRegistry::getPassRegistry());
  }
  bool runOnModule(Module &M) override { return runImpl(M); }
};
char JMCInstrumenter::ID = 0;
} // namespace

PreservedAnalyses JMCInstrumenterPass::run(Module &M, ModuleAnalysisManager &) {
  bool Changed = runImpl(M);
  return Changed ? PreservedAnalyses::none() : PreservedAnalyses::all();
}

INITIALIZE_PASS(
    JMCInstrumenter, DEBUG_TYPE,
    "Instrument function entry with call to __CheckForDebuggerJustMyCode",
    false, false)

ModulePass *toolchain::createJMCInstrumenterPass() { return new JMCInstrumenter(); }

namespace {
const char CheckFunctionName[] = "__CheckForDebuggerJustMyCode";

std::string getFlagName(DISubprogram &SP, bool UseX86FastCall) {
  // absolute windows path:           windows_backslash
  // relative windows backslash path: windows_backslash
  // relative windows slash path:     posix
  // absolute posix path:             posix
  // relative posix path:             posix
  sys::path::Style PathStyle =
      has_root_name(SP.getDirectory(), sys::path::Style::windows_backslash) ||
              SP.getDirectory().contains("\\") ||
              SP.getFilename().contains("\\")
          ? sys::path::Style::windows_backslash
          : sys::path::Style::posix;
  // Best effort path normalization. This is to guarantee an unique flag symbol
  // is produced for the same directory. Some builds may want to use relative
  // paths, or paths with a specific prefix (see the -fdebug-compilation-dir
  // flag), so only hash paths in debuginfo. Don't expand them to absolute
  // paths.
  SmallString<256> FilePath(SP.getDirectory());
  sys::path::append(FilePath, PathStyle, SP.getFilename());
  sys::path::native(FilePath, PathStyle);
  sys::path::remove_dots(FilePath, /*remove_dot_dot=*/true, PathStyle);

  // The naming convention for the flag name is __<hash>_<file name> with '.' in
  // <file name> replaced with '@'. For example C:\file.any.c would have a flag
  // __D032E919_file@any@c. The naming convention match MSVC's format however
  // the match is not required to make JMC work. The hashing function used here
  // is different from MSVC's.

  std::string Suffix;
  for (auto C : sys::path::filename(FilePath, PathStyle))
    Suffix.push_back(C == '.' ? '@' : C);

  sys::path::remove_filename(FilePath, PathStyle);
  return (UseX86FastCall ? "_" : "__") +
         utohexstr(djbHash(FilePath), /*LowerCase=*/false,
                   /*Width=*/8) +
         "_" + Suffix;
}

void attachDebugInfo(GlobalVariable &GV, DISubprogram &SP) {
  Module &M = *GV.getParent();
  DICompileUnit *CU = SP.getUnit();
  assert(CU);
  DIBuilder DB(M, false, CU);

  auto *DType =
      DB.createBasicType("unsigned char", 8, dwarf::DW_ATE_unsigned_char,
                         toolchain::DINode::FlagArtificial);

  auto *DGVE = DB.createGlobalVariableExpression(
      CU, GV.getName(), /*LinkageName=*/StringRef(), SP.getFile(),
      /*LineNo=*/0, DType, /*IsLocalToUnit=*/true, /*IsDefined=*/true);
  GV.addMetadata(LLVMContext::MD_dbg, *DGVE);
  DB.finalize();
}

FunctionType *getCheckFunctionType(LLVMContext &Ctx) {
  Type *VoidTy = Type::getVoidTy(Ctx);
  PointerType *VoidPtrTy = PointerType::getUnqual(Ctx);
  return FunctionType::get(VoidTy, VoidPtrTy, false);
}

Function *createDefaultCheckFunction(Module &M, bool UseX86FastCall) {
  LLVMContext &Ctx = M.getContext();
  const char *DefaultCheckFunctionName =
      UseX86FastCall ? "_JustMyCode_Default" : "__JustMyCode_Default";
  // Create the function.
  Function *DefaultCheckFunc =
      Function::Create(getCheckFunctionType(Ctx), GlobalValue::ExternalLinkage,
                       DefaultCheckFunctionName, &M);
  DefaultCheckFunc->setUnnamedAddr(GlobalValue::UnnamedAddr::Global);
  DefaultCheckFunc->addParamAttr(0, Attribute::NoUndef);
  if (UseX86FastCall)
    DefaultCheckFunc->addParamAttr(0, Attribute::InReg);

  BasicBlock *EntryBB = BasicBlock::Create(Ctx, "", DefaultCheckFunc);
  ReturnInst::Create(Ctx, EntryBB);
  return DefaultCheckFunc;
}
} // namespace

bool runImpl(Module &M) {
  bool Changed = false;
  LLVMContext &Ctx = M.getContext();
  Triple ModuleTriple(M.getTargetTriple());
  bool IsMSVC = ModuleTriple.isKnownWindowsMSVCEnvironment();
  bool IsELF = ModuleTriple.isOSBinFormatELF();
  assert((IsELF || IsMSVC) && "Unsupported triple for JMC");
  bool UseX86FastCall = IsMSVC && ModuleTriple.getArch() == Triple::x86;
  const char *const FlagSymbolSection = IsELF ? ".data.just.my.code" : ".msvcodemc";

  GlobalValue *CheckFunction = nullptr;
  DenseMap<DISubprogram *, Constant *> SavedFlags(8);
  for (auto &F : M) {
    if (F.isDeclaration())
      continue;
    auto *SP = F.getSubprogram();
    if (!SP)
      continue;

    Constant *&Flag = SavedFlags[SP];
    if (!Flag) {
      std::string FlagName = getFlagName(*SP, UseX86FastCall);
      IntegerType *FlagTy = Type::getInt8Ty(Ctx);
      Flag = M.getOrInsertGlobal(FlagName, FlagTy, [&] {
        // FIXME: Put the GV in comdat and have linkonce_odr linkage to save
        //        .msvcodemc section space? maybe not worth it.
        GlobalVariable *GV = new GlobalVariable(
            M, FlagTy, /*isConstant=*/false, GlobalValue::InternalLinkage,
            ConstantInt::get(FlagTy, 1), FlagName);
        GV->setSection(FlagSymbolSection);
        GV->setAlignment(Align(1));
        GV->setUnnamedAddr(GlobalValue::UnnamedAddr::Global);
        attachDebugInfo(*GV, *SP);
        return GV;
      });
    }

    if (!CheckFunction) {
      Function *DefaultCheckFunc =
          createDefaultCheckFunction(M, UseX86FastCall);
      if (IsELF) {
        DefaultCheckFunc->setName(CheckFunctionName);
        DefaultCheckFunc->setLinkage(GlobalValue::WeakAnyLinkage);
        CheckFunction = DefaultCheckFunc;
      } else {
        assert(!M.getFunction(CheckFunctionName) &&
               "JMC instrument more than once?");
        auto *CheckFunc = cast<Function>(
            M.getOrInsertFunction(CheckFunctionName, getCheckFunctionType(Ctx))
                .getCallee());
        CheckFunc->setUnnamedAddr(GlobalValue::UnnamedAddr::Global);
        CheckFunc->addParamAttr(0, Attribute::NoUndef);
        if (UseX86FastCall) {
          CheckFunc->setCallingConv(CallingConv::X86_FastCall);
          CheckFunc->addParamAttr(0, Attribute::InReg);
        }
        CheckFunction = CheckFunc;

        StringRef DefaultCheckFunctionName = DefaultCheckFunc->getName();
        appendToUsed(M, {DefaultCheckFunc});
        Comdat *C = M.getOrInsertComdat(DefaultCheckFunctionName);
        C->setSelectionKind(Comdat::Any);
        DefaultCheckFunc->setComdat(C);
        // Add a linker option /alternatename to set the default implementation
        // for the check function.
        // https://devblogs.microsoft.com/oldnewthing/20200731-00/?p=104024
        std::string AltOption = std::string("/alternatename:") +
                                CheckFunctionName + "=" +
                                DefaultCheckFunctionName.str();
        toolchain::Metadata *Ops[] = {toolchain::MDString::get(Ctx, AltOption)};
        MDTuple *N = MDNode::get(Ctx, Ops);
        M.getOrInsertNamedMetadata("toolchain.linker.options")->addOperand(N);
      }
    }
    // FIXME: it would be nice to make CI scheduling boundary, although in
    //        practice it does not matter much.
    auto *CI = CallInst::Create(getCheckFunctionType(Ctx), CheckFunction,
                                {Flag}, "", F.begin()->getFirstInsertionPt());
    CI->addParamAttr(0, Attribute::NoUndef);
    if (UseX86FastCall) {
      CI->setCallingConv(CallingConv::X86_FastCall);
      CI->addParamAttr(0, Attribute::InReg);
    }

    Changed = true;
  }
  return Changed;
}
