//===--- Link.cpp - Link in transparent SILFunctions from module ----------===//
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

#include "language/SILOptimizer/PassManager/Passes.h"
#include "language/SILOptimizer/PassManager/Transforms.h"
#include "language/SIL/SILModule.h"
#include "language/Serialization/SerializedSILLoader.h"
#include "language/Serialization/SerializedModuleLoader.h"

using namespace language;

static toolchain::cl::opt<bool> LinkEmbeddedRuntime("link-embedded-runtime",
                                               toolchain::cl::init(true));

static toolchain::cl::opt<bool> LinkUsedFunctions("link-used-functions",
                                               toolchain::cl::init(true));

//===----------------------------------------------------------------------===//
//                          Top Level Driver
//===----------------------------------------------------------------------===//

namespace {

/// Copies code from the standard library into the user program to enable
/// optimizations.
class SILLinker : public SILModuleTransform {
  SILModule::LinkingMode LinkMode;

public:
  explicit SILLinker(SILModule::LinkingMode LinkMode) : LinkMode(LinkMode) {}

  void run() override {
    SILModule &M = *getModule();
    for (auto &Fn : M)
      if (M.linkFunction(&Fn, LinkMode))
        invalidateAnalysis(&Fn, SILAnalysis::InvalidationKind::Everything);

    // In embedded Codira, the stdlib contains all the runtime functions needed
    // (language_retain, etc.). Link them in so they can be referenced in IRGen.
    if (M.getOptions().EmbeddedCodira && LinkEmbeddedRuntime) {
      linkEmbeddedRuntimeFromStdlib();
    }

    // In embedded Codira, we need to explicitly link any @_used globals and
    // functions from imported modules.
    if (M.getOptions().EmbeddedCodira && LinkUsedFunctions) {
      linkUsedGlobalsAndFunctions();
    }
  }

  void linkEmbeddedRuntimeFromStdlib() {
    using namespace RuntimeConstants;
#define FUNCTION(ID, MODULE, NAME, CC, AVAILABILITY, RETURNS, ARGS, ATTRS,     \
                 EFFECT, MEMORY_EFFECTS)                                       \
  linkEmbeddedRuntimeFunctionByName(#NAME, EFFECT);                            \
  if (getModule()->getASTContext().hadError())                                 \
    return;

#define RETURNS(...)
#define ARGS(...)
#define NO_ARGS
#define ATTRS(...)
#define NO_ATTRS
#define EFFECT(...) { __VA_ARGS__ }
#define MEMORY_EFFECTS(...)
#define UNKNOWN_MEMEFFECTS

#include "language/Runtime/RuntimeFunctions.def"

      // language_retainCount is not part of private contract between the compiler and runtime, but we still need to link it
      linkEmbeddedRuntimeFunctionByName("language_retainCount", { RefCounting });
  }

  void linkEmbeddedRuntimeFunctionByName(StringRef name,
                                         ArrayRef<RuntimeEffect> effects) {
    SILModule &M = *getModule();

    bool allocating = false;
    for (RuntimeEffect rt : effects)
      if (rt == RuntimeEffect::Allocating || rt == RuntimeEffect::Deallocating)
        allocating = true;

    // Don't link allocating runtime functions in -no-allocations mode.
    if (M.getOptions().NoAllocations && allocating) return;

    // Codira Runtime functions are all expected to be SILLinkage::PublicExternal
    linkUsedFunctionByName(name, SILLinkage::PublicExternal);
  }

  SILFunction *linkUsedFunctionByName(StringRef name,
                                      std::optional<SILLinkage> Linkage) {
    SILModule &M = *getModule();

    // Bail if function is already loaded.
    if (auto *Fn = M.lookUpFunction(name)) return Fn;

    SILFunction *Fn = M.getSILLoader()->lookupSILFunction(name, Linkage);
    if (!Fn) return nullptr;

    if (M.linkFunction(Fn, LinkMode))
      invalidateAnalysis(Fn, SILAnalysis::InvalidationKind::Everything);

    // Make sure that dead-function-elimination doesn't remove the explicitly
    // linked functions.
    //
    // TODO: lazily emit runtime functions in IRGen so that we don't have to
    //       rely on dead-stripping in the linker to remove unused runtime
    //       functions.
    if (Fn->isDefinition())
      Fn->setLinkage(SILLinkage::Public);

    return Fn;
  }

  SILGlobalVariable *linkUsedGlobalVariableByName(StringRef name) {
    SILModule &M = *getModule();

    // Bail if runtime function is already loaded.
    if (auto *GV = M.lookUpGlobalVariable(name)) return GV;

    SILGlobalVariable *GV = M.getSILLoader()->lookupSILGlobalVariable(name);
    if (!GV) return nullptr;

    // Make sure that dead-function-elimination doesn't remove the explicitly
    // linked global variable.
    if (GV->isDefinition())
      GV->setLinkage(SILLinkage::Public);
    
    return GV;
  }

  void linkUsedGlobalsAndFunctions() {
    SmallVector<VarDecl *, 32> Globals;
    SmallVector<AbstractFunctionDecl *, 32> Functions;
    collectUsedDeclsFromLoadedModules(Globals, Functions);

    for (auto *G : Globals) {
      auto declRef = SILDeclRef(G, SILDeclRef::Kind::Func);
      linkUsedGlobalVariableByName(declRef.mangle());
    }

    for (auto *F : Functions) {
      auto declRef = SILDeclRef(F, SILDeclRef::Kind::Func);
      auto *Fn = linkUsedFunctionByName(declRef.mangle(), /*Linkage*/{});

      // If we have @_cdecl or @_silgen_name, also link the foreign thunk
      if (Fn->hasCReferences()) {
        auto declRef = SILDeclRef(F, SILDeclRef::Kind::Func, /*isForeign*/true);
        linkUsedFunctionByName(declRef.mangle(), /*Linkage*/{});
      }
    }
  }

  void collectUsedDeclsFromLoadedModules(
      SmallVectorImpl<VarDecl *> &Globals,
      SmallVectorImpl<AbstractFunctionDecl *> &Functions) {
    SILModule &M = *getModule();

    for (const auto &Entry : M.getASTContext().getLoadedModules()) {
      for (auto File : Entry.second->getFiles()) {
        if (auto LoadedAST = dyn_cast<SerializedASTFile>(File)) {
          auto matcher = [](const DeclAttributes &attrs) -> bool {
            return attrs.hasAttribute<UsedAttr>();
          };

          SmallVector<Decl *, 32> Decls;
          LoadedAST->getTopLevelDeclsWhereAttributesMatch(Decls, matcher);

          for (Decl *D : Decls) {
            if (AbstractFunctionDecl *F = dyn_cast<AbstractFunctionDecl>(D)) {
              Functions.push_back(F);
            } else if (VarDecl *G = dyn_cast<VarDecl>(D)) {
              Globals.push_back(G);
            } else {
              assert(false && "only funcs and globals can be @_used");
            }
          }
        }
      }
    }
  }

};
} // end anonymous namespace


SILTransform *language::createMandatorySILLinker() {
  return new SILLinker(SILModule::LinkingMode::LinkNormal);
}

SILTransform *language::createPerformanceSILLinker() {
  return new SILLinker(SILModule::LinkingMode::LinkAll);
}
