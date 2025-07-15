//===--- ModuleTrace.cpp -- Emit a trace of all loaded Codira modules ------===//
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

#include "Dependencies.h"
#include "language/AST/ASTContext.h"
#include "language/AST/DiagnosticEngine.h"
#include "language/AST/DiagnosticsFrontend.h"
#include "language/AST/Module.h"
#include "language/AST/ModuleLoader.h"
#include "language/AST/PluginRegistry.h"
#include "language/AST/SourceFile.h"
#include "language/Basic/Assertions.h"
#include "language/Basic/FileTypes.h"
#include "language/Basic/JSONSerialization.h"
#include "language/Basic/SourceManager.h"
#include "language/Frontend/Frontend.h"
#include "language/Frontend/FrontendOptions.h"
#include "language/Frontend/ModuleInterfaceSupport.h"
#include "language/IDE/SourceEntityWalker.h"

#include "clang/AST/DeclObjC.h"
#include "clang/AST/ObjCMethodReferenceInfo.h"
#include "clang/Basic/Module.h"

#include "toolchain/ADT/SmallPtrSet.h"
#include "toolchain/ADT/SmallString.h"
#include "toolchain/Support/JSON.h"
#include "toolchain/Support/Path.h"
#include "toolchain/Support/YAMLTraits.h"
#include "toolchain/Support/FileUtilities.h"

#if !defined(_MSC_VER) && !defined(__MINGW32__)
#include <unistd.h>
#else
#include <io.h>
#endif

using namespace language;

namespace {
struct CodiraModuleTraceInfo {
  Identifier Name;
  std::string Path;
  bool IsImportedDirectly;
  bool SupportsLibraryEvolution;
  bool StrictMemorySafety;
};

struct CodiraMacroTraceInfo {
  Identifier Name;
  std::string Path;
};

struct LoadedModuleTraceFormat {
  static const unsigned CurrentVersion = 2;
  unsigned Version;
  Identifier Name;
  std::string Arch;
  std::string LanguageMode;
  std::vector<StringRef> EnabledLanguageFeatures;
  bool StrictMemorySafety;
  std::vector<CodiraModuleTraceInfo> CodiraModules;
  std::vector<CodiraMacroTraceInfo> CodiraMacros;
};
} // namespace

namespace language {
namespace json {
template <> struct ObjectTraits<CodiraModuleTraceInfo> {
  static void mapping(Output &out, CodiraModuleTraceInfo &contents) {
    StringRef name = contents.Name.str();
    out.mapRequired("name", name);
    out.mapRequired("path", contents.Path);
    out.mapRequired("isImportedDirectly", contents.IsImportedDirectly);
    out.mapRequired("supportsLibraryEvolution",
                    contents.SupportsLibraryEvolution);
    out.mapRequired("strictMemorySafety",
                    contents.StrictMemorySafety);
  }
};

template <>
struct ObjectTraits<CodiraMacroTraceInfo> {
  static void mapping(Output &out, CodiraMacroTraceInfo &contents) {
    StringRef name = contents.Name.str();
    out.mapRequired("name", name);
    out.mapRequired("path", contents.Path);
  }
};

// Version notes:
// 1. Keys: name, arch, languagemodules
// 2. New keys: version, languagemodulesDetailedInfo
template <> struct ObjectTraits<LoadedModuleTraceFormat> {
  static void mapping(Output &out, LoadedModuleTraceFormat &contents) {
    out.mapRequired("version", contents.Version);

    StringRef name = contents.Name.str();
    out.mapRequired("name", name);

    out.mapRequired("arch", contents.Arch);

    out.mapRequired("languageMode", contents.codeMode);

    out.mapRequired("enabledLanguageFeatures",
                    contents.EnabledLanguageFeatures);

    out.mapRequired("strictMemorySafety", contents.StrictMemorySafety);

    // The 'languagemodules' key is kept for backwards compatibility.
    std::vector<std::string> moduleNames;
    for (auto &m : contents.CodiraModules)
      moduleNames.push_back(m.Path);
    out.mapRequired("languagemodules", moduleNames);

    out.mapRequired("languagemodulesDetailedInfo", contents.CodiraModules);

    out.mapRequired("languagemacros", contents.CodiraMacros);
  }
};
} // namespace json
} // namespace language

static bool isClangOverlayOf(ModuleDecl *potentialOverlay,
                             ModuleDecl *potentialUnderlying) {
  return !potentialOverlay->isNonCodiraModule() &&
         potentialUnderlying->isNonCodiraModule() &&
         potentialOverlay->getName() == potentialUnderlying->getName();
}

// TODO: Delete this once changes from https://reviews.toolchain.org/D83449 land on
// apple/toolchain-project's language/main branch.
template <typename SetLike, typename Item>
static bool contains(const SetLike &setLike, Item item) {
  return setLike.find(item) != setLike.end();
}

/// Get a set of modules imported by \p module.
///
/// By default, all imports are included.
static void getImmediateImports(
    ModuleDecl *module, SmallPtrSetImpl<ModuleDecl *> &imports,
    ModuleDecl::ImportFilter importFilter = ModuleDecl::getImportFilterAll()) {
  SmallVector<ImportedModule, 8> importList;
  module->getImportedModules(importList, importFilter);

  for (ImportedModule &import : importList)
    imports.insert(import.importedModule);
}

namespace {
/// Helper type for computing (approximate) information about ABI-dependencies.
///
/// This misses out on details such as typealiases and more.
/// See the "isImportedDirectly" field above for more details.
class ABIDependencyEvaluator {
  /// Map of ABIs exported by a particular module, excluding itself.
  ///
  /// For example, consider (primed letters represent Clang modules):
  /// \code
  /// - A is @_exported-imported by B
  /// - B is #imported by C' (via a compiler-generated umbrella header)
  /// - C' is @_exported-imported by C (Codira overlay)
  /// - D' is #imported by E'
  /// - D' is @_exported-imported by D (Codira overlay)
  /// - E' is @_exported-imported by E (Codira overlay)
  /// \endcode
  ///
  /// Then the \c abiExportMap will be
  /// \code
  /// { A: {}, B: {A}, C: {B}, C': {B}, D: {}, D': {}, E: {D}, E': {D'} }
  /// \endcode
  ///
  /// \b WARNING: Use \c reexposeImportedABI instead of inserting directly.
  toolchain::DenseMap<ModuleDecl *, toolchain::DenseSet<ModuleDecl *>> abiExportMap;

  /// Stack for depth-first traversal.
  SmallVector<ModuleDecl *, 32> searchStack;

  toolchain::DenseSet<ModuleDecl *> visited;

  /// Helper function to handle invariant violations as crashes in debug mode.
  void
  crashOnInvariantViolation(toolchain::function_ref<void(raw_ostream &)> f) const;

  /// Computes the ABI exports for \p importedModule and adds them to
  /// \p module's ABI exports.
  ///
  /// If \p includeImportedModule is true, also adds \p importedModule to
  /// \p module's ABI exports.
  ///
  /// Correct way to add entries to \c abiExportMap.
  void reexposeImportedABI(ModuleDecl *module, ModuleDecl *importedModule,
                           bool includeImportedModule = true);

  /// Check if a Codira module is an overlay for some Clang module.
  ///
  /// FIXME: Delete this hack once https://github.com/apple/language/issues/55804 is fixed and ModuleDecl has the right API which we can use directly.
  bool isOverlayOfClangModule(ModuleDecl *languageModule);

  /// Check for cases where we have a fake cycle through an overlay.
  ///
  /// Sometimes, we have fake cycles in the import graph due to the Clang
  /// importer injecting overlays between Clang modules. These don't represent
  /// an actual cycle in the build, so we should ignore them.
  ///
  /// We check this lazily after detecting a cycle because it is difficult to
  /// determine at the point where we see the overlay whether it was incorrectly
  /// injected by the Clang importer or whether any of its imports will
  /// eventually lead to a cycle.
  ///
  /// For more details, see [NOTE: ABIDependencyEvaluator-fake-cycle-detection]
  ///
  /// \param startOfCycle A pointer to the element of \c searchStack where
  ///        the module \em first appeared.
  ///
  /// \pre The module on top of \c searchStack is the same module as
  ///      *startOfCycle.
  ///
  /// \pre searchStack.begin() <= startOfCycle < searchStack.end()
  bool isFakeCycleThroughOverlay(ModuleDecl **startOfCycle);

  /// Recursive step in computing ABI dependencies.
  ///
  /// Use this method instead of using the \c forClangModule/\c forCodiraModule
  /// methods.
  void computeABIDependenciesForModule(ModuleDecl *module);
  void computeABIDependenciesForCodiraModule(ModuleDecl *module);
  void computeABIDependenciesForClangModule(ModuleDecl *module);

  static void printModule(const ModuleDecl *module, toolchain::raw_ostream &os);

  template <typename SetLike>
  static void printModuleSet(const SetLike &set, toolchain::raw_ostream &os);

public:
  ABIDependencyEvaluator() = default;
  ABIDependencyEvaluator(const ABIDependencyEvaluator &) = delete;
  ABIDependencyEvaluator(ABIDependencyEvaluator &&) = default;

  void getABIDependenciesForCodiraModule(
      ModuleDecl *module, SmallPtrSetImpl<ModuleDecl *> &abiDependencies);

  void printABIExportMap(toolchain::raw_ostream &os) const;
};
} // end anonymous namespace

// See [NOTE: Bailing-vs-crashing-in-trace-emission].
// TODO: Use PrettyStackTrace instead?
void ABIDependencyEvaluator::crashOnInvariantViolation(
    toolchain::function_ref<void(raw_ostream &)> f) const {
#ifndef NDEBUG
  SmallVector<char, 0> msg;
  toolchain::raw_svector_ostream os(msg);
  os << "error: invariant violation: ";
  f(os);
  toolchain::report_fatal_error(msg);
#endif
}

// [NOTE: Trace-Clang-submodule-complexity]
//
// A Clang module may have zero or more submodules. In practice, when traversing
// the imports of a module, we observe that different submodules of the same
// top-level module (almost) freely import each other. Despite this, we still
// need to conceptually traverse the tree formed by the submodule relationship
// (with the top-level module being the root).
//
// This needs to be taken care of in two ways:
// 1. We need to make sure we only go towards the leaves. It's okay if we "jump"
//    branches, so long as we don't try to visit an ancestor when one of its
//    descendants is still on the traversal stack, so that we don't end up with
//    arbitrarily complex intra-module cycles.
//    See also: [NOTE: Intra-module-leafwards-traversal].
// 2. When adding entries to the ABI export map, we need to avoid marking
//    dependencies within the same top-level module. This step is needed in
//    addition to step 1 to avoid creating cycles like
//    Overlay -> Underlying -> Submodule -> Overlay.

void ABIDependencyEvaluator::reexposeImportedABI(ModuleDecl *module,
                                                 ModuleDecl *importedModule,
                                                 bool includeImportedModule) {
  if (module == importedModule) {
    crashOnInvariantViolation([&](raw_ostream &os) {
      os << "module ";
      printModule(module, os);
      os << " imports itself!\n";
    });
    return;
  }

  auto addToABIExportMap = [this](ModuleDecl *module, ModuleDecl *reexport) {
    if (module == reexport) {
      crashOnInvariantViolation([&](raw_ostream &os) {
        os << "expected module ";
        printModule(reexport, os);
        os << "  to not re-export itself\n";
      });
      return;
    }
    if (reexport->isNonCodiraModule() && module->isNonCodiraModule() &&
        module->getTopLevelModule() == reexport->getTopLevelModule()) {
      // Dependencies within the same top-level Clang module are not useful.
      // See also: [NOTE: Trace-Clang-submodule-complexity].
      return;
    }

    // We only care about dependencies across top-level modules and we want to
    // avoid exploding abiExportMap with submodules. So we only insert entries
    // after calling getTopLevelModule().

    if (::isClangOverlayOf(module, reexport)) {
      // For overlays, we need to have a dependency on the underlying module.
      // Otherwise, we might accidentally create a Codira -> Codira cycle.
      abiExportMap[module].insert(
          reexport->getTopLevelModule(/*preferOverlay*/ false));
      return;
    }
    abiExportMap[module].insert(
        reexport->getTopLevelModule(/*preferOverlay*/ true));
  };

  computeABIDependenciesForModule(importedModule);
  if (includeImportedModule) {
    addToABIExportMap(module, importedModule);
  }
  // Force creation of default value if missing. This prevents abiExportMap from
  // growing (and moving) when calling addToABIExportMap. If abiExportMap gets
  // moved, then abiExportMap[importedModule] will be moved, forcing us to
  // create a defensive copy to avoid iterator invalidation on move.
  (void)abiExportMap[module];
  for (auto reexportedModule : abiExportMap[importedModule])
    addToABIExportMap(module, reexportedModule);
}

bool ABIDependencyEvaluator::isOverlayOfClangModule(ModuleDecl *languageModule) {
  assert(!languageModule->isNonCodiraModule());

  toolchain::SmallPtrSet<ModuleDecl *, 8> importList;
  ::getImmediateImports(languageModule, importList,
                        {ModuleDecl::ImportFilterKind::Exported});
  bool isOverlay =
      toolchain::any_of(importList, [&](ModuleDecl *importedModule) -> bool {
        return isClangOverlayOf(languageModule, importedModule);
      });
  return isOverlay;
}

// [NOTE: ABIDependencyEvaluator-fake-cycle-detection]
//
// First, let's consider a concrete example.
// - In Clang-land, ToyKit #imports CoreDoll.
// - The Codira overlay for CoreDoll imports both CoreDoll and ToyKit.
// Importing ToyKit from CoreDoll's overlay informally violates the layering
// of frameworks, but it doesn't actually create any cycles in the build
// dependencies.
//                        ┌───────────────────────────┐
//                    ┌───│    CoreDoll.codemodule   │
//                    │   └───────────────────────────┘
//                    │                 │
//              import ToyKit     @_exported import CoreDoll
//                    │                 │
//                    │                 │
//                    ▼                 │
//      ┌──────────────────────────┐    │
//      │ ToyKit (ToyKit/ToyKit.h) │    │
//      └──────────────────────────┘    │
//                    │                 │
//       #import <CoreDoll/CoreDoll.h>  │
//                    │                 │
//                    ▼                 │
//   ┌──────────────────────────────┐   │
//   │CoreDoll (CoreDoll/CoreDoll.h)│◀──┘
//   └──────────────────────────────┘
//
// Say we are trying to build a Codira module that imports ToyKit. Due to how
// module loading works, the Clang importer inserts the CoreDoll overlay
// between the ToyKit and CoreDoll Clang modules, creating a cycle in the
// import graph.
//
//   ┌──────────────────────────┐
//   │ ToyKit (ToyKit/ToyKit.h) │◀──────────┐
//   └──────────────────────────┘           │
//                 │                        │
//    #import <CoreDoll/CoreDoll.h>    import ToyKit
//                 │                        │
//                 ▼                        │
//   ┌────────────────────────────┐         │
//   │    CoreDoll.codemodule    │─────────┘
//   └────────────────────────────┘
//                 │
//     @_exported import CoreDoll
//                 │
//                 ▼
//   ┌──────────────────────────────┐
//   │CoreDoll (CoreDoll/CoreDoll.h)│
//   └──────────────────────────────┘
//
// This means that, at some point, searchStack will look like:
//
//   [others] → ToyKit → CoreDoll (overlay) → ToyKit
//
// In the general case, there may be arbitrarily many modules in the cycle,
// including submodules.
//
//   [others] → ToyKit → [others] → CoreDoll (overlay) → [others] → ToyKit
//
// where "[others]" indicates 0 or more modules of any kind.
//
// To detect this, we check that the start of the cycle is a Clang module and
// that there is at least one overlay between it and its recurrence at the end
// of the searchStack. If so, we assume we have detected a benign cycle which
// can be safely ignored.

bool ABIDependencyEvaluator::isFakeCycleThroughOverlay(
    ModuleDecl **startOfCycle) {
  assert(startOfCycle >= searchStack.begin() &&
         startOfCycle < searchStack.end() &&
         "startOfCycleIter points to an element in searchStack");
  // The startOfCycle module must be a Clang module.
  if (!(*startOfCycle)->isNonCodiraModule())
    return false;
  // Next, we must have zero or more modules followed by a Codira overlay for a
  // Clang module.
  return std::any_of(
      startOfCycle + 1, searchStack.end(), [this](ModuleDecl *module) {
        return !module->isNonCodiraModule() && isOverlayOfClangModule(module);
      });
}

void ABIDependencyEvaluator::computeABIDependenciesForModule(
    ModuleDecl *module) {
  auto moduleIter = toolchain::find(searchStack, module);
  if (moduleIter != searchStack.end()) {
    if (isFakeCycleThroughOverlay(moduleIter))
      return;
    crashOnInvariantViolation([&](raw_ostream &os) {
      os << "unexpected cycle in import graph!\n";
      for (auto m : searchStack) {
        printModule(m, os);
        if (!m->isNonCodiraModule()) {
          os << " (isOverlay = " << isOverlayOfClangModule(m) << ")";
        }
        os << "\ndepends on ";
      }
      printModule(module, os);
      os << '\n';
    });
    return;
  }
  if (::contains(visited, module))
    return;
  searchStack.push_back(module);
  if (module->isNonCodiraModule())
    computeABIDependenciesForClangModule(module);
  else
    computeABIDependenciesForCodiraModule(module);
  searchStack.pop_back();
  visited.insert(module);
}

void ABIDependencyEvaluator::computeABIDependenciesForCodiraModule(
    ModuleDecl *module) {
  SmallPtrSet<ModuleDecl *, 32> allImports;
  ::getImmediateImports(module, allImports);
  for (auto import : allImports) {
    computeABIDependenciesForModule(import);
    if (::isClangOverlayOf(module, import)) {
      reexposeImportedABI(module, import,
                          /*includeImportedModule=*/false);
    }
  }

  SmallPtrSet<ModuleDecl *, 32> reexportedImports;
  ::getImmediateImports(module, reexportedImports,
                        {ModuleDecl::ImportFilterKind::Exported});
  for (auto reexportedImport : reexportedImports) {
    reexposeImportedABI(module, reexportedImport);
  }
}

void ABIDependencyEvaluator::computeABIDependenciesForClangModule(
    ModuleDecl *module) {
  SmallPtrSet<ModuleDecl *, 32> imports;
  ::getImmediateImports(module, imports);
  for (auto import : imports) {
    // There are three cases here which can potentially create cycles:
    //
    // 1. Clang modules importing the stdlib.
    //    See [NOTE: Pure-Clang-modules-privately-import-stdlib].
    // 2. Overlay S @_exported-imports underlying module S' and another Clang
    //    module C'. C' (transitively) #imports S' but it gets treated as if
    //    C' imports S. This creates a cycle: S -> C' -> ... -> S.
    //    In practice, this case is hit for
    //      Darwin (Codira) -> CodiraOverlayShims (Clang) -> Darwin (Codira).
    //    We may also hit this in a slightly different direction, in case
    //    the module directly imports CodiraOverlayShims:
    //      CodiraOverlayShims -> Darwin (Codira) -> CodiraOverlayShims
    //    The latter is handled later by isFakeCycleThroughOverlay.
    // 3. [NOTE: Intra-module-leafwards-traversal]
    //    Cycles within the same top-level module.
    //    These don't matter for us, since we only care about the dependency
    //    graph at the granularity of top-level modules. So we ignore these
    //    by only considering parent -> submodule dependencies.
    //    See also [NOTE: Trace-Clang-submodule-complexity].
    if (import->isStdlibModule()) {
      continue;
    }
    if (!import->isNonCodiraModule() && isOverlayOfClangModule(import) &&
        toolchain::find(searchStack, import) != searchStack.end()) {
      continue;
    }
    if (import->isNonCodiraModule() &&
        module->getTopLevelModule() == import->getTopLevelModule() &&
        (module == import ||
         !import->findUnderlyingClangModule()->isSubModuleOf(
             module->findUnderlyingClangModule()))) {
      continue;
    }
    computeABIDependenciesForModule(import);
    reexposeImportedABI(module, import);
  }
}

void ABIDependencyEvaluator::getABIDependenciesForCodiraModule(
    ModuleDecl *module, SmallPtrSetImpl<ModuleDecl *> &abiDependencies) {
  computeABIDependenciesForModule(module);
  SmallPtrSet<ModuleDecl *, 32> allImports;
  ::getImmediateImports(module, allImports);
  for (auto directDependency : allImports) {
    abiDependencies.insert(directDependency);
    for (auto exposedDependency : abiExportMap[directDependency]) {
      abiDependencies.insert(exposedDependency);
    }
  }
}

void ABIDependencyEvaluator::printModule(const ModuleDecl *module,
                                         toolchain::raw_ostream &os) {
  module->getReverseFullModuleName().printForward(os);
  os << (module->isNonCodiraModule() ? " (Clang)" : " (Codira)");
  os << " @ " << toolchain::format("0x%llx", reinterpret_cast<uintptr_t>(module));
}

template <typename SetLike>
void ABIDependencyEvaluator::printModuleSet(const SetLike &set,
                                            toolchain::raw_ostream &os) {
  os << "{ ";
  for (auto module : set) {
    printModule(module, os);
    os << ", ";
  }
  os << "}";
}

void ABIDependencyEvaluator::printABIExportMap(toolchain::raw_ostream &os) const {
  os << "ABI Export Map {{\n";
  for (auto &entry : abiExportMap) {
    printModule(entry.first, os);
    os << " : ";
    printModuleSet(entry.second, os);
    os << "\n";
  }
  os << "}}\n";
}

/// Compute the per-module information to be recorded in the trace file.
//
// The most interesting/tricky thing here is _which_ paths get recorded in
// the trace file as dependencies. It depends on how the module was synthesized.
// The key points are:
//
// 1. Paths to languagemodules in the module cache or in the prebuilt cache are not
//    recorded - Precondition: the corresponding path to the languageinterface must
//    already be present as a key in pathToModuleDecl.
// 2. languagemodules next to a languageinterface are saved if they are up-to-date.
//
// FIXME: Use the VFS instead of handling paths directly. We are particularly
// sloppy about handling relative paths in the dependency tracker.
static void computeCodiraModuleTraceInfo(
    ASTContext &ctx, const SmallPtrSetImpl<ModuleDecl *> &abiDependencies,
    const toolchain::DenseMap<StringRef, ModuleDecl *> &pathToModuleDecl,
    const DependencyTracker &depTracker, StringRef prebuiltCachePath,
    std::vector<CodiraModuleTraceInfo> &traceInfo) {
  using namespace toolchain::sys;

  auto computeAdjacentInterfacePath = [](SmallVectorImpl<char> &modPath) {
    auto languageInterfaceExt =
        file_types::getExtension(file_types::TY_CodiraModuleInterfaceFile);
    path::replace_extension(modPath, languageInterfaceExt);
  };

  SmallString<256> buffer;
  auto deps = depTracker.getDependencies();
  SmallVector<std::string, 16> dependencies{deps.begin(), deps.end()};
  auto incrDeps = depTracker.getIncrementalDependencyPaths();
  dependencies.append(incrDeps.begin(), incrDeps.end());
  // NOTE: macro dependencies are handled differently.
  // See 'computeCodiraMacroTraceInfo()'.
  for (const auto &depPath : dependencies) {

    // Decide if this is a languagemodule based on the extension of the raw
    // dependency path, as the true file may have a different one.
    // For example, this might happen when the canonicalized path points to
    // a Content Addressed Storage (CAS) location.
    auto moduleFileType =
        file_types::lookupTypeForExtension(path::extension(depPath));
    auto isCodiramodule = moduleFileType == file_types::TY_CodiraModuleFile;
    auto isCodirainterface =
        moduleFileType == file_types::TY_CodiraModuleInterfaceFile;

    if (!(isCodiramodule || isCodirainterface))
      continue;

    auto dep = pathToModuleDecl.find(depPath);
    if (dep != pathToModuleDecl.end()) {
      // Great, we recognize the path! Check if the file is still around.

      ModuleDecl *depMod = dep->second;
      if (depMod->isResilient() && !isCodirainterface) {
        // FIXME: Ideally, we would check that the languagemodule has a
        // languageinterface next to it. Tracked by rdar://problem/56351399.
      }

      // FIXME: Better error handling
      StringRef realDepPath =
          fs::real_path(depPath, buffer, /*expand_tile*/ true)
              ? StringRef(depPath) // Couldn't find the canonical path, assume
                                   // this is good enough.
              : buffer.str();

      bool isImportedDirectly = ::contains(abiDependencies, depMod);

      traceInfo.push_back(
          {/*Name=*/
           depMod->getName(),
           /*Path=*/
           realDepPath.str(),
           // TODO: There is an edge case which is not handled here.
           // When we build a framework using -import-underlying-module, or an
           // app/test using -import-objc-header, we should look at the direct
           // imports of the bridging modules, and mark those as our direct
           // imports.
           // TODO: Add negative test cases for the comment above.
           // TODO: Describe precise semantics of "isImportedDirectly".
           /*IsImportedDirectly=*/
           isImportedDirectly,
           /*SupportsLibraryEvolution=*/
           depMod->isResilient(),
           depMod->strictMemorySafety()});
      buffer.clear();

      continue;
    }

    // If the depTracker had an interface, that means that we must've
    // built a languagemodule from that interface, so we should have that
    // filename available.
    if (isCodirainterface) {
      // FIXME: Use PrettyStackTrace instead.
      toolchain::errs() << "WARNING: unexpected path for languageinterface file:\n"
                   << depPath << "\n"
                   << "The module <-> path mapping we have is:\n";
      for (auto &m : pathToModuleDecl)
        toolchain::errs() << m.second->getName() << " <-> " << m.first << '\n';
      continue;
    }

    // Skip cached modules in the prebuilt cache. We will add the corresponding
    // languageinterface from the SDK directly, but this isn't checked. :-/
    //
    // FIXME: This is incorrect if both paths are not relative w.r.t. to the
    // same root.
    if (StringRef(depPath).starts_with(prebuiltCachePath))
      continue;

    // If we have a languagemodule next to an interface, that interface path will
    // be saved (not checked), so don't save the path to this languagemodule.
    SmallString<256> moduleAdjacentInterfacePath(depPath);
    computeAdjacentInterfacePath(moduleAdjacentInterfacePath);
    if (::contains(pathToModuleDecl, moduleAdjacentInterfacePath))
      continue;

    // FIXME: The behavior of fs::exists for relative paths is undocumented.
    // Use something else instead?
    if (fs::exists(moduleAdjacentInterfacePath)) {
      // This should be an error but it is not because of funkiness around
      // compatible modules such as us having both armv7s.codeinterface
      // and armv7.codeinterface in the dependency tracker.
      continue;
    }
    buffer.clear();

    // We might land here when we have a arm.codemodule in the cache path
    // which added a dependency on a arm.codeinterface (which was not loaded).
  }

  // Almost a re-implementation of reversePathSortedFilenames :(.
  std::sort(traceInfo.begin(), traceInfo.end(),
            [](const CodiraModuleTraceInfo &m1,
               const CodiraModuleTraceInfo &m2) -> bool {
              return std::lexicographical_compare(
                  m1.Path.rbegin(), m1.Path.rend(), m2.Path.rbegin(),
                  m2.Path.rend());
            });
}

static void
computeCodiraMacroTraceInfo(ASTContext &ctx, const DependencyTracker &depTracker,
                           std::vector<CodiraMacroTraceInfo> &traceInfo) {
  for (const auto &macroDep : depTracker.getMacroPluginDependencies()) {
    traceInfo.push_back({macroDep.moduleName, macroDep.path});
  }

  // Again, almost a re-implementation of reversePathSortedFilenames :(.
  std::sort(
      traceInfo.begin(), traceInfo.end(),
      [](const CodiraMacroTraceInfo &m1, const CodiraMacroTraceInfo &m2) -> bool {
        return std::lexicographical_compare(m1.Path.rbegin(), m1.Path.rend(),
                                            m2.Path.rbegin(), m2.Path.rend());
      });
}

static void computeEnabledFeatures(ASTContext &ctx,
                                   std::vector<StringRef> &enabledFeatures) {
  struct FeatureAndName {
    Feature feature;
    StringRef name;
  };

  static const FeatureAndName features[] = {
#define FEATURE_ENTRY(FeatureName) {Feature::FeatureName, #FeatureName},
#define LANGUAGE_FEATURE(FeatureName, SENumber, Version)
#define EXPERIMENTAL_FEATURE(FeatureName, AvailableInProd)                     \
  FEATURE_ENTRY(FeatureName)
#define UPCOMING_FEATURE(FeatureName, SENumber, Version)                       \
  FEATURE_ENTRY(FeatureName)
#define OPTIONAL_LANGUAGE_FEATURE(FeatureName, SENumber, Version)              \
  FEATURE_ENTRY(FeatureName)
#include "language/Basic/Features.def"
  };

  for (auto &featureAndName : features) {
    if (ctx.LangOpts.hasFeature(featureAndName.feature))
      enabledFeatures.push_back(featureAndName.name);
  }

  // FIXME: It would be nice if the features were added in sorted order instead.
  // However, std::sort is not constexpr until C++20.
  std::sort(enabledFeatures.begin(), enabledFeatures.end(),
            [](const StringRef &lhs, const StringRef &rhs) -> bool {
              return lhs.compare(rhs) < 0;
            });
}

// [NOTE: Bailing-vs-crashing-in-trace-emission] There are certain edge cases
// in trace emission where an invariant that you think should hold does not hold
// in practice. For example, sometimes we have seen modules without any
// corresponding filename.
//
// Since the trace is a supplementary output for build system consumption, it
// it better to emit it on a best-effort basis instead of crashing and failing
// the build.
//
// Moreover, going forward, it would be nice if trace emission were more robust
// so we could emit the trace on a best-effort basis even if the dependency
// graph is ill-formed, so that the trace can be used as a debugging aid.
bool language::emitLoadedModuleTraceIfNeeded(ModuleDecl *mainModule,
                                          DependencyTracker *depTracker,
                                          const FrontendOptions &opts,
                                          const InputFile &input) {
  ASTContext &ctxt = mainModule->getASTContext();
  assert(!ctxt.hadError() &&
         "We should've already exited earlier if there was an error.");

  auto loadedModuleTracePath = input.getLoadedModuleTracePath();
  if (loadedModuleTracePath.empty())
    return false;

  SmallPtrSet<ModuleDecl *, 32> abiDependencies;
  {
    ABIDependencyEvaluator evaluator{};
    evaluator.getABIDependenciesForCodiraModule(mainModule, abiDependencies);
  }

  toolchain::DenseMap<StringRef, ModuleDecl *> pathToModuleDecl;
  for (const auto &module : ctxt.getLoadedModules()) {
    ModuleDecl *loadedDecl = module.second;
    if (!loadedDecl) {
      toolchain::errs() << "WARNING: Unable to load module '" << module.first
                   << ".\n";
      continue;
    }
    if (loadedDecl == mainModule)
      continue;
    if (loadedDecl->getModuleFilename().empty()) {
      // FIXME: rdar://problem/59853077
      // Ideally, this shouldn't happen. As a temporary workaround, avoid
      // crashing with a message while we investigate the problem.
      toolchain::errs() << "WARNING: Module '" << loadedDecl->getName().str()
                   << "' has an empty filename. This is probably an "
                   << "invariant violation.\n"
                   << "Please report it as a compiler bug.\n";
      continue;
    }
    pathToModuleDecl.insert(
        std::make_pair(loadedDecl->getModuleFilename(), loadedDecl));
  }

  std::vector<CodiraModuleTraceInfo> languageModules;
  computeCodiraModuleTraceInfo(ctxt, abiDependencies, pathToModuleDecl,
                              *depTracker, opts.PrebuiltModuleCachePath,
                              languageModules);

  std::vector<CodiraMacroTraceInfo> languageMacros;
  computeCodiraMacroTraceInfo(ctxt, *depTracker, languageMacros);

  std::vector<StringRef> enabledFeatures;
  computeEnabledFeatures(ctxt, enabledFeatures);

  LoadedModuleTraceFormat trace = {
      /*version=*/LoadedModuleTraceFormat::CurrentVersion,
      /*name=*/mainModule->getName(),
      /*arch=*/ctxt.LangOpts.Target.getArchName().str(),
      ctxt.LangOpts.EffectiveLanguageVersion.asAPINotesVersionString(),
      enabledFeatures,
      mainModule ? mainModule->strictMemorySafety() : false,
      languageModules,
      languageMacros};

  // raw_fd_ostream is unbuffered, and we may have multiple processes writing,
  // so first write to memory and then dump the buffer to the trace file.
  std::string stringBuffer;
  {
    toolchain::raw_string_ostream memoryBuffer(stringBuffer);
    json::Output jsonOutput(memoryBuffer, /*UserInfo=*/{},
                            /*PrettyPrint=*/false);
    json::jsonize(jsonOutput, trace, /*Required=*/true);
  }
  stringBuffer += "\n";

  // Write output via atomic append.
  toolchain::vfs::OutputConfig config;
  config.setAppend().setAtomicWrite();
  auto outputFile =
      ctxt.getOutputBackend().createFile(loadedModuleTracePath, config);

  if (!outputFile) {
    ctxt.Diags.diagnose(SourceLoc(), diag::error_opening_output,
                        loadedModuleTracePath,
                        toString(outputFile.takeError()));
    return true;
  }

  *outputFile << stringBuffer;

  if (auto err = outputFile->keep()) {
    ctxt.Diags.diagnose(SourceLoc(), diag::error_opening_output,
                        loadedModuleTracePath, toString(std::move(err)));
    return true;
  }
  return false;
}

class ObjcMethodReferenceCollector: public SourceEntityWalker {
  unsigned CurrentFileID;
  toolchain::DenseMap<const clang::ObjCMethodDecl*, unsigned> results;
  bool visitDeclReference(ValueDecl *D, SourceRange Range, TypeDecl *CtorTyRef,
                          ExtensionDecl *ExtTyRef, Type T,
                          ReferenceMetaData Data) override {
    if (!Range.isValid())
      return true;
    if (auto *clangD = dyn_cast_or_null<clang::ObjCMethodDecl>(D->getClangDecl()))
      Info.References[CurrentFileID].push_back(clangD);
    return true;
  }

  clang::ObjCMethodReferenceInfo Info;

public:
  ObjcMethodReferenceCollector(ModuleDecl *MD) {
    Info.ToolName = "language-compiler-version";
    Info.ToolVersion =
      getCodiraInterfaceCompilerVersionForCurrentCompiler(MD->getASTContext());
    auto &Opts = MD->getASTContext().LangOpts;
    Info.Target = Opts.Target.str();
    Info.TargetVariant = Opts.TargetVariant.has_value() ?
      Opts.TargetVariant->str() : "";
  }
  void setFileBeforeVisiting(SourceFile *SF) {
    assert(SF && "need to visit actual source files");
    Info.FilePaths.push_back(SF->getFilename().str());
    CurrentFileID = Info.FilePaths.size();
  }
  void serializeAsJson(toolchain::raw_ostream &OS) {
    clang::serializeObjCMethodReferencesAsJson(Info, OS);
  }
};

static void createFineModuleTraceFile(CompilerInstance &instance,
                                      const InputFile &input) {
  StringRef tracePath = input.getFineModuleTracePath();
  if (tracePath.empty()) {
    // we basically rely on the passing down of module trace file path
    // as an indicator that this job needs to emit an ObjC message trace file.
    // FIXME: add a separate language-frontend flag for ObjC message trace path
    // specifically.
    return;
  }
  ModuleDecl *MD = instance.getMainModule();
  auto &ctx = MD->getASTContext();
  // Write output via atomic append.
  toolchain::vfs::OutputConfig config;
  config.setAppend().setAtomicWrite();
  auto outputFile = ctx.getOutputBackend().createFile(tracePath, config);
  if (!outputFile) {
    ctx.Diags.diagnose(SourceLoc(), diag::error_opening_output, tracePath,
                       toString(outputFile.takeError()));
    return;
  }
  ObjcMethodReferenceCollector collector(MD);

  auto blocklisted = ctx.blockListConfig.hasBlockListAction(MD->getNameStr(),
    BlockListKeyKind::ModuleName, BlockListAction::SkipEmittingFineModuleTrace);

  if (!blocklisted) {
    instance.forEachFileToTypeCheck([&](SourceFile& SF) {
      collector.setFileBeforeVisiting(&SF);
      collector.walk(SF);
      return false;
    });
  }

  // print this json line.
  std::string stringBuffer;
  {
    toolchain::raw_string_ostream memoryBuffer(stringBuffer);
    collector.serializeAsJson(memoryBuffer);
  }
  stringBuffer += "\n";

  // Write output via atomic append.
  *outputFile << stringBuffer;
  if (auto err = outputFile->keep()) {
    ctx.Diags.diagnose(SourceLoc(), diag::error_opening_output,
                       tracePath, toString(std::move(err)));
    return;
  }
}

bool language::emitFineModuleTraceIfNeeded(CompilerInstance &Instance,
                                        const FrontendOptions &opts) {
  ModuleDecl *mainModule = Instance.getMainModule();
  ASTContext &ctxt = mainModule->getASTContext();
  assert(!ctxt.hadError() &&
         "We should've already exited earlier if there was an error.");

  opts.InputsAndOutputs.forEachInput([&](const InputFile &input) {
    createFineModuleTraceFile(Instance, input);
    return true;
  });
  return false;
}
