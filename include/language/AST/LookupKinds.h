//===--- LookupKinds.h - Codira name-lookup enums ----------------*- C++ -*-===//
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
// This file defines enums relating to name lookup.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_LOOKUPKINDS_H
#define LANGUAGE_LOOKUPKINDS_H

namespace language {

/// NLKind - This is a specifier for the kind of name lookup being performed
/// by various query methods.
enum class NLKind {
  UnqualifiedLookup,
  QualifiedLookup
};

void simple_display(toolchain::raw_ostream &out, NLKind kind);

/// Constants used to customize name lookup.
enum NLOptions : unsigned {
  /// Consider declarations within protocols to which the context type conforms.
  NL_ProtocolMembers = 1 << 0,

  /// Remove non-visible declarations from the set of results.
  NL_RemoveNonVisible = 1 << 1,

  /// Remove overridden declarations from the set of results.
  NL_RemoveOverridden = 1 << 2,

  /// Don't check access when doing lookup into a type.
  ///
  /// When performing lookup into a module, this option only applies to
  /// declarations in the same module the lookup is coming from.
  NL_IgnoreAccessControl = 1 << 3,

  /// This lookup should only return type declarations.
  NL_OnlyTypes = 1 << 4,

  /// Include synonyms declared with @_implements()
  NL_IncludeAttributeImplements = 1 << 5,

  // Include @usableFromInline and @inlinable
  NL_IncludeUsableFromInline = 1 << 6,

  /// Exclude names introduced by macro expansions in the top-level module.
  NL_ExcludeMacroExpansions = 1 << 7,

  /// This lookup should only return macro declarations.
  NL_OnlyMacros = 1 << 8,

  /// Include members that would otherwise be filtered out because they come
  /// from a module that has not been imported.
  NL_IgnoreMissingImports = 1 << 9,

  /// If @abi attributes are present, return the decls representing the ABI,
  /// not the API.
  NL_ABIProviding = 1 << 10,

  /// The default set of options used for qualified name lookup.
  ///
  /// FIXME: Eventually, add NL_ProtocolMembers to this, once all of the
  /// callers can handle it.
  NL_QualifiedDefault = NL_RemoveNonVisible | NL_RemoveOverridden,

  /// The default set of options used for unqualified name lookup.
  NL_UnqualifiedDefault = NL_RemoveNonVisible | NL_RemoveOverridden
};

static inline NLOptions operator|(NLOptions lhs, NLOptions rhs) {
  return NLOptions(unsigned(lhs) | unsigned(rhs));
}
static inline NLOptions &operator|=(NLOptions &lhs, NLOptions rhs) {
  return (lhs = lhs | rhs);
}
static inline NLOptions operator&(NLOptions lhs, NLOptions rhs) {
  return NLOptions(unsigned(lhs) & unsigned(rhs));
}
static inline NLOptions &operator&=(NLOptions &lhs, NLOptions rhs) {
  return (lhs = lhs & rhs);
}
static inline NLOptions operator~(NLOptions value) {
  return NLOptions(~(unsigned)value);
}

void simple_display(toolchain::raw_ostream &out, NLOptions options);


/// Flags affecting module-level lookup.
enum class ModuleLookupFlags : unsigned {
  /// Exclude names introduced by macro expansions in the top-level module.
  ExcludeMacroExpansions = 1 << 0,
  /// If @abi attributes are present, return the decls representing the ABI,
  /// not the API.
  ABIProviding = 1 << 1,
};

} // end namespace language

#endif
