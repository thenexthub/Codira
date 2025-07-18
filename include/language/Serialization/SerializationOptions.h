//===--- SerializationOptions.h - Control languagemodule emission --*- C++ -*-===//
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

#ifndef LANGUAGE_SERIALIZATION_SERIALIZATIONOPTIONS_H
#define LANGUAGE_SERIALIZATION_SERIALIZATIONOPTIONS_H

#include "language/AST/SearchPathOptions.h"
#include "language/Basic/Toolchain.h"
#include "language/Basic/PathRemapper.h"
#include "toolchain/Support/VersionTuple.h"

#include <set>
#include <string>
#include <tuple>
#include <vector>

namespace language {

class SerializationOptions {
public:
  SerializationOptions() = default;
  SerializationOptions(SerializationOptions &&) = default;
  SerializationOptions &operator=(SerializationOptions &&) = default;
  SerializationOptions(const SerializationOptions &) = default;
  SerializationOptions &operator=(const SerializationOptions &) = default;
  ~SerializationOptions() = default;

  StringRef OutputPath;
  StringRef DocOutputPath;
  StringRef SourceInfoOutputPath;
  std::string ABIDescriptorPath;
  bool emptyABIDescriptor = false;
  toolchain::VersionTuple UserModuleVersion;
  std::set<std::string> AllowableClients;
  std::string SDKName;
  std::string SDKVersion;

  StringRef GroupInfoPath;
  StringRef ImportedHeader;
  StringRef ImportedPCHPath;
  bool SerializeBridgingHeader = false;
  bool SerializeEmptyBridgingHeader = false;
  StringRef ModuleLinkName;
  StringRef ModuleInterface;
  std::vector<std::string> ExtraClangOptions;
  std::vector<language::PluginSearchOption> PluginSearchOptions;

  /// Path prefixes that should be rewritten in debug info.
  PathRemapper DebuggingOptionsPrefixMap;

  /// Obfuscate the serialized paths so we don't have the actual paths encoded
  /// in the .codemodule file.
  PathObfuscator PathObfuscator;

  /// Describes a single-file dependency for this module, along with the
  /// appropriate strategy for how to verify if it's up-to-date.
  class FileDependency {
    /// The size of the file on disk, in bytes.
    uint64_t Size : 62;

    /// A dependency can be either hash-based or modification-time-based.
    bool IsHashBased : 1;

    /// The dependency path can be absolute or relative to the SDK
    bool IsSDKRelative : 1;

    union {
      /// The last modification time of the file.
      uint64_t ModificationTime;

      /// The xxHash of the full contents of the file.
      uint64_t ContentHash;
    };

    /// The path to the dependency.
    std::string Path;

    FileDependency(uint64_t size, bool isHash, uint64_t hashOrModTime,
                   StringRef path, bool isSDKRelative)
        : Size(size), IsHashBased(isHash), IsSDKRelative(isSDKRelative),
          ModificationTime(hashOrModTime), Path(path) {}

  public:
    FileDependency() = delete;

    /// Creates a new hash-based file dependency.
    static FileDependency hashBased(StringRef path, bool isSDKRelative,
                                    uint64_t size, uint64_t hash) {
      return FileDependency(size, /*isHash*/ true, hash, path, isSDKRelative);
    }

    /// Creates a new modification time-based file dependency.
    static FileDependency modTimeBased(StringRef path, bool isSDKRelative,
                                       uint64_t size, uint64_t mtime) {
      return FileDependency(size, /*isHash*/ false, mtime, path, isSDKRelative);
    }

    /// Updates the last-modified time of this dependency.
    /// If the dependency is a hash-based dependency, it becomes
    /// modification time-based.
    void setLastModificationTime(uint64_t mtime) {
      IsHashBased = false;
      ModificationTime = mtime;
    }

    /// Updates the content hash of this dependency.
    /// If the dependency is a modification time-based dependency, it becomes
    /// hash-based.
    void setContentHash(uint64_t hash) {
      IsHashBased = true;
      ContentHash = hash;
    }

    /// Determines if this dependency is hash-based and should be validated
    /// based on content hash.
    bool isHashBased() const { return IsHashBased; }

    /// Determines if this dependency is absolute or relative to the SDK.
    bool isSDKRelative() const { return IsSDKRelative; }

    /// Determines if this dependency is hash-based and should be validated
    /// based on modification time.
    bool isModificationTimeBased() const { return !IsHashBased; }

    /// Gets the modification time, if this is a modification time-based
    /// dependency.
    uint64_t getModificationTime() const {
      assert(isModificationTimeBased() &&
             "cannot get modification time for hash-based dependency");
      return ModificationTime;
    }

    /// Gets the content hash, if this is a hash-based
    /// dependency.
    uint64_t getContentHash() const {
      assert(isHashBased() &&
             "cannot get content hash for mtime-based dependency");
      return ContentHash;
    }

    StringRef getPath() const { return Path; }
    uint64_t getSize() const { return Size; }
  };
  ArrayRef<FileDependency> Dependencies;
  ArrayRef<std::tuple<std::string, bool>> PublicDependentLibraries;

  bool AutolinkForceLoad = false;
  bool SerializeAllSIL = false;
  bool SerializeDebugInfoSIL = false;
  bool SerializeOptionsForDebugging = false;
  bool IsSIB = false;
  bool DisableCrossModuleIncrementalInfo = false;
  bool StaticLibrary = false;
  bool HermeticSealAtLink = false;
  bool EmbeddedCodiraModule = false;
  bool IsOSSA = false;
  bool SkipNonExportableDecls = false;
  bool ExplicitModuleBuild = false;
  bool EnableSerializationRemarks = false;
  bool IsInterfaceSDKRelative = false;
};

} // end namespace language
#endif
