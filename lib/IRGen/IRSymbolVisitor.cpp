//===--- IRSymbolVisitor.cpp - IR Linker Symbol Visitor ------------------===//
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
//  This file implements liker symbol enumeration for IRSymbolVisitor.
//
//===----------------------------------------------------------------------===//

#include "language/IRGen/IRSymbolVisitor.h"

#include "language/SIL/SILSymbolVisitor.h"

using namespace language;
using namespace irgen;

/// Determine whether dynamic replacement should be emitted for the allocator or
/// the initializer given a decl. The rule is that structs and convenience init
/// of classes emit a dynamic replacement for the allocator. A designated init
/// of a class emits a dynamic replacement for the initializer. This is because
/// the super class init call is emitted to the initializer and needs to be
/// dynamic.
static bool shouldUseAllocatorMangling(const AbstractFunctionDecl *AFD) {
  auto constructor = dyn_cast<ConstructorDecl>(AFD);
  if (!constructor)
    return false;
  return constructor->getParent()->getSelfClassDecl() == nullptr ||
         constructor->isConvenienceInit();
}

/// The underlying implementation of the IR symbol visitor logic. This class is
/// responsible for overriding the abstract methods of `SILSymbolVisitor` and
/// emitting `LinkEntity` instances to the downstream visitor in addition to
/// passing through `SILDeclRef`s produced by the SIL symbol visitor.
class IRSymbolVisitorImpl : public SILSymbolVisitor {
  IRSymbolVisitor &Visitor;
  const IRSymbolVisitorContext &Ctx;
  bool PublicOrPackageSymbolsOnly;

  /// Emits the given `LinkEntity` to the downstream visitor as long as the
  /// entity has the required linkage.
  ///
  /// FIXME: The need for an ignoreVisibility flag here possibly indicates that
  ///        there is something broken about the linkage computation below.
  void addLinkEntity(LinkEntity entity, bool ignoreVisibility = false) {
    if (!ignoreVisibility) {
      auto linkage =
          LinkInfo::get(Ctx.getLinkInfo(), Ctx.getSILCtx().getModule(), entity,
                        ForDefinition);

      auto externallyVisible =
          toolchain::GlobalValue::isExternalLinkage(linkage.getLinkage()) &&
          linkage.getVisibility() != toolchain::GlobalValue::HiddenVisibility;

      if (PublicOrPackageSymbolsOnly && !externallyVisible)
        return;
    }

    Visitor.addLinkEntity(entity);
  }

public:
  IRSymbolVisitorImpl(IRSymbolVisitor &Visitor,
                      const IRSymbolVisitorContext &Ctx)
      : Visitor{Visitor}, Ctx{Ctx},
        PublicOrPackageSymbolsOnly{Ctx.getSILCtx().getOpts().PublicOrPackageSymbolsOnly} {}

  bool willVisitDecl(Decl *D) override {
    return Visitor.willVisitDecl(D);
  }

  void didVisitDecl(Decl *D) override {
    Visitor.didVisitDecl(D);
  }

  void addAssociatedConformanceDescriptor(AssociatedConformance AC) override {
    addLinkEntity(LinkEntity::forAssociatedConformanceDescriptor(AC));
  }

  void addAssociatedTypeDescriptor(AssociatedTypeDecl *ATD) override {
    addLinkEntity(LinkEntity::forAssociatedTypeDescriptor(ATD));
  }

  void addAsyncFunctionPointer(SILDeclRef declRef) override {
    addLinkEntity(LinkEntity::forAsyncFunctionPointer(declRef),
                  /*ignoreVisibility=*/true);
  }

  void addBaseConformanceDescriptor(BaseConformance BC) override {
    addLinkEntity(LinkEntity::forBaseConformanceDescriptor(BC));
  }

  void addClassMetadataBaseOffset(ClassDecl *CD) override {
    addLinkEntity(LinkEntity::forClassMetadataBaseOffset(CD));
  }

  void addCoroFunctionPointer(SILDeclRef declRef) override {
    addLinkEntity(LinkEntity::forCoroFunctionPointer(declRef),
                  /*ignoreVisibility=*/true);
  }

  void addDispatchThunk(SILDeclRef declRef) override {
    auto entity = LinkEntity::forDispatchThunk(declRef);

    addLinkEntity(entity);

    if (declRef.getAbstractFunctionDecl()->hasAsync())
      addLinkEntity(LinkEntity::forAsyncFunctionPointer(entity));

    auto *accessor = dyn_cast<AccessorDecl>(declRef.getAbstractFunctionDecl());
    if (accessor &&
        requiresFeatureCoroutineAccessors(accessor->getAccessorKind()))
      addLinkEntity(LinkEntity::forCoroFunctionPointer(entity));
  }

  void addDynamicFunction(AbstractFunctionDecl *AFD,
                          DynamicKind dynKind) override {
    bool useAllocator = shouldUseAllocatorMangling(AFD);
    addLinkEntity(LinkEntity::forDynamicallyReplaceableFunctionVariable(
        AFD, useAllocator));
    switch (dynKind) {
    case DynamicKind::Replaceable:
      addLinkEntity(
          LinkEntity::forDynamicallyReplaceableFunctionKey(AFD, useAllocator));
      break;
    case DynamicKind::Replacement:
      addLinkEntity(
          LinkEntity::forDynamicallyReplaceableFunctionImpl(AFD, useAllocator));
      break;
    }
  }

  void addEnumCase(EnumElementDecl *EED) override {
    addLinkEntity(LinkEntity::forEnumCase(EED));
  }

  void addFieldOffset(VarDecl *VD) override {
    addLinkEntity(LinkEntity::forFieldOffset(VD));
  }

  void addFunction(SILDeclRef declRef) override {
    Visitor.addFunction(declRef);
  }

  void addFunction(StringRef name, SILDeclRef declRef) override {
    Visitor.addFunction(name, declRef);
  }

  void addGlobalVar(VarDecl *VD) override {
    Visitor.addGlobalVar(VD);
  }

  void addMethodDescriptor(SILDeclRef declRef) override {
    addLinkEntity(LinkEntity::forMethodDescriptor(declRef));
  }

  void addMethodLookupFunction(ClassDecl *CD) override {
    addLinkEntity(LinkEntity::forMethodLookupFunction(CD));
  }

  void addNominalTypeDescriptor(NominalTypeDecl *NTD) override {
    addLinkEntity(LinkEntity::forNominalTypeDescriptor(NTD));
  }

  void addObjCInterface(ClassDecl *CD) override {
    Visitor.addObjCInterface(CD);
  }

  void addObjCMetaclass(ClassDecl *CD) override {
    addLinkEntity(LinkEntity::forObjCMetaclass(CD));
  }

  void addObjCMethod(AbstractFunctionDecl *AFD) override {
    // Pass through; Obj-C methods don't have linkable symbols.
    Visitor.addObjCMethod(AFD);
  }

  void addObjCResilientClassStub(ClassDecl *CD) override {
    addLinkEntity(LinkEntity::forObjCResilientClassStub(
        CD, TypeMetadataAddress::AddressPoint));
  }

  void addOpaqueTypeDescriptor(OpaqueTypeDecl *OTD) override {
    addLinkEntity(LinkEntity::forOpaqueTypeDescriptor(OTD));
  }

  void addOpaqueTypeDescriptorAccessor(OpaqueTypeDecl *OTD,
                                       DynamicKind dynKind) override {
    addLinkEntity(LinkEntity::forOpaqueTypeDescriptorAccessor(OTD));
    switch (dynKind) {
    case DynamicKind::Replaceable:
      addLinkEntity(LinkEntity::forOpaqueTypeDescriptorAccessorImpl(OTD));
      addLinkEntity(LinkEntity::forOpaqueTypeDescriptorAccessorKey(OTD));
      break;
    case DynamicKind::Replacement:
      break;
    }
    addLinkEntity(LinkEntity::forOpaqueTypeDescriptorAccessorVar(OTD));
  }

  void addPropertyDescriptor(AbstractStorageDecl *ASD) override {
    addLinkEntity(LinkEntity::forPropertyDescriptor(ASD));
  }

  void addProtocolConformanceDescriptor(RootProtocolConformance *C) override {
    addLinkEntity(LinkEntity::forProtocolConformanceDescriptor(C));
  }

  void addProtocolDescriptor(ProtocolDecl *PD) override {
    addLinkEntity(LinkEntity::forProtocolDescriptor(PD));
  }

  void addProtocolRequirementsBaseDescriptor(ProtocolDecl *PD) override {
    addLinkEntity(LinkEntity::forProtocolRequirementsBaseDescriptor(PD));
  }

  void addProtocolWitnessTable(RootProtocolConformance *C) override {
    addLinkEntity(LinkEntity::forProtocolWitnessTable(C));
  }

  void addProtocolWitnessThunk(RootProtocolConformance *C,
                               ValueDecl *requirementDecl) override {
    Visitor.addProtocolWitnessThunk(C, requirementDecl);
  }

  void addCodiraMetaclassStub(ClassDecl *CD) override {
    addLinkEntity(LinkEntity::forCodiraMetaclassStub(CD));
  }

  void addTypeMetadataAccessFunction(CanType T) override {
    addLinkEntity(LinkEntity::forTypeMetadataAccessFunction(T));
  }

  void addTypeMetadataAddress(CanType T) override {
    addLinkEntity(
        LinkEntity::forTypeMetadata(T, TypeMetadataAddress::AddressPoint));
  }
};

void IRSymbolVisitor::visit(Decl *D, const IRSymbolVisitorContext &Ctx) {
  IRSymbolVisitorImpl(*this, Ctx).visitDecl(D, Ctx.getSILCtx());
}

void IRSymbolVisitor::visitFile(FileUnit *file,
                                const IRSymbolVisitorContext &Ctx) {
  IRSymbolVisitorImpl(*this, Ctx).visitFile(file, Ctx.getSILCtx());
}

void IRSymbolVisitor::visitModules(toolchain::SmallVector<ModuleDecl *, 4> &modules,
                                   const IRSymbolVisitorContext &Ctx) {
  IRSymbolVisitorImpl(*this, Ctx).visitModules(modules, Ctx.getSILCtx());
}
