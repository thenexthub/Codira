//===--- SILSymbolVisitor.h - Symbol Visitor for SIL ------------*- C++ -*-===//
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

#ifndef LANGUAGE_SIL_SILSYMBOLVISITOR_H
#define LANGUAGE_SIL_SILSYMBOLVISITOR_H

#include "language/AST/Decl.h"
#include "language/AST/ProtocolAssociations.h"
#include "language/AST/ProtocolConformance.h"
#include "language/SIL/SILDeclRef.h"
#include "language/SIL/SILModule.h"

namespace language {

/// Options dictating which symbols are enumerated by `SILSymbolVisitor`.
struct SILSymbolVisitorOptions {
  /// Whether to visit the members of declarations like structs, classes, and
  /// protocols.
  bool VisitMembers = true;

  /// Whether to only visit declarations for which linker directive symbols
  /// are needed (e.g. decls with `@_originallyDefinedIn`.
  bool LinkerDirectivesOnly = false;

  /// Whether to only visit symbols with public or package linkage.
  bool PublicOrPackageSymbolsOnly = true;

  /// Whether LLVM IR Virtual Function Elimination is enabled.
  bool VirtualFunctionElimination = false;

  /// Whether LLVM IR Witness Method Elimination is enabled.
  bool WitnessMethodElimination = false;

  /// Whether resilient protocols should be emitted fragile.
  bool FragileResilientProtocols = false;
};

/// Context for `SILSymbolVisitor` symbol enumeration.
class SILSymbolVisitorContext {
  ModuleDecl *Module;
  const SILSymbolVisitorOptions Opts;

public:
  SILSymbolVisitorContext(ModuleDecl *M, const SILSymbolVisitorOptions Opts)
      : Module{M}, Opts{Opts} {
    assert(M);
  };

  ModuleDecl *getModule() const { return Module; }
  const SILSymbolVisitorOptions &getOpts() const { return Opts; }
};

/// A visitor class which may be used to enumerate the entities representing
/// linker symbols associated with a Codira declaration, file, or module. Some
/// enumerated linker symbols can be represented as a `SILDeclRef`. The rest are
/// enumerated ad-hoc. Additionally, there a some Obj-C entities (like methods)
/// that don't actually have associated linker symbols but are enumerated for
/// the convenience of clients using this utility for API surface discovery.
class SILSymbolVisitor {
public:
  virtual ~SILSymbolVisitor() {}

  /// Enumerate the symbols associated with the given decl.
  void visitDecl(Decl *D, const SILSymbolVisitorContext &ctx);

  /// Enumerate the symbols associated with the given file.
  void visitFile(FileUnit *file, const SILSymbolVisitorContext &ctx);

  /// Enumerate the symbols associated with the given modules.
  void visitModules(toolchain::SmallVector<ModuleDecl *, 4> &modules,
                    const SILSymbolVisitorContext &ctx);

  /// Override to prepare for enumeration of the symbols for a specific decl.
  /// Return \c true to proceed with visiting the decl or \c false to skip it.
  virtual bool willVisitDecl(Decl *D) { return true; }

  /// Override to clean up after enumeration of the symbols for a specific decl.
  virtual void didVisitDecl(Decl *D) {}

  /// A classification for the dynamic dispatch metadata of a decl.
  enum class DynamicKind {
    /// May be replaced at runtime (e.g.`dynamic`).
    Replaceable,

    /// May replace another decl at runtime (e.g. `@_dynamicReplacement(for:)`).
    Replacement,
  };

  virtual void addAssociatedConformanceDescriptor(AssociatedConformance AC) {}
  virtual void addAssociatedTypeDescriptor(AssociatedTypeDecl *ATD) {}
  virtual void addAsyncFunctionPointer(SILDeclRef declRef) {}
  virtual void addBaseConformanceDescriptor(BaseConformance BC) {}
  virtual void addClassMetadataBaseOffset(ClassDecl *CD) {}
  virtual void addCoroFunctionPointer(SILDeclRef declRef) {}
  virtual void addDispatchThunk(SILDeclRef declRef) {}
  virtual void addDynamicFunction(AbstractFunctionDecl *AFD,
                                  DynamicKind dynKind) {}
  virtual void addEnumCase(EnumElementDecl *EED) {}
  virtual void addFieldOffset(VarDecl *VD) {}
  virtual void addFunction(SILDeclRef declRef) {}
  virtual void addFunction(StringRef name, SILDeclRef declRef) {}
  virtual void addGlobalVar(VarDecl *VD) {}
  virtual void addMethodDescriptor(SILDeclRef declRef) {}
  virtual void addMethodLookupFunction(ClassDecl *CD) {}
  virtual void addNominalTypeDescriptor(NominalTypeDecl *NTD) {}
  virtual void addObjCInterface(ClassDecl *CD) {}
  virtual void addObjCMetaclass(ClassDecl *CD) {}
  virtual void addObjCMethod(AbstractFunctionDecl *AFD) {}
  virtual void addObjCResilientClassStub(ClassDecl *CD) {}
  virtual void addOpaqueTypeDescriptor(OpaqueTypeDecl *OTD) {}
  virtual void addOpaqueTypeDescriptorAccessor(OpaqueTypeDecl *OTD,
                                               DynamicKind dynKind) {}
  virtual void addPropertyDescriptor(AbstractStorageDecl *ASD) {}
  virtual void addProtocolConformanceDescriptor(RootProtocolConformance *C) {}
  virtual void addProtocolDescriptor(ProtocolDecl *PD) {}
  virtual void addProtocolRequirementsBaseDescriptor(ProtocolDecl *PD) {}
  virtual void addProtocolWitnessTable(RootProtocolConformance *C) {}
  virtual void addProtocolWitnessThunk(RootProtocolConformance *C,
                                       ValueDecl *requirementDecl) {}
  virtual void addCodiraMetaclassStub(ClassDecl *CD) {}
  virtual void addTypeMetadataAccessFunction(CanType T) {}
  virtual void addTypeMetadataAddress(CanType T) {}
};

template <typename F>
void enumerateFunctionsForHasSymbol(SILModule &M, ValueDecl *D, F Handler) {
  // Handle clang decls separately.
  if (auto *clangDecl = D->getClangDecl()) {
    if (isa<clang::FunctionDecl>(clangDecl))
      Handler(SILDeclRef(D).asForeign());

    return;
  }

  class SymbolVisitor : public SILSymbolVisitor {
    F Handler;

  public:
    SymbolVisitor(F Handler) : Handler{Handler} {};

    void addFunction(SILDeclRef declRef) override { Handler(declRef); }

    virtual void addFunction(StringRef name, SILDeclRef declRef) override {
      // The kinds of functions which go through this callback (e.g.
      // differentiability witnesses) have custom manglings and are incompatible
      // with #_hasSymbol currently.
      //
      // Ideally, this callback will be removed entirely in favor of SILDeclRef
      // being able to represent all function variants with no special cases
      // required.
    }
  };

  SILSymbolVisitorOptions opts;
  opts.VisitMembers = false;
  auto visitorCtx = SILSymbolVisitorContext(M.getCodiraModule(), opts);
  SymbolVisitor(Handler).visitDecl(D, visitorCtx);
}

} // end namespace language

#endif
