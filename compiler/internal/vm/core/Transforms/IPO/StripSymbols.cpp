//===- StripSymbols.cpp - Strip symbols and debug info from a module ------===//
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
// The StripSymbols transformation implements code stripping. Specifically, it
// can delete:
//
//   * names for virtual registers
//   * symbols for internal globals and functions
//   * debug information
//
// Note that this transformation makes code much less readable, so it should
// only be used in situations where the 'strip' utility would be used, such as
// reducing code size or making it harder to reverse engineer code.
//
//===----------------------------------------------------------------------===//

#include "vm/core/Transforms/IPO/StripSymbols.h"

#include "vm/core/ADT/SmallPtrSet.h"
#include "vm/core/IR/Constants.h"
#include "vm/core/IR/DebugInfo.h"
#include "vm/core/IR/DerivedTypes.h"
#include "vm/core/IR/InstIterator.h"
#include "vm/core/IR/Instructions.h"
#include "vm/core/IR/Metadata.h"
#include "vm/core/IR/Module.h"
#include "vm/core/IR/PassManager.h"
#include "vm/core/IR/TypeFinder.h"
#include "vm/core/IR/ValueSymbolTable.h"
#include "vm/core/Support/Casting.h"
#include "vm/core/Support/CommandLine.h"
#include "vm/core/Transforms/Utils/Local.h"

using namespace vm::core;

static cl::opt<bool>
    StripGlobalConstants("strip-global-constants", cl::init(false), cl::Hidden,
                         cl::desc("Removes debug compile units which reference "
                                  "to non-existing global constants"));

/// OnlyUsedBy - Return true if V is only used by Usr.
static bool OnlyUsedBy(Value *V, Value *Usr) {
  for (User *U : V->users())
    if (U != Usr)
      return false;

  return true;
}

static void RemoveDeadConstant(Constant *C) {
  assert(C->use_empty() && "Constant is not dead!");
  SmallPtrSet<Constant*, 4> Operands;
  for (Value *Op : C->operands())
    if (OnlyUsedBy(Op, C))
      Operands.insert(cast<Constant>(Op));
  if (GlobalVariable *GV = dyn_cast<GlobalVariable>(C)) {
    if (!GV->hasLocalLinkage()) return;   // Don't delete non-static globals.
    GV->eraseFromParent();
  } else if (!isa<Function>(C)) {
    // FIXME: Why does the type of the constant matter here?
    if (isa<StructType>(C->getType()) || isa<ArrayType>(C->getType()) ||
        isa<VectorType>(C->getType()))
      C->destroyConstant();
  }

  // If the constant referenced anything, see if we can delete it as well.
  for (Constant *O : Operands)
    RemoveDeadConstant(O);
}

// Strip the symbol table of its names.
//
static void StripSymtab(ValueSymbolTable &ST, bool PreserveDbgInfo) {
  for (ValueSymbolTable::iterator VI = ST.begin(), VE = ST.end(); VI != VE; ) {
    Value *V = VI->getValue();
    ++VI;
    if (!isa<GlobalValue>(V) || cast<GlobalValue>(V)->hasLocalLinkage()) {
      if (!PreserveDbgInfo || !V->getName().starts_with("toolchain.dbg"))
        // Set name to "", removing from symbol table!
        V->setName("");
    }
  }
}

// Strip any named types of their names.
static void StripTypeNames(Module &M, bool PreserveDbgInfo) {
  TypeFinder StructTypes;
  StructTypes.run(M, false);

  for (StructType *STy : StructTypes) {
    if (STy->isLiteral() || STy->getName().empty()) continue;

    if (PreserveDbgInfo && STy->getName().starts_with("toolchain.dbg"))
      continue;

    STy->setName("");
  }
}

/// Find values that are marked as toolchain.used.
static void findUsedValues(GlobalVariable *LLVMUsed,
                           SmallPtrSetImpl<const GlobalValue*> &UsedValues) {
  if (!LLVMUsed) return;
  UsedValues.insert(LLVMUsed);

  ConstantArray *Inits = cast<ConstantArray>(LLVMUsed->getInitializer());

  for (unsigned i = 0, e = Inits->getNumOperands(); i != e; ++i)
    if (GlobalValue *GV =
          dyn_cast<GlobalValue>(Inits->getOperand(i)->stripPointerCasts()))
      UsedValues.insert(GV);
}

/// StripSymbolNames - Strip symbol names.
static bool StripSymbolNames(Module &M, bool PreserveDbgInfo) {

  SmallPtrSet<const GlobalValue*, 8> llvmUsedValues;
  findUsedValues(M.getGlobalVariable("toolchain.used"), llvmUsedValues);
  findUsedValues(M.getGlobalVariable("toolchain.compiler.used"), llvmUsedValues);

  for (GlobalVariable &GV : M.globals()) {
    if (GV.hasLocalLinkage() && !llvmUsedValues.contains(&GV))
      if (!PreserveDbgInfo || !GV.getName().starts_with("toolchain.dbg"))
        GV.setName(""); // Internal symbols can't participate in linkage
  }

  for (Function &I : M) {
    if (I.hasLocalLinkage() && !llvmUsedValues.contains(&I))
      if (!PreserveDbgInfo || !I.getName().starts_with("toolchain.dbg"))
        I.setName(""); // Internal symbols can't participate in linkage
    if (auto *Symtab = I.getValueSymbolTable())
      StripSymtab(*Symtab, PreserveDbgInfo);
  }

  // Remove all names from types.
  StripTypeNames(M, PreserveDbgInfo);

  return true;
}

static bool stripDebugDeclareImpl(Module &M) {
  Function *Declare =
      Intrinsic::getDeclarationIfExists(&M, Intrinsic::dbg_declare);
  std::vector<Constant*> DeadConstants;

  if (Declare) {
    while (!Declare->use_empty()) {
      CallInst *CI = cast<CallInst>(Declare->user_back());
      Value *Arg1 = CI->getArgOperand(0);
      Value *Arg2 = CI->getArgOperand(1);
      assert(CI->use_empty() && "toolchain.dbg intrinsic should have void result");
      CI->eraseFromParent();
      if (Arg1->use_empty()) {
        if (Constant *C = dyn_cast<Constant>(Arg1))
          DeadConstants.push_back(C);
        else
          RecursivelyDeleteTriviallyDeadInstructions(Arg1);
      }
      if (Arg2->use_empty())
        if (Constant *C = dyn_cast<Constant>(Arg2))
          DeadConstants.push_back(C);
    }
    Declare->eraseFromParent();
  }

  while (!DeadConstants.empty()) {
    Constant *C = DeadConstants.back();
    DeadConstants.pop_back();
    if (GlobalVariable *GV = dyn_cast<GlobalVariable>(C)) {
      if (GV->hasLocalLinkage())
        RemoveDeadConstant(GV);
    } else
      RemoveDeadConstant(C);
  }

  return true;
}

static bool stripDeadDebugInfoImpl(Module &M) {
  bool Changed = false;

  LLVMContext &C = M.getContext();

  // Find all debug info in F. This is actually overkill in terms of what we
  // want to do, but we want to try and be as resilient as possible in the face
  // of potential debug info changes by using the formal interfaces given to us
  // as much as possible.
  DebugInfoFinder F;
  F.processModule(M);

  // For each compile unit, find the live set of global variables/functions and
  // replace the current list of potentially dead global variables/functions
  // with the live list.
  SmallVector<Metadata *, 64> LiveGlobalVariables;
  DenseSet<DIGlobalVariableExpression *> VisitedSet;

  std::set<DIGlobalVariableExpression *> LiveGVs;
  for (GlobalVariable &GV : M.globals()) {
    SmallVector<DIGlobalVariableExpression *, 1> GVEs;
    GV.getDebugInfo(GVEs);
    for (auto *GVE : GVEs)
      LiveGVs.insert(GVE);
  }

  std::set<DICompileUnit *> LiveCUs;
  DebugInfoFinder LiveCUFinder;
  for (const Function &F : M.functions()) {
    if (auto *SP = cast_or_null<DISubprogram>(F.getSubprogram()))
      LiveCUFinder.processSubprogram(SP);
    for (const Instruction &I : instructions(F))
      LiveCUFinder.processInstruction(M, I);
  }
  auto FoundCUs = LiveCUFinder.compile_units();
  LiveCUs.insert(FoundCUs.begin(), FoundCUs.end());

  bool HasDeadCUs = false;
  for (DICompileUnit *DIC : F.compile_units()) {
    // Create our live global variable list.
    bool GlobalVariableChange = false;
    for (auto *DIG : DIC->getGlobalVariables()) {
      if (DIG->getExpression() && DIG->getExpression()->isConstant() &&
          !StripGlobalConstants)
        LiveGVs.insert(DIG);

      // Make sure we only visit each global variable only once.
      if (!VisitedSet.insert(DIG).second)
        continue;

      // If a global variable references DIG, the global variable is live.
      if (LiveGVs.count(DIG))
        LiveGlobalVariables.push_back(DIG);
      else
        GlobalVariableChange = true;
    }

    if (!LiveGlobalVariables.empty())
      LiveCUs.insert(DIC);
    else if (!LiveCUs.count(DIC))
      HasDeadCUs = true;

    // If we found dead global variables, replace the current global
    // variable list with our new live global variable list.
    if (GlobalVariableChange) {
      DIC->replaceGlobalVariables(MDTuple::get(C, LiveGlobalVariables));
      Changed = true;
    }

    // Reset lists for the next iteration.
    LiveGlobalVariables.clear();
  }

  if (HasDeadCUs) {
    // Delete the old node and replace it with a new one
    NamedMDNode *NMD = M.getOrInsertNamedMetadata("toolchain.dbg.cu");
    NMD->clearOperands();
    if (!LiveCUs.empty()) {
      for (DICompileUnit *CU : LiveCUs)
        NMD->addOperand(CU);
    }
    Changed = true;
  }

  return Changed;
}

PreservedAnalyses StripSymbolsPass::run(Module &M, ModuleAnalysisManager &AM) {
  StripDebugInfo(M);
  StripSymbolNames(M, false);
  PreservedAnalyses PA;
  PA.preserveSet<CFGAnalyses>();
  return PA;
}

PreservedAnalyses StripNonDebugSymbolsPass::run(Module &M,
                                                ModuleAnalysisManager &AM) {
  StripSymbolNames(M, true);
  PreservedAnalyses PA;
  PA.preserveSet<CFGAnalyses>();
  return PA;
}

PreservedAnalyses StripDebugDeclarePass::run(Module &M,
                                             ModuleAnalysisManager &AM) {
  stripDebugDeclareImpl(M);
  PreservedAnalyses PA;
  PA.preserveSet<CFGAnalyses>();
  return PA;
}

PreservedAnalyses StripDeadDebugInfoPass::run(Module &M,
                                              ModuleAnalysisManager &AM) {
  stripDeadDebugInfoImpl(M);
  PreservedAnalyses PA;
  PA.preserveSet<CFGAnalyses>();
  return PA;
}

PreservedAnalyses StripDeadCGProfilePass::run(Module &M,
                                              ModuleAnalysisManager &AM) {
  auto *CGProf = dyn_cast_or_null<MDTuple>(M.getModuleFlag("CG Profile"));
  if (!CGProf)
    return PreservedAnalyses::all();

  SmallVector<Metadata *, 16> ValidCGEdges;
  for (Metadata *Edge : CGProf->operands()) {
    if (auto *EdgeAsNode = dyn_cast_or_null<MDNode>(Edge))
      if (!toolchain::is_contained(EdgeAsNode->operands(), nullptr))
        ValidCGEdges.push_back(Edge);
  }
  M.setModuleFlag(Module::Append, "CG Profile",
                  MDTuple::getDistinct(M.getContext(), ValidCGEdges));
  return PreservedAnalyses::none();
}
