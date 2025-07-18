//===--- ClangModule.h - An imported Clang module ---------------*- C++ -*-===//
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
// This file implements support for loading Clang modules into Codira.
//
//===----------------------------------------------------------------------===//
#ifndef LANGUAGE_CLANGIMPORTER_CLANGMODULE_H
#define LANGUAGE_CLANGIMPORTER_CLANGMODULE_H

#include "language/AST/FileUnit.h"
#include "language/Basic/Version.h"
#include "language/ClangImporter/ClangImporter.h"
#include "clang/AST/ExternalASTSource.h"
#include "clang/Basic/ASTSourceDescriptor.h"
#include "clang/Basic/Module.h"

namespace clang {
  class ASTContext;
  class Module;
}

namespace language {

class ASTContext;
class ModuleLoader;

/// Represents a Clang module that has been imported into Codira.
class ClangModuleUnit final : public LoadedFile {
  ClangImporter::Implementation &owner;
  const clang::Module *clangModule;
  toolchain::PointerIntPair<ModuleDecl *, 1, bool> overlayModule;
  mutable std::optional<ArrayRef<ImportedModule>> importedModulesForLookup;
  /// The metadata of the underlying Clang module.
  clang::ASTSourceDescriptor ASTSourceDescriptor;

public:
  /// True if the given Module contains an imported Clang module unit.
  static bool hasClangModule(ModuleDecl *M);

  ClangModuleUnit(ModuleDecl &M, ClangImporter::Implementation &owner,
                  const clang::Module *clangModule);

  /// Retrieve the underlying Clang module.
  ///
  /// This will be null if the module unit represents the imported headers.
  const clang::Module *getClangModule() const { return clangModule; }

  /// Returns true if this is a top-level Clang module (not a submodule).
  bool isTopLevel() const;

  /// Returns the Codira module that overlays this Clang module.
  ModuleDecl *getOverlayModule() const override;

  /// Retrieve the "exported" name of the module, which is usually the module
  /// real name, but might be the name of the public module through which this
  /// (private) module is re-exported.
  StringRef getExportedModuleName() const override;

  virtual bool isSystemModule() const override;

  virtual void lookupValue(DeclName name, NLKind lookupKind,
                           OptionSet<ModuleLookupFlags> Flags,
                           SmallVectorImpl<ValueDecl*> &results) const override;

  virtual TypeDecl *
  lookupNestedType(Identifier name,
                   const NominalTypeDecl *baseType) const override;

  virtual void lookupVisibleDecls(ImportPath::Access accessPath,
                                  VisibleDeclConsumer &consumer,
                                  NLKind lookupKind) const override;

  virtual void lookupClassMembers(ImportPath::Access accessPath,
                                  VisibleDeclConsumer &consumer) const override;

  virtual void
  lookupClassMember(ImportPath::Access accessPath, DeclName name,
                    SmallVectorImpl<ValueDecl*> &decls) const override;

  void lookupObjCMethods(
         ObjCSelector selector,
         SmallVectorImpl<AbstractFunctionDecl *> &results) const override;

  void lookupAvailabilityDomains(
      Identifier identifier,
      SmallVectorImpl<AvailabilityDomain> &results) const override;

  virtual void getTopLevelDecls(SmallVectorImpl<Decl*> &results) const override;

  virtual void getDisplayDecls(SmallVectorImpl<Decl*> &results, bool recursive = false) const override;

  virtual void
  getImportedModules(SmallVectorImpl<ImportedModule> &imports,
                     ModuleDecl::ImportFilter filter) const override;

  virtual void getImportedModulesForLookup(
      SmallVectorImpl<ImportedModule> &imports) const override;

  virtual void
  collectLinkLibraries(ModuleDecl::LinkLibraryCallback callback) const override;

  Identifier
  getDiscriminatorForPrivateDecl(const Decl *D) const override {
    toolchain_unreachable("Clang modules do not need discriminators");
  }

  virtual version::Version getLanguageVersionBuiltWith() const override {
    return version::Version();
  }

  virtual StringRef getFilename() const override;

  virtual StringRef getLoadedFilename() const override;

  virtual const clang::Module *getUnderlyingClangModule() const override {
    return getClangModule();
  }

  clang::ASTContext &getClangASTContext() const;

  /// Returns the ASTSourceDescriptor of the associated Clang module if one
  /// exists.
  std::optional<clang::ASTSourceDescriptor> getASTSourceDescriptor() const;

  virtual StringRef getModuleDefiningPath() const override;

  static bool classof(const FileUnit *file) {
    return file->getKind() == FileUnitKind::ClangModule;
  }
  static bool classof(const DeclContext *DC) {
    return isa<FileUnit>(DC) && classof(cast<FileUnit>(DC));
  }
};

}

#endif
