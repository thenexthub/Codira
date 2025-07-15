//===--- SymbolGraph.h - Symbol Graph Data Structure ----------------------===//
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

#ifndef LANGUAGE_SYMBOLGRAPHGEN_SYMBOLGRAPH_H
#define LANGUAGE_SYMBOLGRAPHGEN_SYMBOLGRAPH_H

#include "Edge.h"
#include "Symbol.h"
#include "language/Basic/Toolchain.h"
#include "language/Markup/Markup.h"
#include "language/SymbolGraphGen/SymbolGraphOptions.h"
#include "toolchain/ADT/SetVector.h"
#include "toolchain/Support/JSON.h"
#include "toolchain/Support/VersionTuple.h"

namespace language {
namespace symbolgraphgen {

/// A graph of symbols and the relationships between them.
struct SymbolGraph {
  /**
   The options to use while building the graph.
   */
  SymbolGraphASTWalker &Walker;

  /**
   The module this symbol graph represents.
  */
  ModuleDecl &M;

  /**
   The module whose types were extended in `M`.
   */
  std::optional<ModuleDecl *> ExtendedModule;

  /**
   The module declaring `M`, if `M` is a cross-import overlay.
   */
  std::optional<ModuleDecl *> DeclaringModule;

  /**
   The modules that must be imported alongside `DeclaringModule` for `M` to be imported, if `M` is a cross-import overlay.
   */
  SmallVector<Identifier, 1> BystanderModules;

  /**
   A context for allocations.
   */
  markup::MarkupContext &Ctx;

  /**
   The semantic version of the module that this symbol graph describes,
   if known.
   */
  std::optional<toolchain::VersionTuple> ModuleVersion;

  /**
   The symbols in a module: the nodes in the graph.
   */
  toolchain::SetVector<Symbol> Nodes;

  /**
   The relationships between symbols: the edges in the graph.
   */
  toolchain::SetVector<Edge> Edges;

  /**
   True if this graph is for a single symbol, rather than an entire module.
   */
  bool IsForSingleNode;

  SymbolGraph(SymbolGraphASTWalker &Walker, ModuleDecl &M,
              std::optional<ModuleDecl *> ExtendedModule,
              markup::MarkupContext &Ctx,
              std::optional<toolchain::VersionTuple> ModuleVersion = std::nullopt,
              bool IsForSingleNode = false);

  // MARK: - Utilities

  /// Get the base print options for declaration fragments.
  PrintOptions getDeclarationFragmentsPrintOptions() const;


  /// Returns `true` if `VD` is the best known candidate for an
  /// overload set in `Owner`.
  bool synthesizedMemberIsBestCandidate(const ValueDecl *VD,
                                        const NominalTypeDecl *Owner) const;

  /// Get the print options for subHeading declaration fragments.
  PrintOptions getSubHeadingDeclarationFragmentsPrintOptions() const;

  // MARK: - Symbols (Nodes)

  /**
   Record a symbol as a node in the graph.
   */
  void recordNode(Symbol S);

  // MARK: - Relationships (Edges)

  /**
   Record a relationship between two declarations as an edge in the graph.

   \param Source The declaration serving as the source of the edge in the
   directed graph.
   \param Target The declaration serving as the target of the edge in the
   directed graph.
   \param Kind The kind of relationship the edge represents.
   */
  void recordEdge(Symbol Source, Symbol Target, RelationshipKind Kind,
                  const ExtensionDecl *ConformanceExtension = nullptr);

  /**
   Record a MemberOf relationship, if the given declaration is nested
   in another.
   */
  void recordMemberRelationship(Symbol S);

  /**
   If a declaration has members by conforming to a protocol, such as default
   implementations, record a symbol with a "synthesized" USR to disambiguate
   from the protocol's real implementation.

   This makes it more convenient to curate symbols on
   a conformer's documentation.

   The reason these "virtual" members are recorded is to show documentation
   under a conforming type for members with the concrete types substituted.

   For example, if `Array` takes on a function from a collection protocol,
   `subscript(index: Self.Index) -> Element`, the documentation for Array may
   wish to show this function as `subscript(index: Int) -> Element` instead,
   and show unique documentation for it.
   */
  void recordConformanceSynthesizedMemberRelationships(Symbol S);

  /**
   Record InheritsFrom relationships for every class from which the
   declaration inherits.
   */
  void recordInheritanceRelationships(Symbol S);

  /**
   If the declaration is a default implementation in a protocol extension,
   record a DefaultImplementationOf relationship between the declaration and
   the requirement.
   */
  void recordDefaultImplementationRelationships(Symbol S);

  /**
   Record a RequirementOf relationship if the declaration is a requirement
   of a protocol.
   */
  void recordRequirementRelationships(Symbol S);

  /**
   If the declaration is an Objective-C-based optional protocol requirement,
   record an OptionalRequirementOf relationship between the declaration
   and its containing protocol.
   */
  void recordOptionalRequirementRelationships(Symbol S);

  /**
   Record ConformsTo relationships for each protocol conformance of
   the declaration.
   */
  void recordConformanceRelationships(Symbol S);

  /**
   Record ConformsTo relationships for each protocol conformance of
   a declaration through via an extension.
   */
  void recordExtensionConformanceRelationships(Symbol S);

  /**
   Records an Overrides relationship if the given declaration
   overrides another.
   */
  void recordOverrideRelationship(Symbol S);

  // MARK: - Serialization

  /// Serialize this symbol graph's JSON to an output stream.
  void serialize(toolchain::json::OStream &OS);

  /// Serialize the overall declaration fragments for a `ValueDecl`.
  void
  serializeDeclarationFragments(StringRef Key, const Symbol &S,
                                toolchain::json::OStream &OS);

  /// Get the declaration fragments for a symbol when viewed in a navigator
  /// where there is limited horizontal space.
  void
  serializeNavigatorDeclarationFragments(StringRef Key,
                                         const Symbol &S,
                                         toolchain::json::OStream &OS);

  /// Get the declaration fragments for a symbol when it is viewed
  /// as a subheading and/or part of a larger group of symbol listings.
  void
  serializeSubheadingDeclarationFragments(StringRef Key, const Symbol &S,
                                          toolchain::json::OStream &OS);

  /// Get the overall declaration for a symbol.
  void
  serializeDeclarationFragments(StringRef Key, Type T, Type BaseTy,
                                toolchain::json::OStream &OS);

  /// Returns `true` if the declaration has a name that makes it
  /// implicitly internal/private, such as underscore prefixes,
  /// and checking every named parent context as well.
  ///
  /// \param IgnoreContext A function ref that receives the parent decl
  /// and returns whether or not the context should be ignored when determining
  /// privacy.
  bool isImplicitlyPrivate(
      const Decl *D,
      toolchain::function_ref<bool(const Decl *)> IgnoreContext = nullptr) const;

  /// Returns `true` if the declaration has an availability attribute
  /// that marks it as unconditionally unavailable on all platforms (i.e., where
  /// the platform is marked '*').
  bool isUnconditionallyUnavailableOnAllPlatforms(const Decl *D) const;

  /// Returns `true` if the declaration should be included as a node
  /// in the graph.
  ///
  /// If `PublicAncestorDecl` is set and is an ancestor of `D`, that declaration
  /// is considered to be public, regardless of its surrounding context.
  bool canIncludeDeclAsNode(const Decl *D,
                            const Decl *PublicAncestorDecl = nullptr) const;

  /// Returns `true` if the declaration is a requirement of a protocol
  /// or is a default implementation of a protocol
  bool isRequirementOrDefaultImplementation(const ValueDecl *VD) const;

  /// Returns `true` if there are no nodes or edges in this graph.
  bool empty() const {
    return Nodes.empty() && Edges.empty();
  }
};

} // end namespace symbolgraphgen
} // end namespace language 

#endif // LANGUAGE_SYMBOLGRAPHGEN_SYMBOLGRAPH_H
