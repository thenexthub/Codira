//===--- ParseVersion.h - Parser Swift Version Numbers ----------*- C++ -*-===//
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

#ifndef SWIFT_PARSE_PARSEVERSION_H
#define SWIFT_PARSE_PARSEVERSION_H

#include "language/Basic/Version.h"

namespace language {
class DiagnosticEngine;

namespace version {
/// Returns a version from the currently defined SWIFT_COMPILER_VERSION.
///
/// If SWIFT_COMPILER_VERSION is undefined, this will return the empty
/// compiler version.
Version getCurrentCompilerVersion();
} // namespace version

class VersionParser final {
public:
  /// Parse a version in the form used by the _compiler_version(string-literal)
  /// \#if condition.
  ///
  /// \note This is \em only used for the string literal version, so it includes
  /// backwards-compatibility logic to convert it to something that can be
  /// compared with a modern SWIFT_COMPILER_VERSION.
  static std::optional<version::Version>
  parseCompilerVersionString(StringRef VersionString, SourceLoc Loc,
                             DiagnosticEngine *Diags);

  /// Parse a generic version string of the format [0-9]+(.[0-9]+)*
  ///
  /// Version components can be any unsigned 64-bit number.
  static std::optional<version::Version>
  parseVersionString(StringRef VersionString, SourceLoc Loc,
                     DiagnosticEngine *Diags);
};
} // namespace language

#endif // SWIFT_PARSE_PARSEVERSION_H
