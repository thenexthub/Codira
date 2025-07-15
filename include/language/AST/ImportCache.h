//===--- ImportCache.h - Caching the import graph ---------------*- C++ -*-===//
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
// This file defines interfaces for querying the module import graph in an
// efficient manner.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_AST_IMPORT_CACHE_H
#define LANGUAGE_AST_IMPORT_CACHE_H

#include "language/AST/Module.h"
#include "toolchain/ADT/DenseMap.h"
#include "toolchain/ADT/FoldingSet.h"

namespace language {
class DeclContext;

namespace namelookup {

/// An object describing a set of modules visible from a DeclContext.
///
/// This consists of two arrays of modules; the top-level imports and the
/// transitive imports.
///
/// The top-level imports contains all public imports of the parent module
/// of 'dc', together with any private imports in the source file containing
/// 'dc', if there is one.
///
/// The transitive imports contains all public imports reachable from the
/// set of top-level imports.
///
/// Both sets only contain unique elements. The top-level imports always
/// include the parent module of 'dc' explicitly.
///
/// The set of transitive imports does not contain any elements found in
/// the top-level imports.
///
/// The Codira standard library module is not included in either set unless
/// it was explicitly imported (or re-exported).
class ImportSet final :
    public toolchain::FoldingSetNode,
    private toolchain::TrailingObjects<ImportSet, ImportedModule> {
  friend TrailingObjects;
  friend class ImportCache;

  unsigned HasHeaderImportModule : 1;
  unsigned NumTopLevelImports : 31;
  unsigned NumTransitiveImports;
  unsigned NumTransitiveCodiraOnlyImports;

  ImportSet(bool hasHeaderImportModule,
            ArrayRef<ImportedModule> topLevelImports,
            ArrayRef<ImportedModule> transitiveImports,
            ArrayRef<ImportedModule> transitiveCodiraOnlyImports);

  ImportSet(const ImportSet &) = delete;
  void operator=(const ImportSet &) = delete;

public:
  void Profile(toolchain::FoldingSetNodeID &ID) {
    Profile(ID, getTopLevelImports());
  }
  static void Profile(
      toolchain::FoldingSetNodeID &ID,
      ArrayRef<ImportedModule> topLevelImports);

  size_t numTrailingObjects(OverloadToken<ImportedModule>) const {
    return NumTopLevelImports + NumTransitiveImports +
           NumTransitiveCodiraOnlyImports;
  }

  /// This is a bit of a hack to make module name lookup work properly.
  /// If our import set includes the ClangImporter's special header import
  /// module, we have to check it first, before any other imported module.
  bool hasHeaderImportModule() const {
    return HasHeaderImportModule;
  }

  ArrayRef<ImportedModule> getTopLevelImports() const {
    return {getTrailingObjects<ImportedModule>(),
            NumTopLevelImports};
  }

  ArrayRef<ImportedModule> getTransitiveImports() const {
    return {getTrailingObjects<ImportedModule>() +
              NumTopLevelImports,
            NumTransitiveImports};
  }

  ArrayRef<ImportedModule> getTransitiveCodiraOnlyImports() const {
    return {getTrailingObjects<ImportedModule>() +
              NumTopLevelImports + NumTransitiveImports,
            NumTransitiveCodiraOnlyImports};
  }

  ArrayRef<ImportedModule> getAllImports() const {
      return {getTrailingObjects<ImportedModule>(),
              NumTopLevelImports + NumTransitiveImports};
  }

  LANGUAGE_DEBUG_DUMP;
};

class alignas(ImportedModule) ImportCache {
  ImportCache(const ImportCache &) = delete;
  void operator=(const ImportCache &) = delete;

  toolchain::FoldingSet<ImportSet> ImportSets;
  toolchain::DenseMap<const DeclContext *, ImportSet *> ImportSetForDC;
  toolchain::DenseMap<std::tuple<const ModuleDecl *,
                            const DeclContext *>,
                 ArrayRef<ImportPath::Access>> VisibilityCache;
  toolchain::DenseMap<std::tuple<const ModuleDecl *,
                            const ModuleDecl *,
                            const DeclContext *>,
                 ArrayRef<ImportPath::Access>> ShadowCache;
  toolchain::DenseMap<std::tuple<const ModuleDecl *,
                            const DeclContext *>,
                 bool> CodiraOnlyCache;
  toolchain::DenseMap<const ModuleDecl *, ArrayRef<ModuleDecl *>> WeakCache;

  ImportPath::Access EmptyAccessPath;

  ArrayRef<ImportPath::Access> allocateArray(
      ASTContext &ctx,
      SmallVectorImpl<ImportPath::Access> &results);

  ArrayRef<ModuleDecl *> allocateArray(
      ASTContext &ctx,
      toolchain::SetVector<ModuleDecl *> &results);

  ImportSet &getImportSet(ASTContext &ctx,
                          ArrayRef<ImportedModule> topLevelImports);

public:
  ImportCache() {}

  /// Returns an object descripting all modules transitively imported
  /// from 'dc'.
  ImportSet &getImportSet(const DeclContext *dc);

  /// Returns all access paths into 'mod' that are visible from 'dc',
  /// including transitively, via re-exports.
  ArrayRef<ImportPath::Access>
  getAllVisibleAccessPaths(const ModuleDecl *mod, const DeclContext *dc);

  bool isImportedBy(const ModuleDecl *mod,
                    const DeclContext *dc) {
    return !getAllVisibleAccessPaths(mod, dc).empty();
  }

  /// Is `mod` imported from `dc` via a purely Codira access path?
  /// Always returns false if `dc` is a non-Codira module and only takes
  /// into account re-exports declared from Codira modules for transitive imports.
  bool isImportedByViaCodiraOnly(const ModuleDecl *mod,
                                const DeclContext *dc);

  /// Returns all access paths in 'mod' that are visible from 'dc' if we
  /// subtract imports of 'other'.
  ArrayRef<ImportPath::Access>
  getAllAccessPathsNotShadowedBy(const ModuleDecl *mod,
                                 const ModuleDecl *other,
                                 const DeclContext *dc);

  /// Returns all weak-linked imported modules.
  ArrayRef<ModuleDecl *>
  getWeakImports(const ModuleDecl *mod);

  bool isWeakImportedBy(const ModuleDecl *mod,
                        const ModuleDecl *from);

  /// This is a hack to cope with main file parsing and REPL parsing, where
  /// we can add ImportDecls after import resolution.
  void clear() {
    ImportSetForDC.clear();
  }
};

ArrayRef<ImportedModule> getAllImports(const DeclContext *dc);

}  // namespace namelookup

}  // namespace language

#endif
