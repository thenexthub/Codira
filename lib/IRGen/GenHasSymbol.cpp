//===--- GenHasSymbol.cpp - IR Generation for #_hasSymbol queries ---------===//
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
//
//  This file implements IR generation for `if #_hasSymbol` condition queries.
//
//===----------------------------------------------------------------------===//

#include "language/AST/ASTMangler.h"
#include "language/AST/PrettyStackTrace.h"
#include "language/Basic/Assertions.h"
#include "language/IRGen/IRSymbolVisitor.h"
#include "language/IRGen/Linking.h"
#include "language/SIL/SILFunctionBuilder.h"
#include "language/SIL/SILModule.h"
#include "language/SIL/SILSymbolVisitor.h"
#include "clang/AST/DeclObjC.h"
#include "clang/AST/GlobalDecl.h"

#include "GenDecl.h"
#include "GenericArguments.h"
#include "IRGenFunction.h"
#include "IRGenModule.h"

using namespace language;
using namespace irgen;

/// Wrapper for IRGenModule::getAddrOfLLVMVariable() that also handles a few
/// additional types of entities that the main utility cannot.
static toolchain::Constant *getAddrOfLLVMVariable(IRGenModule &IGM,
                                             LinkEntity entity) {
  if (entity.isTypeMetadataAccessFunction()) {
    auto type = entity.getType();
    auto nominal = type->getAnyNominal();
    assert(nominal);

    if (nominal->isGenericContext()) {
      GenericArguments genericArgs;
      genericArgs.collectTypes(IGM, nominal);

      return IGM.getAddrOfGenericTypeMetadataAccessFunction(nominal,
                                                            genericArgs.Types,
                                                            NotForDefinition);
    } else {
      return IGM.getAddrOfTypeMetadataAccessFunction(type,
                                                     NotForDefinition);
    }
  }
  if (entity.isDispatchThunk())
    return IGM.getAddrOfDispatchThunk(entity.getSILDeclRef(), NotForDefinition);

  if (entity.isOpaqueTypeDescriptorAccessor()) {
    OpaqueTypeDecl *decl =
        const_cast<OpaqueTypeDecl *>(cast<OpaqueTypeDecl>(entity.getDecl()));
    bool isImplementation = entity.isOpaqueTypeDescriptorAccessorImpl();
    return IGM
        .getAddrOfOpaqueTypeDescriptorAccessFunction(decl, NotForDefinition,
                                                     isImplementation)
        .getDirectPointer();
  }

  // FIXME: Look up addr of the replaceable function (has "TI" mangling suffix)
  if (entity.isDynamicallyReplaceableFunctionImpl())
    return nullptr;

  return IGM.getAddrOfLLVMVariable(entity, ConstantInit(), DebugTypeInfo());
}

class HasSymbolIRGenVisitor : public IRSymbolVisitor {
  IRGenModule &IGM;
  toolchain::SmallVector<toolchain::Constant *, 4> &Addrs;

  void addFunction(SILFunction *fn) {
    Addrs.emplace_back(IGM.getAddrOfSILFunction(fn, NotForDefinition));
  }

  void addFunction(StringRef name) {
    SILFunction *silFn = IGM.getSILModule().lookUpFunction(name);
    // Definitions for each SIL function should have been emitted by SILGen.
    assert(silFn && "missing SIL function?");
    if (silFn)
      addFunction(silFn);
  }

public:
  HasSymbolIRGenVisitor(IRGenModule &IGM,
                        toolchain::SmallVector<toolchain::Constant *, 4> &Addrs)
      : IGM{IGM}, Addrs{Addrs} {};

  void addFunction(SILDeclRef declRef) override {
    addFunction(declRef.mangle());
  }

  void addFunction(StringRef name, SILDeclRef declRef) override {
    addFunction(name);
  }

  void addGlobalVar(VarDecl *VD) override {
    // FIXME: Handle global vars
    toolchain::report_fatal_error("unhandled global var");
  }

  void addLinkEntity(LinkEntity entity) override {
    // Skip property descriptors for static properties, which were only
    // introduced with SE-0438 and are therefore not present in all libraries.
    if (entity.isPropertyDescriptor()) {
      if (entity.getAbstractStorageDecl()->isStatic())
        return;
    }

    if (entity.hasSILFunction()) {
      addFunction(entity.getSILFunction());
      return;
    }

    auto constant = getAddrOfLLVMVariable(IGM, entity);
    if (constant) {
      auto global = cast<toolchain::GlobalValue>(constant);
      Addrs.emplace_back(global);
    }
  }

  void addProtocolWitnessThunk(RootProtocolConformance *C,
                               ValueDecl *requirementDecl) override {
    // FIXME: Handle protocol witness thunks
    toolchain::report_fatal_error("unhandled protocol witness thunk");
  }
};

static toolchain::Constant *
getAddrOfLLVMVariableForClangDecl(IRGenModule &IGM, ValueDecl *decl,
                                  const clang::Decl *clangDecl) {
  if (isa<clang::FunctionDecl>(clangDecl)) {
    SILFunction *silFn =
        IGM.getSILModule().lookUpFunction(SILDeclRef(decl).asForeign());
    assert(silFn && "missing SIL function?");
    return silFn ? IGM.getAddrOfSILFunction(silFn, NotForDefinition) : nullptr;
  }

  if (isa<clang::ObjCInterfaceDecl>(clangDecl))
    return IGM.getAddrOfObjCClass(cast<ClassDecl>(decl), NotForDefinition);

  toolchain::report_fatal_error("Unexpected clang decl kind");
}

static void
getSymbolAddrsForDecl(IRGenModule &IGM, ValueDecl *decl,
                      toolchain::SmallVector<toolchain::Constant *, 4> &addrs) {
  if (auto *clangDecl = decl->getClangDecl()) {
    if (auto *addr = getAddrOfLLVMVariableForClangDecl(IGM, decl, clangDecl))
      addrs.push_back(addr);
    return;
  }

  SILSymbolVisitorOptions opts;
  opts.VisitMembers = false;
  auto silCtx = SILSymbolVisitorContext(IGM.getCodiraModule(), opts);
  auto linkInfo = UniversalLinkageInfo(IGM);
  auto symbolVisitorCtx = IRSymbolVisitorContext(linkInfo, silCtx);
  HasSymbolIRGenVisitor(IGM, addrs).visit(decl, symbolVisitorCtx);
}

toolchain::Function *IRGenModule::emitHasSymbolFunction(ValueDecl *decl) {

  PrettyStackTraceDecl trace("emitting #_hasSymbol query for", decl);
  Mangle::ASTMangler mangler(Context);

  auto fn = cast<toolchain::Function>(getOrCreateHelperFunction(
      mangler.mangleHasSymbolQuery(decl), Int1Ty, {},
      [decl, this](IRGenFunction &IGF) {
        auto &Builder = IGF.Builder;
        toolchain::SmallVector<toolchain::Constant *, 4> addrs;
        getSymbolAddrsForDecl(*this, decl, addrs);

        toolchain::Value *ret = nullptr;
        for (toolchain::Constant *addr : addrs) {
          assert(cast<toolchain::GlobalValue>(addr)->hasExternalWeakLinkage());

          auto isNonNull = IGF.Builder.CreateIsNotNull(addr);
          ret = (ret) ? IGF.Builder.CreateAnd(ret, isNonNull) : isNonNull;
        }

        if (ret) {
          Builder.CreateRet(ret);
        } else {
          // There were no addresses produced by the visitor, return true.
          Builder.CreateRet(toolchain::ConstantInt::get(Int1Ty, 1));
        }
      },
      /*IsNoInline*/ false));

  fn->setDoesNotThrow();
  fn->setCallingConv(DefaultCC);
  fn->setOnlyReadsMemory();

  return fn;
}
