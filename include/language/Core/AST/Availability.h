//===--- Availability.h - Classes for availability --------------*- C++ -*-===//
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
// This files defines some classes that implement availability checking.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_AST_AVAILABILITY_H
#define LANGUAGE_CORE_AST_AVAILABILITY_H

#include "language/Core/Basic/SourceLocation.h"
#include "toolchain/ADT/SmallString.h"
#include "toolchain/ADT/StringRef.h"
#include "toolchain/Support/VersionTuple.h"

namespace language::Core {

/// One specifier in an @available expression.
///
/// \code
///   @available(macos 10.10, *)
/// \endcode
///
/// Here, 'macos 10.10' and '*' both map to an instance of this type.
///
class AvailabilitySpec {
  /// Represents the version that this specifier requires. If the host OS
  /// version is greater than or equal to Version, the @available will evaluate
  /// to true.
  VersionTuple Version;

  /// Name of the platform that Version corresponds to.
  StringRef Platform;

  SourceLocation BeginLoc, EndLoc;

public:
  AvailabilitySpec(VersionTuple Version, StringRef Platform,
                   SourceLocation BeginLoc, SourceLocation EndLoc)
      : Version(Version), Platform(Platform), BeginLoc(BeginLoc),
        EndLoc(EndLoc) {}

  /// This constructor is used when representing the '*' case.
  AvailabilitySpec(SourceLocation StarLoc)
      : BeginLoc(StarLoc), EndLoc(StarLoc) {}

  VersionTuple getVersion() const { return Version; }
  StringRef getPlatform() const { return Platform; }
  SourceLocation getBeginLoc() const { return BeginLoc; }
  SourceLocation getEndLoc() const { return EndLoc; }

  /// Returns true when this represents the '*' case.
  bool isOtherPlatformSpec() const { return Version.empty(); }
};

class Decl;

/// Storage of availability attributes for a declaration.
struct AvailabilityInfo {
  /// The domain is the platform for which this availability info applies to.
  toolchain::SmallString<32> Domain;
  VersionTuple Introduced;
  VersionTuple Deprecated;
  VersionTuple Obsoleted;
  bool Unavailable = false;
  bool UnconditionallyDeprecated = false;
  bool UnconditionallyUnavailable = false;

  AvailabilityInfo() = default;

  /// Determine if this AvailabilityInfo represents the default availability.
  bool isDefault() const { return *this == AvailabilityInfo(); }

  /// Check if the symbol has been obsoleted.
  bool isObsoleted() const { return !Obsoleted.empty(); }

  /// Check if the symbol is unavailable unconditionally or
  /// on the active platform and os version.
  bool isUnavailable() const {
    return Unavailable || isUnconditionallyUnavailable();
  }

  /// Check if the symbol is unconditionally deprecated.
  ///
  /// i.e. \code __attribute__((deprecated)) \endcode
  bool isUnconditionallyDeprecated() const { return UnconditionallyDeprecated; }

  /// Check if the symbol is unconditionally unavailable.
  ///
  /// i.e. \code __attribute__((unavailable)) \endcode
  bool isUnconditionallyUnavailable() const {
    return UnconditionallyUnavailable;
  }

  /// Augments the existing information with additional constraints provided by
  /// \c Other.
  void mergeWith(AvailabilityInfo Other);

  AvailabilityInfo(StringRef Domain, VersionTuple I, VersionTuple D,
                   VersionTuple O, bool U, bool UD, bool UU)
      : Domain(Domain), Introduced(I), Deprecated(D), Obsoleted(O),
        Unavailable(U), UnconditionallyDeprecated(UD),
        UnconditionallyUnavailable(UU) {}

  friend bool operator==(const AvailabilityInfo &Lhs,
                         const AvailabilityInfo &Rhs);

public:
  static AvailabilityInfo createFromDecl(const Decl *Decl);
};

inline bool operator==(const AvailabilityInfo &Lhs,
                       const AvailabilityInfo &Rhs) {
  return std::tie(Lhs.Introduced, Lhs.Deprecated, Lhs.Obsoleted,
                  Lhs.Unavailable, Lhs.UnconditionallyDeprecated,
                  Lhs.UnconditionallyUnavailable) ==
         std::tie(Rhs.Introduced, Rhs.Deprecated, Rhs.Obsoleted,
                  Rhs.Unavailable, Rhs.UnconditionallyDeprecated,
                  Rhs.UnconditionallyUnavailable);
}

} // end namespace language::Core

#endif
