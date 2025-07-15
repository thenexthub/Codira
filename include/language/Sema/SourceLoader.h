//===--- SourceLoader.h - Import .code files as modules --------*- C++ -*-===//
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

#ifndef LANGUAGE_SEMA_SOURCELOADER_H
#define LANGUAGE_SEMA_SOURCELOADER_H

#include "language/AST/ModuleLoader.h"

namespace language {

class ASTContext;
class ModuleDecl;
  
/// Imports serialized Codira modules into an ASTContext.
class SourceLoader : public ModuleLoader {
private:
  ASTContext &Ctx;
  bool EnableLibraryEvolution;

  explicit SourceLoader(ASTContext &ctx,
                        bool enableResilience,
                        DependencyTracker *tracker)
    : ModuleLoader(tracker), Ctx(ctx),
      EnableLibraryEvolution(enableResilience) {}

public:
  static std::unique_ptr<SourceLoader>
  create(ASTContext &ctx, bool enableResilience,
         DependencyTracker *tracker = nullptr) {
    return std::unique_ptr<SourceLoader>{
      new SourceLoader(ctx, enableResilience, tracker)
    };
  }

  SourceLoader(const SourceLoader &) = delete;
  SourceLoader(SourceLoader &&) = delete;
  SourceLoader &operator=(const SourceLoader &) = delete;
  SourceLoader &operator=(SourceLoader &&) = delete;

  /// Append visible module names to \p names. Note that names are possibly
  /// duplicated, and not guaranteed to be ordered in any way.
  void collectVisibleTopLevelModuleNames(
      SmallVectorImpl<Identifier> &names) const override;

  /// Check whether the module with a given name can be imported without
  /// importing it.
  ///
  /// Note that even if this check succeeds, errors may still occur if the
  /// module is loaded in full.
  ///
  /// If a non-null \p versionInfo is provided, the module version will be
  /// parsed and populated.
  virtual bool
  canImportModule(ImportPath::Module named, SourceLoc loc,
                  ModuleVersionInfo *versionInfo,
                  bool isTestableDependencyLookup = false) override;

  /// Import a module with the given module path.
  ///
  /// \param importLoc The location of the 'import' keyword.
  ///
  /// \param path A sequence of (identifier, location) pairs that denote
  /// the dotted module name to load, e.g., AppKit.NSWindow.
  ///
  /// \returns the module referenced, if it could be loaded. Otherwise,
  /// returns NULL.
  virtual ModuleDecl *
  loadModule(SourceLoc importLoc,
             ImportPath::Module path,
             bool AllowMemoryCache) override;

  /// Load extensions to the given nominal type.
  ///
  /// \param nominal The nominal type whose extensions should be loaded.
  ///
  /// \param previousGeneration The previous generation number. The AST already
  /// contains extensions loaded from any generation up to and including this
  /// one.
  virtual void loadExtensions(NominalTypeDecl *nominal,
                              unsigned previousGeneration) override;

  virtual void loadObjCMethods(
                 NominalTypeDecl *typeDecl,
                 ObjCSelector selector,
                 bool isInstanceMethod,
                 unsigned previousGeneration,
                 toolchain::TinyPtrVector<AbstractFunctionDecl *> &methods) override
  {
    // Parsing populates the Objective-C method tables.
  }
};
}

#endif
