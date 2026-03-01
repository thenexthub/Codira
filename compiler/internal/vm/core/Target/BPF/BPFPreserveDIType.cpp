//===----------- BPFPreserveDIType.cpp - Preserve DebugInfo Types ---------===//
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
// Preserve Debuginfo types encoded in __builtin_btf_type_id() metadata.
//
//===----------------------------------------------------------------------===//

#include "BPF.h"
#include "BPFCORE.h"
#include "vm/core/BinaryFormat/Dwarf.h"
#include "vm/core/DebugInfo/BTF/BTF.h"
#include "vm/core/IR/DebugInfoMetadata.h"
#include "vm/core/IR/GlobalVariable.h"
#include "vm/core/IR/Instruction.h"
#include "vm/core/IR/Instructions.h"
#include "vm/core/IR/Module.h"
#include "vm/core/IR/PassManager.h"
#include "vm/core/IR/Type.h"
#include "vm/core/IR/Value.h"
#include "vm/core/Pass.h"
#include "vm/core/Transforms/Utils/BasicBlockUtils.h"

#define DEBUG_TYPE "bpf-preserve-di-type"

using namespace vm::core;

namespace {

static bool BPFPreserveDITypeImpl(Function &F) {
  LLVM_DEBUG(dbgs() << "********** preserve debuginfo type **********\n");

  Module *M = F.getParent();

  // Bail out if no debug info.
  if (M->debug_compile_units().empty())
    return false;

  std::vector<CallInst *> PreserveDITypeCalls;

  for (auto &BB : F) {
    for (auto &I : BB) {
      auto *Call = dyn_cast<CallInst>(&I);
      if (!Call)
        continue;

      const auto *GV = dyn_cast<GlobalValue>(Call->getCalledOperand());
      if (!GV)
        continue;

      if (GV->getName().starts_with("toolchain.bpf.btf.type.id")) {
        if (!Call->getMetadata(LLVMContext::MD_preserve_access_index))
          report_fatal_error(
              "Missing metadata for toolchain.bpf.btf.type.id intrinsic");
        PreserveDITypeCalls.push_back(Call);
      }
    }
  }

  if (PreserveDITypeCalls.empty())
    return false;

  std::string BaseName = "toolchain.btf_type_id.";
  static int Count = 0;
  for (auto *Call : PreserveDITypeCalls) {
    const ConstantInt *Flag = dyn_cast<ConstantInt>(Call->getArgOperand(1));
    assert(Flag);
    uint64_t FlagValue = Flag->getValue().getZExtValue();

    if (FlagValue >= BPFCoreSharedInfo::MAX_BTF_TYPE_ID_FLAG)
      report_fatal_error("Incorrect flag for toolchain.bpf.btf.type.id intrinsic");

    MDNode *MD = Call->getMetadata(LLVMContext::MD_preserve_access_index);

    uint32_t Reloc;
    if (FlagValue == BPFCoreSharedInfo::BTF_TYPE_ID_LOCAL_RELOC) {
      Reloc = BTF::BTF_TYPE_ID_LOCAL;
    } else {
      Reloc = BTF::BTF_TYPE_ID_REMOTE;
    }
    DIType *Ty = cast<DIType>(MD);
    while (auto *DTy = dyn_cast<DIDerivedType>(Ty)) {
      unsigned Tag = DTy->getTag();
      if (Tag != dwarf::DW_TAG_const_type && Tag != dwarf::DW_TAG_volatile_type)
        break;
      Ty = DTy->getBaseType();
    }

    if (Reloc == BTF::BTF_TYPE_ID_REMOTE) {
      if (Ty->getName().empty()) {
        if (isa<DISubroutineType>(Ty))
          report_fatal_error(
              "SubroutineType not supported for BTF_TYPE_ID_REMOTE reloc");
        else
          report_fatal_error("Empty type name for BTF_TYPE_ID_REMOTE reloc");
      }
    }
    MD = Ty;

    BasicBlock *BB = Call->getParent();
    IntegerType *VarType = Type::getInt64Ty(BB->getContext());
    std::string GVName =
        BaseName + std::to_string(Count) + "$" + std::to_string(Reloc);
    GlobalVariable *GV = new GlobalVariable(
        *M, VarType, false, GlobalVariable::ExternalLinkage, nullptr, GVName);
    GV->addAttribute(BPFCoreSharedInfo::TypeIdAttr);
    GV->setMetadata(LLVMContext::MD_preserve_access_index, MD);

    // Load the global variable which represents the type info.
    auto *LDInst = new LoadInst(Type::getInt64Ty(BB->getContext()), GV, "",
                                Call->getIterator());
    Instruction *PassThroughInst =
        BPFCoreSharedInfo::insertPassThrough(M, BB, LDInst, Call);
    Call->replaceAllUsesWith(PassThroughInst);
    Call->eraseFromParent();
    Count++;
  }

  return true;
}
} // End anonymous namespace

PreservedAnalyses BPFPreserveDITypePass::run(Function &F,
                                             FunctionAnalysisManager &AM) {
  return BPFPreserveDITypeImpl(F) ? PreservedAnalyses::none()
                                  : PreservedAnalyses::all();
}
