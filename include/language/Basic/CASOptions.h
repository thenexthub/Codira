//===--- CASOptions.h - CAS & caching options -------------------*- C++ -*-===//
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
//  This file defines the CASOptions class, which provides various
//  CAS and caching flags.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_BASIC_CASOPTIONS_H
#define LANGUAGE_BASIC_CASOPTIONS_H

#include "clang/CAS/CASOptions.h"
#include "toolchain/ADT/Hashing.h"

namespace language {

class CASOptions final {
public:
  /// Enable compiler caching.
  bool EnableCaching = false;

  /// Enable compiler caching remarks.
  bool EnableCachingRemarks = false;

  /// Skip replaying outputs from cache.
  bool CacheSkipReplay = false;

  /// Import modules from CAS.
  bool ImportModuleFromCAS = false;

  /// CASOptions
  clang::CASOptions CASOpts;

  /// Clang Include Trees.
  std::string ClangIncludeTree;

  /// Clang Include Tree FileList.
  std::string ClangIncludeTreeFileList;

  /// CacheKey for input file.
  std::string InputFileKey;

  /// Cache key for imported bridging header.
  std::string BridgingHeaderPCHCacheKey;

  /// Has immutable file system input.
  bool HasImmutableFileSystem = false;

  /// Get the CAS configuration flags.
  void enumerateCASConfigurationFlags(
      toolchain::function_ref<void(toolchain::StringRef)> Callback) const;

  /// Check to see if a CASFileSystem is required.
  bool requireCASFS() const {
    return EnableCaching &&
           (!ClangIncludeTree.empty() || !ClangIncludeTreeFileList.empty() ||
            !InputFileKey.empty() || !BridgingHeaderPCHCacheKey.empty());
  }

  /// Return a hash code of any components from these options that should
  /// contribute to a Codira Bridging PCH hash.
  toolchain::hash_code getPCHHashComponents() const {
    // The CASIDs are generated from scanner, thus not part of the hash since
    // they will always be empty when requested.
    // TODO: Add frozen clang::CASOptions to the hash.
    return toolchain::hash_combine(EnableCaching);
  }

  /// Return a hash code of any components from these options that should
  /// contribute to a Codira Dependency Scanning hash.
  toolchain::hash_code getModuleScanningHashComponents() const {
    return getPCHHashComponents();
  }
};

} // namespace language

#endif // LANGUAGE_BASIC_CASOPTIONS_H
