//===--- DependencyChecking.cpp -------------------------------------------===//
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

#include "DependencyChecking.h"

#include "language/AST/SourceFile.h"
#include "language/Frontend/Frontend.h"

using namespace language;

namespace {
/// For each dependency file in \p CI, run \p callback until the callback
/// returns \c true. Returns \c true if any callback call returns \c true, \c
/// false otherwise.
static bool
forEachDependencyUntilTrue(CompilerInstance &CI,
                           std::optional<unsigned> excludeBufferID,
                           toolchain::function_ref<bool(StringRef)> callback) {
  // Check files in the current module. If 'excludeBufferID' is None, exclude
  // all source files.
  if (excludeBufferID.has_value()) {
    for (FileUnit *file : CI.getMainModule()->getFiles()) {
      StringRef filename;
      if (auto SF = dyn_cast<SourceFile>(file)) {
        if (SF->getBufferID() == excludeBufferID)
          continue;
        filename = SF->getFilename();
      } else if (auto LF = dyn_cast<LoadedFile>(file))
        filename = LF->getFilename();
      else
        continue;

      // Ignore synthesized files.
      if (filename.empty() || filename.front() == '<')
        continue;

      if (callback(filename))
        return true;
    }
  }

  // Check other non-system depenencies (e.g. modules, headers).
  for (auto &dep : CI.getDependencyTracker()->getDependencies()) {
    if (callback(dep))
      return true;
  }
  for (auto dep : CI.getDependencyTracker()->getIncrementalDependencyPaths()) {
    if (callback(dep))
      return true;
  }
  for (auto dep : CI.getDependencyTracker()->getMacroPluginDependencyPaths()) {
    if (callback(dep))
      return true;
  }

  return false;
}

/// Collect hash codes of the dependencies into \c Map.
static void cacheDependencyHashIfNeeded(CompilerInstance &CI,
                                        std::optional<unsigned> excludeBufferID,
                                        toolchain::StringMap<toolchain::hash_code> &Map) {
  auto &FS = CI.getFileSystem();
  forEachDependencyUntilTrue(CI, excludeBufferID, [&](StringRef filename) {
    if (Map.count(filename))
      return false;

    auto stat = FS.status(filename);
    if (!stat)
      return false;

    // We will check the hash only if the modification time of the dependecy
    // is zero. See 'areAnyDependentFilesInvalidated() below'.
    if (stat->getLastModificationTime() != toolchain::sys::TimePoint<>())
      return false;

    auto buf = FS.getBufferForFile(filename);
    Map[filename] = toolchain::hash_value(buf.get()->getBuffer());
    return false;
  });
}

/// Check if any dependent files are modified since \p timestamp.
static bool
areAnyDependentFilesInvalidated(CompilerInstance &CI, toolchain::vfs::FileSystem &FS,
                                std::optional<unsigned> excludeBufferID,
                                toolchain::sys::TimePoint<> timestamp,
                                const toolchain::StringMap<toolchain::hash_code> &Map) {

  return forEachDependencyUntilTrue(
      CI, excludeBufferID, [&](StringRef filePath) {
        auto stat = FS.status(filePath);
        if (!stat)
          // Missing.
          return true;

        auto lastModTime = stat->getLastModificationTime();
        if (lastModTime > timestamp)
          // Modified.
          return true;

        // If the last modification time is zero, this file is probably from a
        // virtual file system. We need to check the content.
        if (lastModTime == toolchain::sys::TimePoint<>()) {
          // Get the hash code of the last content.
          auto oldHashEntry = Map.find(filePath);
          if (oldHashEntry == Map.end())
            // Unreachable? Not virtual in old filesystem, but virtual in new
            // one.
            return true;
          auto oldHash = oldHashEntry->second;

          // Calculate the hash code of the current content.
          auto newContent = FS.getBufferForFile(filePath);
          if (!newContent)
            // Unreachable? stat succeeded, but couldn't get the content.
            return true;

          auto newHash = toolchain::hash_value(newContent.get()->getBuffer());

          if (oldHash != newHash)
            return true;
        }

        return false;
      });
}

} // namespace

void ide::cacheDependencyHashIfNeeded(CompilerInstance &CI,
                                      std::optional<unsigned> excludeBufferID,
                                      toolchain::StringMap<toolchain::hash_code> &Map) {
  return ::cacheDependencyHashIfNeeded(CI, excludeBufferID, Map);
}

bool ide::areAnyDependentFilesInvalidated(
    CompilerInstance &CI, toolchain::vfs::FileSystem &FS,
    std::optional<unsigned> excludeBufferID, toolchain::sys::TimePoint<> timestamp,
    const toolchain::StringMap<toolchain::hash_code> &Map) {
  return ::areAnyDependentFilesInvalidated(CI, FS, excludeBufferID, timestamp,
                                           Map);
}
