//===--- ReferenceDependencyKeys.h - Keys for languagedeps files ---*- C++ -*-===//
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

#ifndef LANGUAGE_BASIC_REFERENCEDEPENDENCYKEYS_H
#define LANGUAGE_BASIC_REFERENCEDEPENDENCYKEYS_H

#include "language/Basic/Toolchain.h"
#include "toolchain/ADT/StringRef.h"

namespace language {
/// Define these string constants for reference dependencies (a.k.a. languagedeps)
/// in one place to ensure consistency.
namespace reference_dependency_keys {
static constexpr StringLiteral providesTopLevel("provides-top-level");
static constexpr StringLiteral providesNominal("provides-nominal");
static constexpr StringLiteral providesMember("provides-member");
static constexpr StringLiteral providesDynamicLookup("provides-dynamic-lookup");

static constexpr StringLiteral dependsTopLevel("depends-top-level");
static constexpr StringLiteral dependsMember("depends-member");
static constexpr StringLiteral dependsNominal("depends-nominal");
static constexpr StringLiteral dependsDynamicLookup("depends-dynamic-lookup");
static constexpr StringLiteral dependsExternal("depends-external");

static constexpr StringLiteral interfaceHash("interface-hash");
} // end namespace reference_dependency_keys

namespace fine_grained_dependencies {
/// Encode the current sorts of dependencies as kinds of nodes in the dependency
/// graph, splitting the current *member* into \ref member and \ref
/// potentialMember and adding \ref sourceFileProvide.

enum class NodeKind {
  topLevel,
  nominal,
  /// In the status quo scheme, *member* dependencies could have blank names
  /// for the member, to indicate that the provider might add members.
  /// This code uses a separate kind, \ref potentialMember. The holder field is
  /// unused.
  potentialMember,
  /// Corresponding to the status quo *member* dependency with a non-blank
  /// member.
  member,
  dynamicLookup,
  externalDepend,
  sourceFileProvide,
  /// For iterating through the NodeKinds.
  kindCount
};

/// Used for printing out NodeKinds to dot files, and dumping nodes for
/// debugging.
const std::string NodeKindNames[]{
    "topLevel",          "nominal",
    "potentialMember",   "member",
    "dynamicLookup",     "externalDepend",
    "sourceFileProvide"};
} // end namespace fine_grained_dependencies
} // end namespace language

#endif
