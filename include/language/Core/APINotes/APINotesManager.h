//===--- APINotesManager.h - Manage API Notes Files -------------*- C++ -*-===//
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

#ifndef LANGUAGE_CORE_APINOTES_APINOTESMANAGER_H
#define LANGUAGE_CORE_APINOTES_APINOTESMANAGER_H

#include "language/Core/Basic/SourceLocation.h"
#include "toolchain/ADT/ArrayRef.h"
#include "toolchain/ADT/DenseMap.h"
#include "toolchain/ADT/PointerUnion.h"
#include "toolchain/ADT/StringRef.h"
#include "toolchain/Support/VersionTuple.h"
#include <memory>
#include <string>

namespace language::Core {

class DirectoryEntry;
class FileEntry;
class LangOptions;
class Module;
class SourceManager;

namespace api_notes {

class APINotesReader;

/// The API notes manager helps find API notes associated with declarations.
///
/// API notes are externally-provided annotations for declarations that can
/// introduce new attributes (covering availability, nullability of
/// parameters/results, and so on) for specific declarations without directly
/// modifying the headers that contain those declarations.
///
/// The API notes manager is responsible for finding and loading the
/// external API notes files that correspond to a given header. Its primary
/// operation is \c findAPINotes(), which finds the API notes reader that
/// provides information about the declarations at that location.
class APINotesManager {
  using ReaderEntry = toolchain::PointerUnion<DirectoryEntryRef, APINotesReader *>;

  SourceManager &SM;

  /// Whether to implicitly search for API notes files based on the
  /// source file from which an entity was declared.
  bool ImplicitAPINotes;

  /// Whether to apply all APINotes as optionally-applied versioned
  /// entities. This means that when building a Clang module,
  /// we capture every note on a given decl wrapped in a SwiftVersionedAttr
  /// (with an empty version field for unversioned notes), and have the
  /// client apply the relevant version's notes.
  bool VersionIndependentSwift;

  /// The Swift version to use when interpreting versioned API notes.
  toolchain::VersionTuple SwiftVersion;

  enum ReaderKind : unsigned { Public = 0, Private = 1 };

  /// API notes readers for the current module.
  ///
  /// There can be up to two of these, one for public headers and one
  /// for private headers.
  ///
  /// Not using std::unique_ptr to store these, since the reader pointers are
  /// also stored in toolchain::PointerUnion below.
  APINotesReader *CurrentModuleReaders[2] = {nullptr, nullptr};

  /// A mapping from header file directories to the API notes reader for
  /// that directory, or a redirection to another directory entry that may
  /// have more information, or NULL to indicate that there is no API notes
  /// reader for this directory.
  toolchain::DenseMap<const DirectoryEntry *, ReaderEntry> Readers;

  /// Load the API notes associated with the given file, whether it is
  /// the binary or source form of API notes.
  ///
  /// \returns the API notes reader for this file, or null if there is
  /// a failure.
  std::unique_ptr<APINotesReader> loadAPINotes(FileEntryRef APINotesFile);

  /// Load the API notes associated with the given buffer, whether it is
  /// the binary or source form of API notes.
  ///
  /// \returns the API notes reader for this file, or null if there is
  /// a failure.
  std::unique_ptr<APINotesReader> loadAPINotes(StringRef Buffer);

  /// Load the given API notes file for the given header directory.
  ///
  /// \param HeaderDir The directory at which we
  ///
  /// \returns true if an error occurred.
  bool loadAPINotes(const DirectoryEntry *HeaderDir, FileEntryRef APINotesFile);

  /// Look for API notes in the given directory.
  ///
  /// This might find either a binary or source API notes.
  OptionalFileEntryRef findAPINotesFile(DirectoryEntryRef Directory,
                                        StringRef FileName,
                                        bool WantPublic = true);

  /// Attempt to load API notes for the given framework. A framework will have
  /// the API notes file under either {FrameworkPath}/APINotes,
  /// {FrameworkPath}/Headers or {FrameworkPath}/PrivateHeaders, while a
  /// library will have the API notes simply in its directory.
  ///
  /// \param FrameworkPath The path to the framework.
  /// \param Public Whether to load the public API notes. Otherwise, attempt
  /// to load the private API notes.
  ///
  /// \returns the header directory entry (e.g., for Headers or PrivateHeaders)
  /// for which the API notes were successfully loaded, or NULL if API notes
  /// could not be loaded for any reason.
  OptionalDirectoryEntryRef loadFrameworkAPINotes(toolchain::StringRef FrameworkPath,
                                                  toolchain::StringRef FrameworkName,
                                                  bool Public);

public:
  APINotesManager(SourceManager &SM, const LangOptions &LangOpts);
  ~APINotesManager();

  /// Set the Swift version to use when filtering API notes.
  void setSwiftVersion(toolchain::VersionTuple Version) {
    this->SwiftVersion = Version;
  }

  /// Load the API notes for the current module.
  ///
  /// \param M The current module.
  /// \param LookInModule Whether to look inside the module itself.
  /// \param SearchPaths The paths in which we should search for API notes
  /// for the current module.
  ///
  /// \returns true if API notes were successfully loaded, \c false otherwise.
  bool loadCurrentModuleAPINotes(Module *M, bool LookInModule,
                                 ArrayRef<std::string> SearchPaths);

  /// Get FileEntry for the APINotes of the module that is currently being
  /// compiled.
  ///
  /// \param M The current module.
  /// \param LookInModule Whether to look inside the directory of the current
  /// module.
  /// \param SearchPaths The paths in which we should search for API
  /// notes for the current module.
  ///
  /// \returns a vector of FileEntry where APINotes files are.
  toolchain::SmallVector<FileEntryRef, 2>
  getCurrentModuleAPINotes(Module *M, bool LookInModule,
                           ArrayRef<std::string> SearchPaths);

  /// Load Compiled API notes for current module.
  ///
  /// \param Buffers Array of compiled API notes.
  ///
  /// \returns true if API notes were successfully loaded, \c false otherwise.
  bool loadCurrentModuleAPINotesFromBuffer(ArrayRef<StringRef> Buffers);

  /// Retrieve the set of API notes readers for the current module.
  ArrayRef<APINotesReader *> getCurrentModuleReaders() const {
    bool HasPublic = CurrentModuleReaders[ReaderKind::Public];
    bool HasPrivate = CurrentModuleReaders[ReaderKind::Private];
    assert((!HasPrivate || HasPublic) && "private module requires public module");
    if (!HasPrivate && !HasPublic)
      return {};
    return ArrayRef(CurrentModuleReaders).slice(0, HasPrivate ? 2 : 1);
  }

  /// Find the API notes readers that correspond to the given source location.
  toolchain::SmallVector<APINotesReader *, 2> findAPINotes(SourceLocation Loc);

  bool captureVersionIndependentSwift() { return VersionIndependentSwift; }
};

} // end namespace api_notes
} // end namespace language::Core

#endif
