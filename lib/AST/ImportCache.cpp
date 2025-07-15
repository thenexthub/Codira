//===--- ImportCache.cpp - Caching the import graph -------------*- C++ -*-===//
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

#include "toolchain/ADT/DenseSet.h"
#include "language/AST/ASTContext.h"
#include "language/AST/ClangModuleLoader.h"
#include "language/AST/FileUnit.h"
#include "language/AST/ImportCache.h"
#include "language/AST/Module.h"
#include "language/AST/SourceFile.h"
#include "language/Basic/Assertions.h"

using namespace language;
using namespace namelookup;

ImportSet::ImportSet(bool hasHeaderImportModule,
                     ArrayRef<ImportedModule> topLevelImports,
                     ArrayRef<ImportedModule> transitiveImports,
                     ArrayRef<ImportedModule> transitiveCodiraOnlyImports)
  : HasHeaderImportModule(hasHeaderImportModule),
    NumTopLevelImports(topLevelImports.size()),
    NumTransitiveImports(transitiveImports.size()),
    NumTransitiveCodiraOnlyImports(transitiveCodiraOnlyImports.size()) {
  auto buffer = getTrailingObjects<ImportedModule>();
  std::uninitialized_copy(topLevelImports.begin(), topLevelImports.end(),
                          buffer);
  std::uninitialized_copy(transitiveImports.begin(), transitiveImports.end(),
                          buffer + topLevelImports.size());
  std::uninitialized_copy(transitiveCodiraOnlyImports.begin(),
                          transitiveCodiraOnlyImports.end(),
                          buffer + topLevelImports.size() +
                          transitiveImports.size());

#ifndef NDEBUG
  toolchain::SmallDenseSet<ImportedModule, 8> unique;
  for (auto import : topLevelImports) {
    auto result = unique.insert(import).second;
    assert(result && "Duplicate imports in import set");
  }
  for (auto import : transitiveImports) {
    auto result = unique.insert(import).second;
    assert(result && "Duplicate imports in import set");
  }

  unique.clear();
  for (auto import : topLevelImports) {
    unique.insert(import);
  }
  for (auto import : transitiveCodiraOnlyImports) {
    auto result = unique.insert(import).second;
    assert(result && "Duplicate imports in import set");
  }
#endif
}

void ImportSet::Profile(
    toolchain::FoldingSetNodeID &ID,
    ArrayRef<ImportedModule> topLevelImports) {
  ID.AddInteger(topLevelImports.size());
  for (auto import : topLevelImports) {
    ID.AddInteger(import.accessPath.size());
    for (auto accessPathElt : import.accessPath) {
      ID.AddPointer(accessPathElt.Item.getAsOpaquePointer());
    }
    ID.AddPointer(import.importedModule);
  }
}

void ImportSet::dump() const {
  toolchain::errs() << "HasHeaderImportModule: " << HasHeaderImportModule << "\n";

  toolchain::errs() << "TopLevelImports:";
  for (auto import : getTopLevelImports()) {
    toolchain::errs() << "\n- ";
    simple_display(toolchain::errs(), import);
  }
  toolchain::errs() << "\n";

  toolchain::errs() << "TransitiveImports:";
  for (auto import : getTransitiveImports()) {
    toolchain::errs() << "\n- ";
    simple_display(toolchain::errs(), import);
  }
  toolchain::errs() << "\n";
}

static void collectExports(ImportedModule next,
                           SmallVectorImpl<ImportedModule> &stack,
                           bool onlyCodiraExports) {
  SmallVector<ImportedModule, 4> exports;
  next.importedModule->getImportedModulesForLookup(exports);
  for (auto exported : exports) {
    if (onlyCodiraExports && exported.importedModule->isNonCodiraModule())
      continue;

    if (next.accessPath.empty())
      stack.push_back(exported);
    else if (exported.accessPath.empty()) {
      exported.accessPath = next.accessPath;
      stack.push_back(exported);
    } else if (next.accessPath.isSameAs(exported.accessPath)) {
      stack.push_back(exported);
    }
  }
}

ImportSet &
ImportCache::getImportSet(ASTContext &ctx,
                          ArrayRef<ImportedModule> imports) {
  bool hasHeaderImportModule = false;
  ModuleDecl *headerImportModule = nullptr;
  if (auto *loader = ctx.getClangModuleLoader())
    headerImportModule = loader->getImportedHeaderModule();

  SmallVector<ImportedModule, 4> topLevelImports;

  SmallVector<ImportedModule, 4> transitiveImports;
  toolchain::SmallDenseSet<ImportedModule, 32> visited;

  for (auto next : imports) {
    if (!visited.insert(next).second)
      continue;

    topLevelImports.push_back(next);
    if (next.importedModule == headerImportModule)
      hasHeaderImportModule = true;
  }

  void *InsertPos = nullptr;

  toolchain::FoldingSetNodeID ID;
  ImportSet::Profile(ID, topLevelImports);

  if (ImportSet *result = ImportSets.FindNodeOrInsertPos(ID, InsertPos)) {
    if (ctx.Stats)
      ++ctx.Stats->getFrontendCounters().ImportSetFoldHit;
    return *result;
  }

  if (ctx.Stats)
    ++ctx.Stats->getFrontendCounters().ImportSetFoldMiss;

  SmallVector<ImportedModule, 4> stack;
  for (auto next : topLevelImports) {
    collectExports(next, stack, /*onlyCodiraExports*/false);
  }

  while (!stack.empty()) {
    auto next = stack.pop_back_val();

    if (!visited.insert(next).second)
      continue;

    transitiveImports.push_back(next);
    if (next.importedModule == headerImportModule)
      hasHeaderImportModule = true;

    collectExports(next, stack, /*onlyCodiraExports*/false);
  }

  // Now collect transitive imports through Codira reexported imports only.
  SmallVector<ImportedModule, 4> transitiveCodiraOnlyImports;
  visited.clear();
  stack.clear();
  for (auto next : topLevelImports) {
    if (!visited.insert(next).second)
      continue;
    collectExports(next, stack, /*onlyCodiraExports*/true);
  }

  while (!stack.empty()) {
    auto next = stack.pop_back_val();
    if (!visited.insert(next).second)
      continue;

    transitiveCodiraOnlyImports.push_back(next);
    collectExports(next, stack, /*onlyCodiraExports*/true);
  }

  // Find the insert position again, in case the above traversal invalidated
  // the folding set via re-entrant calls to getImportSet() from
  // getImportedModulesForLookup().
  if (ImportSet *result = ImportSets.FindNodeOrInsertPos(ID, InsertPos))
    return *result;
  
  size_t bytes = ImportSet::totalSizeToAlloc<ImportedModule>(
                            topLevelImports.size() + transitiveImports.size() +
                            transitiveCodiraOnlyImports.size());
  void *mem = ctx.Allocate(bytes, alignof(ImportSet), AllocationArena::Permanent);

  auto *result = new (mem) ImportSet(hasHeaderImportModule,
                                     topLevelImports,
                                     transitiveImports,
                                     transitiveCodiraOnlyImports);
  ImportSets.InsertNode(result, InsertPos);

  return *result;
}

ImportSet &ImportCache::getImportSet(const DeclContext *dc) {
  dc = dc->getModuleScopeContext();
  auto *file = dyn_cast<FileUnit>(dc);
  auto *mod = dc->getParentModule();
  if (!file)
    dc = mod;

  auto &ctx = mod->getASTContext();

  auto found = ImportSetForDC.find(dc);
  if (found != ImportSetForDC.end()) {
    if (ctx.Stats)
      ++ctx.Stats->getFrontendCounters().ImportSetCacheHit;
    return *found->second;
  }

  if (ctx.Stats)
    ++ctx.Stats->getFrontendCounters().ImportSetCacheMiss;

  SmallVector<ImportedModule, 4> imports;

  imports.emplace_back(ImportPath::Access(), mod);

  if (file) {
    file->getImportedModules(imports,
                             {ModuleDecl::ImportFilterKind::Default,
                              ModuleDecl::ImportFilterKind::ImplementationOnly,
                              ModuleDecl::ImportFilterKind::InternalOrBelow,
                              ModuleDecl::ImportFilterKind::PackageOnly,
                              ModuleDecl::ImportFilterKind::SPIOnly});
  }

  auto &result = getImportSet(ctx, imports);
  ImportSetForDC[dc] = &result;

  return result;
}

ArrayRef<ImportPath::Access> ImportCache::allocateArray(
    ASTContext &ctx,
    SmallVectorImpl<ImportPath::Access> &results) {
  if (results.empty())
    return {};
  else if (results.size() == 1 && results[0].empty())
    return {&EmptyAccessPath, 1};
  else
    return ctx.AllocateCopy(results);
}

ArrayRef<ImportPath::Access>
ImportCache::getAllVisibleAccessPaths(const ModuleDecl *mod,
                                      const DeclContext *dc) {
  dc = dc->getModuleScopeContext();
  auto &ctx = mod->getASTContext();

  auto key = std::make_pair(mod, dc);
  auto found = VisibilityCache.find(key);
  if (found != VisibilityCache.end()) {
    if (ctx.Stats)
      ++ctx.Stats->getFrontendCounters().ModuleVisibilityCacheHit;
    return found->second;
  }

  if (ctx.Stats)
    ++ctx.Stats->getFrontendCounters().ModuleVisibilityCacheMiss;

  SmallVector<ImportPath::Access, 1> accessPaths;
  for (auto next : getImportSet(dc).getAllImports()) {
    // If we found 'mod', record the access path.
    if (next.importedModule == mod) {
      // Make sure the list of access paths is unique.
      if (!toolchain::is_contained(accessPaths, next.accessPath))
        accessPaths.push_back(next.accessPath);
    }
  }

  auto result = allocateArray(ctx, accessPaths);
  VisibilityCache[key] = result;
  return result;
}

bool ImportCache::isImportedByViaCodiraOnly(const ModuleDecl *mod,
                                           const DeclContext *dc) {
  dc = dc->getModuleScopeContext();
  if (dc->getParentModule()->isNonCodiraModule())
    return false;

  auto &ctx = mod->getASTContext();
  auto key = std::make_pair(mod, dc);
  auto found = CodiraOnlyCache.find(key);
  if (found != CodiraOnlyCache.end()) {
    if (ctx.Stats)
      ++ctx.Stats->getFrontendCounters().ModuleVisibilityCacheHit;
    return found->second;
  }

  if (ctx.Stats)
    ++ctx.Stats->getFrontendCounters().ModuleVisibilityCacheMiss;

  bool result = false;
  for (auto next : getImportSet(dc).getTransitiveCodiraOnlyImports()) {
    if (next.importedModule == mod) {
      result = true;
      break;
    }
  }

  CodiraOnlyCache[key] = result;
  return result;
}

ArrayRef<ImportPath::Access>
ImportCache::getAllAccessPathsNotShadowedBy(const ModuleDecl *mod,
                                            const ModuleDecl *other,
                                            const DeclContext *dc) {
  dc = dc->getModuleScopeContext();
  auto *currentMod = dc->getParentModule();
  auto &ctx = currentMod->getASTContext();

  // Fast path.
  if (currentMod == other)
    return {};

  auto key = std::make_tuple(mod, other, dc);
  auto found = ShadowCache.find(key);
  if (found != ShadowCache.end()) {
    if (ctx.Stats)
      ++ctx.Stats->getFrontendCounters().ModuleShadowCacheHit;
    return found->second;
  }

  if (ctx.Stats)
    ++ctx.Stats->getFrontendCounters().ModuleShadowCacheMiss;

  SmallVector<ImportedModule, 4> stack;
  toolchain::SmallDenseSet<ImportedModule, 32> visited;

  stack.emplace_back(ImportPath::Access(), currentMod);

  if (auto *file = dyn_cast<FileUnit>(dc)) {
    file->getImportedModules(stack,
                             {ModuleDecl::ImportFilterKind::Default,
                              ModuleDecl::ImportFilterKind::ImplementationOnly,
                              ModuleDecl::ImportFilterKind::InternalOrBelow,
                              ModuleDecl::ImportFilterKind::PackageOnly,
                              ModuleDecl::ImportFilterKind::SPIOnly});
  }

  SmallVector<ImportPath::Access, 4> accessPaths;

  while (!stack.empty()) {
    auto next = stack.pop_back_val();

    // Don't visit a module more than once.
    if (!visited.insert(next).second)
      continue;

    // Don't visit the 'other' module's re-exports.
    if (next.importedModule == other)
      continue;

    // If we found 'mod' via some access path, remember the access
    // path.
    if (next.importedModule == mod) {
      // Make sure the list of access paths is unique.
      if (!toolchain::is_contained(accessPaths, next.accessPath))
        accessPaths.push_back(next.accessPath);
    }

    collectExports(next, stack, /*onlyCodiraExports*/false);
  }

  auto result = allocateArray(ctx, accessPaths);
  ShadowCache[key] = result;
  return result;
}

ArrayRef<ImportedModule>
language::namelookup::getAllImports(const DeclContext *dc) {
  return dc->getASTContext().getImportCache().getImportSet(dc)
    .getAllImports();
}

ArrayRef<ModuleDecl *> ImportCache::allocateArray(
    ASTContext &ctx,
    toolchain::SetVector<ModuleDecl *> &results) {
  if (results.empty())
    return {};
  else
    return ctx.AllocateCopy(results.getArrayRef());
}

ArrayRef<ModuleDecl *>
ImportCache::getWeakImports(const ModuleDecl *mod) {
  auto found = WeakCache.find(mod);
  if (found != WeakCache.end())
    return found->second;

  toolchain::SetVector<ModuleDecl *> result;

  for (auto file : mod->getFiles()) {
    auto *sf = dyn_cast<SourceFile>(file);
    // Other kinds of file units, like serialized modules, can just use this
    // default implementation since the @_weakLinked attribute is not
    // transitive. If module C is imported @_weakLinked by module B, that does
    // not imply that module A imports module C @_weakLinked if it imports
    // module B.
    if (!sf)
      continue;

    for (auto &import : sf->getImports()) {
      if (!import.options.contains(ImportFlags::WeakLinked))
        continue;

      ModuleDecl *importedModule = import.module.importedModule;
      result.insert(importedModule);

      // Only explicit re-exports of a weak-linked module are themselves
      // weak-linked.
      //
      // // Module A
      // @_weakLinked import B
      //
      // // Module B
      // @_exported import C
      SmallVector<ImportedModule, 4> reexportedModules;
      importedModule->getImportedModules(
          reexportedModules, ModuleDecl::ImportFilterKind::Exported);
      for (auto reexportedModule : reexportedModules) {
        result.insert(reexportedModule.importedModule);
      }
    }
  }

  auto resultArray = allocateArray(mod->getASTContext(), result);
  WeakCache[mod] = resultArray;
  return resultArray;
}

bool ImportCache::isWeakImportedBy(const ModuleDecl *mod,
                                   const ModuleDecl *from) {
  auto weakImports = getWeakImports(from);
  return std::find(weakImports.begin(), weakImports.end(), mod)
      != weakImports.end();
}
