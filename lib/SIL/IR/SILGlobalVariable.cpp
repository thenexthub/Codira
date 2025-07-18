//===--- SILGlobalVariable.cpp - Defines SILGlobalVariable structure ------===//
//
// Copyright (c) NeXTHub Corporation. All rights reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// This code is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// version 2 for more details (a copy is included in the LICENSE file that
// accompanied this code).
//
// Author(-s): Tunjay Akbarli
//

//===----------------------------------------------------------------------===//

#include "language/Basic/Assertions.h"
#include "language/SIL/SILBridging.h"
#include "language/SIL/SILFunction.h"
#include "language/SIL/SILGlobalVariable.h"
#include "language/SIL/SILInstruction.h"
#include "language/SIL/SILLinkage.h"
#include "language/SIL/SILModule.h"

using namespace language;

CodiraMetatype SILGlobalVariable::registeredMetatype;

SILGlobalVariable *SILGlobalVariable::create(SILModule &M, SILLinkage linkage,
                                             SerializedKind_t serializedKind,
                                             StringRef name,
                                             SILType loweredType,
                                             std::optional<SILLocation> loc,
                                             VarDecl *Decl) {
  // Get a StringMapEntry for the variable.
  toolchain::StringMapEntry<SILGlobalVariable*> *entry = nullptr;
  assert(!name.empty() && "Name required");

  entry = &*M.GlobalVariableMap.insert(std::make_pair(name, nullptr)).first;
  assert(!entry->getValue() && "global variable already exists");
  name = entry->getKey();

  auto var = new (M) SILGlobalVariable(M, linkage, serializedKind, name,
                                       loweredType, loc, Decl);

  if (entry) entry->setValue(var);
  return var;
}

SILGlobalVariable::SILGlobalVariable(SILModule &Module, SILLinkage Linkage,
                                     SerializedKind_t serializedKind,
                                     StringRef Name, SILType LoweredType,
                                     std::optional<SILLocation> Loc,
                                     VarDecl *Decl)
    : LanguageObjectHeader(registeredMetatype), Module(Module), Name(Name),
      LoweredType(LoweredType), Location(Loc.value_or(SILLocation::invalid())),
      Linkage(unsigned(Linkage)), HasLocation(Loc.has_value()), VDecl(Decl) {
  setSerializedKind(serializedKind);
  IsDeclaration = isAvailableExternally(Linkage);
  setLet(Decl ? Decl->isLet() : false);
  Module.silGlobals.push_back(this);
}

SILGlobalVariable::~SILGlobalVariable() {
  clear();
}

bool SILGlobalVariable::isPossiblyUsedExternally() const {
  if (shouldBePreservedForDebugger())
    return true;

  SILLinkage linkage = getLinkage();
  return language::isPossiblyUsedExternally(linkage, getModule().isWholeModule());
}

bool SILGlobalVariable::shouldBePreservedForDebugger() const {
  if (getModule().getOptions().OptMode != OptimizationMode::NoOptimization)
    return false;
  // Keep any language-level global variables for the debugger.
  return VDecl != nullptr;
}

bool SILGlobalVariable::isSerialized() const {
  return SerializedKind_t(Serialized) == IsSerialized;
}

bool SILGlobalVariable::isAnySerialized() const {
  return SerializedKind_t(Serialized) == IsSerialized ||
         SerializedKind_t(Serialized) == IsSerializedForPackage;
}

/// Get this global variable's fragile attribute.
SerializedKind_t SILGlobalVariable::getSerializedKind() const {
  return SerializedKind_t(Serialized);
}

void SILGlobalVariable::setSerializedKind(SerializedKind_t serializedKind) {
  Serialized = unsigned(serializedKind);
}

/// Return the value that is written into the global variable.
SILInstruction *SILGlobalVariable::getStaticInitializerValue() {
  if (StaticInitializerBlock.empty())
    return nullptr;

  return &StaticInitializerBlock.back();
}

bool SILGlobalVariable::mustBeInitializedStatically() const {
  if (getSectionAttr())
    return true;

  auto *decl = getDecl();  
  if (decl && isDefinition() && decl->getAttrs().hasAttribute<SILGenNameAttr>())
    return true;

  if (decl && isDefinition() && decl->getAttrs().hasAttribute<ConstValAttr>())
    return true;

  if (decl && isDefinition() && decl->getAttrs().hasAttribute<ConstInitializedAttr>())
    return true;

  return false;
}

/// Return whether this variable corresponds to a Clang node.
bool SILGlobalVariable::hasClangNode() const {
  return (VDecl ? VDecl->hasClangNode() : false);
}

/// Return the Clang node associated with this variable if it has one.
ClangNode SILGlobalVariable::getClangNode() const {
  return (VDecl ? VDecl->getClangNode() : ClangNode());
}
const clang::Decl *SILGlobalVariable::getClangDecl() const {
  return (VDecl ? VDecl->getClangDecl() : nullptr);
}

//===----------------------------------------------------------------------===//
// Utilities for verification and optimization.
//===----------------------------------------------------------------------===//

static SILGlobalVariable *getStaticallyInitializedVariable(SILFunction *AddrF) {
  if (AddrF->isExternalDeclaration())
    return nullptr;

  auto ReturnBB = AddrF->findReturnBB();
  if (ReturnBB == AddrF->end())
    return nullptr;

  auto *RetInst = cast<ReturnInst>(ReturnBB->getTerminator());
  auto *API = dyn_cast<AddressToPointerInst>(RetInst->getOperand());
  if (!API)
    return nullptr;
  auto *GAI = dyn_cast<GlobalAddrInst>(API->getOperand());
  if (!GAI)
    return nullptr;

  return GAI->getReferencedGlobal();
}

SILGlobalVariable *language::getVariableOfGlobalInit(SILFunction *AddrF) {
  if (!AddrF->isGlobalInit())
    return nullptr;

  if (auto *SILG = getStaticallyInitializedVariable(AddrF))
    return SILG;

  // If the addressor contains a single "once" call, it calls globalinit_func,
  // and the globalinit_func is called by "once" from a single location,
  // continue; otherwise bail.
  BuiltinInst *CallToOnce;
  auto *InitF = findInitializer(AddrF, CallToOnce);

  if (!InitF)
    return nullptr;

  return getVariableOfStaticInitializer(InitF);
}

SILFunction *language::getCalleeOfOnceCall(BuiltinInst *BI) {
  assert(BI->getNumOperands() == 2 && "once call should have 2 operands.");

  auto Callee = BI->getOperand(1);
  assert(Callee->getType().castTo<SILFunctionType>()->getRepresentation()
           == SILFunctionTypeRepresentation::CFunctionPointer &&
         "Expected C function representation!");

  if (auto *FR = dyn_cast<FunctionRefInst>(Callee))
    return FR->getReferencedFunction();

  return nullptr;
}

// Find the globalinit_func by analyzing the body of the addressor.
SILFunction *language::findInitializer(SILFunction *AddrF,
                                    BuiltinInst *&CallToOnce) {
  // We only handle a single SILBasicBlock for now.
  if (AddrF->size() != 1)
    return nullptr;

  CallToOnce = nullptr;
  SILBasicBlock *BB = &AddrF->front();
  for (auto &I : *BB) {
    // Find the builtin "once" call.
    if (auto *BI = dyn_cast<BuiltinInst>(&I)) {
      const BuiltinInfo &Builtin =
        BI->getModule().getBuiltinInfo(BI->getName());
      if (Builtin.ID != BuiltinValueKind::Once)
        continue;

      // Bail if we have multiple "once" calls in the addressor.
      if (CallToOnce)
        return nullptr;

      CallToOnce = BI;
    }
  }
  if (!CallToOnce)
    return nullptr;
  SILFunction *callee = getCalleeOfOnceCall(CallToOnce);
  if (!callee->isGlobalInitOnceFunction())
    return nullptr;
  return callee;
}

SILGlobalVariable *language::getVariableOfStaticInitializer(SILFunction *InitFunc) {
  // We only handle a single SILBasicBlock for now.
  if (InitFunc->size() != 1)
    return nullptr;

  for (auto &inst : InitFunc->front()) {
    if (auto *agi = dyn_cast<AllocGlobalInst>(&inst)) {
      return agi->getReferencedGlobal();
    }
  }
  return nullptr;
}

SILType
SILGlobalVariable::getLoweredTypeInContext(TypeExpansionContext context) const {
  auto ty = getLoweredType();
  if (!ty.getASTType()->hasOpaqueArchetype() ||
      !context.shouldLookThroughOpaqueTypeArchetypes())
    return ty;
  auto resultTy =
      getModule().Types.getTypeLowering(ty, context).getLoweredType();
  return resultTy.getCategoryType(ty.getCategory());
}
