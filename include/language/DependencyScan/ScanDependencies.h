//===--- ScanDependencies.h -- Scans the dependencies of a module ------===//
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

#ifndef LANGUAGE_DEPENDENCY_SCANDEPENDENCIES_H
#define LANGUAGE_DEPENDENCY_SCANDEPENDENCIES_H

#include "language-c/DependencyScan/DependencyScan.h"
#include "language/AST/DiagnosticEngine.h"
#include "toolchain/ADT/StringMap.h"
#include "toolchain/Support/Chrono.h"
#include "toolchain/Support/ErrorOr.h"
#include <unordered_set>

namespace toolchain {
class StringSaver;
namespace cas {
class ObjectStore;
} // namespace cas
namespace vfs {
class FileSystem;
} // namespace vfs
} // namespace toolchain

namespace language {

class CompilerInvocation;
class CompilerInstance;
class DiagnosticEngine;
class ModuleDependenciesCache;
struct ModuleDependencyID;
struct ModuleDependencyIDHash;
using ModuleDependencyIDSet =
    std::unordered_set<ModuleDependencyID, ModuleDependencyIDHash>;
class CodiraDependencyScanningService;

namespace dependencies {
class DependencyScanDiagnosticCollector;

using CompilerArgInstanceCacheMap =
    toolchain::StringMap<std::tuple<std::unique_ptr<CompilerInstance>,
                               std::unique_ptr<CodiraDependencyScanningService>,
                               std::unique_ptr<ModuleDependenciesCache>>>;

// MARK: FrontendTool dependency scanner entry points
/// Scans the dependencies of the main module of \c instance and writes out
/// the resulting JSON according to the instance's output parameters.
bool scanDependencies(CompilerInstance &instance);

/// Identify all imports in the translation unit's module.
bool prescanDependencies(CompilerInstance &instance);

// MARK: Dependency scanning execution
/// Scans the dependencies of the main module of \c instance.
toolchain::ErrorOr<languagescan_dependency_graph_t>
performModuleScan(CodiraDependencyScanningService &service,
                  CompilerInstance &instance,
                  ModuleDependenciesCache &cache,
                  DependencyScanDiagnosticCollector *diagnostics = nullptr);

/// Scans the main module of \c instance for all direct module imports
toolchain::ErrorOr<languagescan_import_set_t>
performModulePrescan(CodiraDependencyScanningService &service,
                     CompilerInstance &instance,
                     ModuleDependenciesCache &cache,
                     DependencyScanDiagnosticCollector *diagnostics = nullptr);

namespace incremental {
/// For the given module dependency graph captured in the 'cache',
/// validate whether each dependency node is up-to-date w.r.t. serialization
/// time-stamp. i.e. if any of the input files of a module dependency are newer
/// than the serialized dependency graph, it is considered invalidated and must
/// be re-scanned.
void validateInterModuleDependenciesCache(
    const ModuleDependencyID &rootModuleID, ModuleDependenciesCache &cache,
    std::shared_ptr<toolchain::cas::ObjectStore> cas,
    const toolchain::sys::TimePoint<> &cacheTimeStamp, toolchain::vfs::FileSystem &fs,
    DiagnosticEngine &diags, bool emitRemarks = false);

/// Perform a postorder DFS to locate modules whose build recipe is out-of-date
/// with respect to their inputs. Upon encountering such a module, add it to the
/// set of invalidated modules, along with the path from the root to this
/// module.
void outOfDateModuleScan(const ModuleDependencyID &sourceModuleID,
                         const ModuleDependenciesCache &cache,
                         std::shared_ptr<toolchain::cas::ObjectStore> cas,
                         const toolchain::sys::TimePoint<> &cacheTimeStamp,
                         toolchain::vfs::FileSystem &fs, DiagnosticEngine &diags,
                         bool emitRemarks, ModuleDependencyIDSet &visited,
                         ModuleDependencyIDSet &modulesRequiringRescan);

/// Validate whether all inputs of a given module dependency
/// are older than the cache serialization time.
bool verifyModuleDependencyUpToDate(
    const ModuleDependencyID &moduleID, const ModuleDependenciesCache &cache,
    std::shared_ptr<toolchain::cas::ObjectStore> cas,
    const toolchain::sys::TimePoint<> &cacheTimeStamp, toolchain::vfs::FileSystem &fs,
    DiagnosticEngine &diags, bool emitRemarks);
} // end namespace incremental
} // end namespace dependencies
} // end namespace language

#endif
