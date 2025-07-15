//===--- PathRemapper.h - Transforms path prefixes --------------*- C++ -*-===//
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
//  This file defines a data structure that stores a string-to-string
//  mapping used to transform file paths based on a prefix mapping. It
//  is optimized for the common case, which is that there will be
//  extremely few mappings (i.e., one or two).
//
//  Remappings are stored such that they are applied in the order they
//  are passed on the command line. This would only matter if one
//  source mapping was a prefix of another.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_BASIC_PATHREMAPPER_H
#define LANGUAGE_BASIC_PATHREMAPPER_H

#include "language/Basic/Toolchain.h"
#include "clang/Basic/PathRemapper.h"
#include "toolchain/ADT/SmallVector.h"
#include "toolchain/ADT/Twine.h"

#include <string>
#include <utility>

namespace language {

class PathRemapper {
  SmallVector<std::pair<std::string, std::string>, 2> PathMappings;
  friend class PathObfuscator;
public:
  /// Adds a mapping such that any paths starting with `FromPrefix` have that
  /// portion replaced with `ToPrefix`.
  void addMapping(StringRef FromPrefix, StringRef ToPrefix) {
    PathMappings.emplace_back(FromPrefix.str(), ToPrefix.str());
  }

  /// Returns a remapped `Path` if it starts with a prefix in the map; otherwise
  /// the original path is returned.
  std::string remapPath(StringRef Path) const {
    // Clang's implementation of this feature also compares the path string
    // directly instead of treating path segments as indivisible units. The
    // latter would arguably be more accurate, but we choose to preserve
    // compatibility with Clang (especially because we propagate the flag to
    // ClangImporter as well).
    for (const auto &Mapping : PathMappings)
      if (Path.starts_with(Mapping.first))
        return (Twine(Mapping.second) +
                Path.substr(Mapping.first.size())).str();
    return Path.str();
  }

  /// Returns the Clang PathRemapper equivalent, suitable for use with Clang
  /// APIs.
  clang::PathRemapper asClangPathRemapper() const {
    clang::PathRemapper Remapper;
    for (const auto &Mapping : PathMappings)
      Remapper.addMapping(Mapping.first, Mapping.second);
    return Remapper;
  }
};

class PathObfuscator {
  PathRemapper obfuscator, recoverer;
public:
  void addMapping(StringRef FromPrefix, StringRef ToPrefix) {
    obfuscator.addMapping(FromPrefix, ToPrefix);
    recoverer.addMapping(ToPrefix, FromPrefix);
  }
  std::string obfuscate(StringRef Path) const {
    return obfuscator.remapPath(Path);
  }
  std::string recover(StringRef Path) const {
    return recoverer.remapPath(Path);
  }
  void forEachPair(toolchain::function_ref<void(StringRef, StringRef)> op) const {
    for (auto pair: obfuscator.PathMappings) {
      op(pair.first, pair.second);
    }
  }
};

} // end namespace language

#endif // LANGUAGE_BASIC_PATHREMAPPER_H
