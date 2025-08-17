//===--- GlobalModuleIndex.h - Global Module Index --------------*- C++ -*-===//
//
// Copyright (c) 2025, NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
// 
// Author: Tunjay Akbarli
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
// 
//     http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// 
// Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
// Middletown, DE 19709, New Castle County, USA.
//
//===----------------------------------------------------------------------===//
//
// This file defines the GlobalModuleIndex class, which manages a global index
// containing all of the identifiers known to the various modules within a given
// subdirectory of the module cache. It is used to improve the performance of
// queries such as "do any modules know about this identifier?"
//
//===----------------------------------------------------------------------===//
#ifndef LANGUAGE_CORE_SERIALIZATION_GLOBALMODULEINDEX_H
#define LANGUAGE_CORE_SERIALIZATION_GLOBALMODULEINDEX_H

#include "toolchain/ADT/DenseMap.h"
#include "toolchain/ADT/SmallPtrSet.h"
#include "toolchain/ADT/SmallVector.h"
#include "toolchain/ADT/StringMap.h"
#include "toolchain/ADT/StringRef.h"
#include "toolchain/Support/Error.h"
#include <memory>
#include <utility>

namespace toolchain {
class BitstreamCursor;
class MemoryBuffer;
}

namespace language::Core {

class FileManager;
class IdentifierIterator;
class PCHContainerOperations;
class PCHContainerReader;

namespace serialization {
  class ModuleFile;
}

/// A global index for a set of module files, providing information about
/// the identifiers within those module files.
///
/// The global index is an aid for name lookup into modules, offering a central
/// place where one can look for identifiers determine which
/// module files contain any information about that identifier. This
/// allows the client to restrict the search to only those module files known
/// to have a information about that identifier, improving performance. Moreover,
/// the global module index may know about module files that have not been
/// imported, and can be queried to determine which modules the current
/// translation could or should load to fix a problem.
class GlobalModuleIndex {
  using ModuleFile = serialization::ModuleFile;

  /// Buffer containing the index file, which is lazily accessed so long
  /// as the global module index is live.
  std::unique_ptr<toolchain::MemoryBuffer> Buffer;

  /// The hash table.
  ///
  /// This pointer actually points to a IdentifierIndexTable object,
  /// but that type is only accessible within the implementation of
  /// GlobalModuleIndex.
  void *IdentifierIndex;

  /// Information about a given module file.
  struct ModuleInfo {
    ModuleInfo() = default;

    /// The module file, once it has been resolved.
    ModuleFile *File = nullptr;

    /// The module file name.
    std::string FileName;

    /// Size of the module file at the time the global index was built.
    off_t Size = 0;

    /// Modification time of the module file at the time the global
    /// index was built.
    time_t ModTime = 0;

    /// The module IDs on which this module directly depends.
    /// FIXME: We don't really need a vector here.
    toolchain::SmallVector<unsigned, 4> Dependencies;
  };

  /// A mapping from module IDs to information about each module.
  ///
  /// This vector may have gaps, if module files have been removed or have
  /// been updated since the index was built. A gap is indicated by an empty
  /// file name.
  toolchain::SmallVector<ModuleInfo, 16> Modules;

  /// Lazily-populated mapping from module files to their
  /// corresponding index into the \c Modules vector.
  toolchain::DenseMap<ModuleFile *, unsigned> ModulesByFile;

  /// The set of modules that have not yet been resolved.
  ///
  /// The string is just the name of the module itself, which maps to the
  /// module ID.
  toolchain::StringMap<unsigned> UnresolvedModules;

  /// The number of identifier lookups we performed.
  unsigned NumIdentifierLookups;

  /// The number of identifier lookup hits, where we recognize the
  /// identifier.
  unsigned NumIdentifierLookupHits;

  /// Internal constructor. Use \c readIndex() to read an index.
  explicit GlobalModuleIndex(std::unique_ptr<toolchain::MemoryBuffer> Buffer,
                             toolchain::BitstreamCursor Cursor);

  GlobalModuleIndex(const GlobalModuleIndex &) = delete;
  GlobalModuleIndex &operator=(const GlobalModuleIndex &) = delete;

public:
  ~GlobalModuleIndex();

  /// Read a global index file for the given directory.
  ///
  /// \param Path The path to the specific module cache where the module files
  /// for the intended configuration reside.
  ///
  /// \returns A pair containing the global module index (if it exists) and
  /// the error.
  static std::pair<GlobalModuleIndex *, toolchain::Error>
  readIndex(toolchain::StringRef Path);

  /// Returns an iterator for identifiers stored in the index table.
  ///
  /// The caller accepts ownership of the returned object.
  IdentifierIterator *createIdentifierIterator() const;

  /// Retrieve the set of module files on which the given module file
  /// directly depends.
  void getModuleDependencies(ModuleFile *File,
                             toolchain::SmallVectorImpl<ModuleFile *> &Dependencies);

  /// A set of module files in which we found a result.
  typedef toolchain::SmallPtrSet<ModuleFile *, 4> HitSet;

  /// Look for all of the module files with information about the given
  /// identifier, e.g., a global function, variable, or type with that name.
  ///
  /// \param Name The identifier to look for.
  ///
  /// \param Hits Will be populated with the set of module files that have
  /// information about this name.
  ///
  /// \returns true if the identifier is known to the index, false otherwise.
  bool lookupIdentifier(toolchain::StringRef Name, HitSet &Hits);

  /// Note that the given module file has been loaded.
  ///
  /// \returns false if the global module index has information about this
  /// module file, and true otherwise.
  bool loadedModuleFile(ModuleFile *File);

  /// Print statistics to standard error.
  void printStats();

  /// Print debugging view to standard error.
  void dump();

  /// Write a global index into the given
  ///
  /// \param FileMgr The file manager to use to load module files.
  /// \param PCHContainerRdr - The PCHContainerOperations to use for loading and
  /// creating modules.
  /// \param Path The path to the directory containing module files, into
  /// which the global index will be written.
  static toolchain::Error writeIndex(FileManager &FileMgr,
                                const PCHContainerReader &PCHContainerRdr,
                                toolchain::StringRef Path);
};
}

#endif
