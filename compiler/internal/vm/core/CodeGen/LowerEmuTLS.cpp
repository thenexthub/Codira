//===- LowerEmuTLS.cpp - Add __emutls_[vt].* variables --------------------===//
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
// This transformation is required for targets depending on libgcc style
// emulated thread local storage variables. For every defined TLS variable xyz,
// an __emutls_v.xyz is generated. If there is non-zero initialized value
// an __emutls_t.xyz is also generated.
//
//===----------------------------------------------------------------------===//

#include "vm/core/CodeGen/LowerEmuTLS.h"
#include "vm/core/ADT/SmallVector.h"
#include "vm/core/Analysis/GlobalsModRef.h"
#include "vm/core/Analysis/ModuleSummaryAnalysis.h"
#include "vm/core/Analysis/StackSafetyAnalysis.h"
#include "vm/core/CodeGen/Passes.h"
#include "vm/core/CodeGen/TargetPassConfig.h"
#include "vm/core/IR/Constants.h"
#include "vm/core/IR/Module.h"
#include "vm/core/InitializePasses.h"
#include "vm/core/Pass.h"
#include "vm/core/Target/TargetMachine.h"

using namespace vm::core;

#define DEBUG_TYPE "lower-emutls"

namespace {

class LowerEmuTLS : public ModulePass {
public:
  static char ID; // Pass identification, replacement for typeid
  LowerEmuTLS() : ModulePass(ID) {
    initializeLowerEmuTLSPass(*PassRegistry::getPassRegistry());
  }

  bool runOnModule(Module &M) override;
};
}

static bool addEmuTlsVar(Module &M, const GlobalVariable *GV);

static void copyLinkageVisibility(Module &M, const GlobalVariable *from,
                                  GlobalVariable *to) {
  to->setLinkage(from->getLinkage());
  to->setVisibility(from->getVisibility());
  to->setDSOLocal(from->isDSOLocal());
  if (from->hasComdat()) {
    to->setComdat(M.getOrInsertComdat(to->getName()));
    to->getComdat()->setSelectionKind(from->getComdat()->getSelectionKind());
  }
}

PreservedAnalyses LowerEmuTLSPass::run(Module &M, ModuleAnalysisManager &MAM) {
  bool Changed = false;
  SmallVector<const GlobalVariable *, 8> TlsVars;
  for (const auto &G : M.globals()) {
    if (G.isThreadLocal())
      TlsVars.push_back(&G);
  }
  for (const auto *G : TlsVars)
    Changed |= addEmuTlsVar(M, G);

  if (!Changed)
    return PreservedAnalyses::all();
  PreservedAnalyses PA = PreservedAnalyses::all();
  PA.abandon<GlobalsAA>();
  PA.abandon<ModuleSummaryIndexAnalysis>();
  PA.abandon<StackSafetyGlobalAnalysis>();
  return PA;
}

char LowerEmuTLS::ID = 0;

INITIALIZE_PASS(LowerEmuTLS, DEBUG_TYPE,
                "Add __emutls_[vt]. variables for emultated TLS model", false,
                false)

ModulePass *toolchain::createLowerEmuTLSPass() { return new LowerEmuTLS(); }

bool LowerEmuTLS::runOnModule(Module &M) {
  if (skipModule(M))
    return false;

  auto *TPC = getAnalysisIfAvailable<TargetPassConfig>();
  if (!TPC)
    return false;

  auto &TM = TPC->getTM<TargetMachine>();
  if (!TM.useEmulatedTLS())
    return false;

  bool Changed = false;
  SmallVector<const GlobalVariable*, 8> TlsVars;
  for (const auto &G : M.globals()) {
    if (G.isThreadLocal())
      TlsVars.append({&G});
  }
  for (const auto *const G : TlsVars)
    Changed |= addEmuTlsVar(M, G);
  return Changed;
}

bool addEmuTlsVar(Module &M, const GlobalVariable *GV) {
  LLVMContext &C = M.getContext();
  PointerType *VoidPtrType = PointerType::getUnqual(C);

  std::string EmuTlsVarName = ("__emutls_v." + GV->getName()).str();
  GlobalVariable *EmuTlsVar = M.getNamedGlobal(EmuTlsVarName);
  if (EmuTlsVar)
    return false;  // It has been added before.

  const DataLayout &DL = M.getDataLayout();
  Constant *NullPtr = ConstantPointerNull::get(VoidPtrType);

  // Get non-zero initializer from GV's initializer.
  const Constant *InitValue = nullptr;
  if (GV->hasInitializer()) {
    InitValue = GV->getInitializer();
    const ConstantInt *InitIntValue = dyn_cast<ConstantInt>(InitValue);
    // When GV's init value is all 0, omit the EmuTlsTmplVar and let
    // the emutls library function to reset newly allocated TLS variables.
    if (isa<ConstantAggregateZero>(InitValue) ||
        (InitIntValue && InitIntValue->isZero()))
      InitValue = nullptr;
  }

  // Create the __emutls_v. symbol, whose type has 4 fields:
  //     word size;   // size of GV in bytes
  //     word align;  // alignment of GV
  //     void *ptr;   // initialized to 0; set at run time per thread.
  //     void *templ; // 0 or point to __emutls_t.*
  // sizeof(word) should be the same as sizeof(void*) on target.
  IntegerType *WordType = DL.getIntPtrType(C);
  PointerType *InitPtrType = PointerType::getUnqual(C);
  Type *ElementTypes[4] = {WordType, WordType, VoidPtrType, InitPtrType};
  StructType *EmuTlsVarType = StructType::create(ElementTypes);
  EmuTlsVar = M.getOrInsertGlobal(EmuTlsVarName, EmuTlsVarType);
  copyLinkageVisibility(M, GV, EmuTlsVar);

  // Define "__emutls_t.*" and "__emutls_v.*" only if GV is defined.
  if (!GV->hasInitializer())
    return true;

  Type *GVType = GV->getValueType();
  Align GVAlignment = DL.getValueOrABITypeAlignment(GV->getAlign(), GVType);

  // Define "__emutls_t.*" if there is InitValue
  GlobalVariable *EmuTlsTmplVar = nullptr;
  if (InitValue) {
    std::string EmuTlsTmplName = ("__emutls_t." + GV->getName()).str();
    EmuTlsTmplVar = M.getOrInsertGlobal(EmuTlsTmplName, GVType);
    assert(EmuTlsTmplVar && "Failed to create emualted TLS initializer");
    EmuTlsTmplVar->setConstant(true);
    EmuTlsTmplVar->setInitializer(const_cast<Constant*>(InitValue));
    EmuTlsTmplVar->setAlignment(GVAlignment);
    copyLinkageVisibility(M, GV, EmuTlsTmplVar);
  }

  // Define "__emutls_v.*" with initializer and alignment.
  Constant *ElementValues[4] = {
      ConstantInt::get(WordType, DL.getTypeStoreSize(GVType)),
      ConstantInt::get(WordType, GVAlignment.value()), NullPtr,
      EmuTlsTmplVar ? EmuTlsTmplVar : NullPtr};
  EmuTlsVar->setInitializer(ConstantStruct::get(EmuTlsVarType, ElementValues));
  Align MaxAlignment =
      std::max(DL.getABITypeAlign(WordType), DL.getABITypeAlign(VoidPtrType));
  EmuTlsVar->setAlignment(MaxAlignment);
  return true;
}
