//===--- IRSymbolVisitor.h - Symbol Visitor for IR --------------*- C++ -*-===//
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

#ifndef LANGUAGE_IRGEN_IRSYMBOLVISITOR_H
#define LANGUAGE_IRGEN_IRSYMBOLVISITOR_H

#include "language/IRGen/Linking.h"

namespace language {

class SILSymbolVisitorContext;

namespace irgen {

/// Context for symbol enumeration using `IRSymbolVisitor`.
class IRSymbolVisitorContext {
  const UniversalLinkageInfo &LinkInfo;
  const SILSymbolVisitorContext &SILCtx;

public:
  IRSymbolVisitorContext(const UniversalLinkageInfo &LinkInfo,
                         const SILSymbolVisitorContext &SILCtx)
      : LinkInfo{LinkInfo}, SILCtx{SILCtx} {}

  const UniversalLinkageInfo &getLinkInfo() const { return LinkInfo; }
  const SILSymbolVisitorContext &getSILCtx() const { return SILCtx; }
};

/// A visitor class which may be used to enumerate the entities representing
/// linker symbols associated with a Codira declaration, file, or module. This
/// class is a refinement of `SILSymbolVisitor` for the IRGen layer. Most of the
/// enumerated linker symbols can be represented as either a `SILDeclRef` or a
/// `LinkEntity`, but a few aren't supported by those abstractions and are
/// handled ad-hoc. Additionally, there are some Obj-C entities (like methods)
/// that don't actually have associated linker symbols but are enumerated for
/// the convenience of clients using this utility for API surface discovery.
class IRSymbolVisitor {
public:
  virtual ~IRSymbolVisitor() {}

  /// Enumerate the symbols associated with the given decl.
  void visit(Decl *D, const IRSymbolVisitorContext &ctx);

  /// Enumerate the symbols associated with the given file.
  void visitFile(FileUnit *file, const IRSymbolVisitorContext &ctx);

  /// Enumerate the symbols associated with the given modules.
  void visitModules(toolchain::SmallVector<ModuleDecl *, 4> &modules,
                    const IRSymbolVisitorContext &ctx);

  /// Override to prepare for enumeration of the symbols for a specific decl.
  /// Return \c true to proceed with visiting the decl or \c false to skip it.
  virtual bool willVisitDecl(Decl *D) { return true; }

  /// Override to clean up after enumeration of the symbols for a specific decl.
  virtual void didVisitDecl(Decl *D) {}

  virtual void addFunction(SILDeclRef declRef) {}
  virtual void addFunction(StringRef name, SILDeclRef declRef) {}
  virtual void addGlobalVar(VarDecl *VD) {}
  virtual void addLinkEntity(LinkEntity entity) {}
  virtual void addObjCInterface(ClassDecl *CD) {}
  virtual void addObjCMethod(AbstractFunctionDecl *AFD) {}
  virtual void addProtocolWitnessThunk(RootProtocolConformance *C,
                                       ValueDecl *requirementDecl) {}
};

} // end namespace irgen
} // end namespace language

#endif
