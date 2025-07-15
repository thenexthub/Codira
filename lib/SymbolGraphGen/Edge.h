//===--- Edge.h - Symbol Graph Edge ---------------------------------------===//
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

#ifndef LANGUAGE_SYMBOLGRAPHGEN_EDGE_H
#define LANGUAGE_SYMBOLGRAPHGEN_EDGE_H

#include "toolchain/Support/JSON.h"
#include "language/AST/Decl.h"
#include "language/AST/Type.h"
#include "language/Basic/Toolchain.h"

#include "JSON.h"
#include "Symbol.h"

namespace language {
namespace symbolgraphgen {

struct SymbolGraph;
  
/// The kind of relationship, tagging an edge in the graph.
struct RelationshipKind {
  StringRef Name;
  
  RelationshipKind(toolchain::StringRef Name) : Name(Name) {}
  
  /**
   A symbol A is a member of another symbol B.
   
   For example, a method or field of a class would be a member of that class.
   
   The implied inverse of this relationship is a symbol B is the owner
   of a member symbol A.
   */
  static inline RelationshipKind MemberOf() {
    return RelationshipKind { "memberOf" };
  }

  /**
   A symbol A conforms to an interface/protocol symbol B.
   
   For example, a class `C` that conforms to protocol `P` in Codira would use
   this relationship.
   
   The implied inverse of this relationship is a symbol B that has
   a conformer A.
   */
  static inline RelationshipKind ConformsTo() {
    return RelationshipKind { "conformsTo" };
  }
  /**
   A symbol A inherits from another symbol B.
   
   For example, a derived class inherits from a base class, or a protocol
   that refines another protocol would use this relationship.
   
   The implied inverse of this relationship is a symbol B is a base
   of another symbol A.
   */
  static inline RelationshipKind InheritsFrom() {
    return RelationshipKind { "inheritsFrom" };
  }
  /**
   A symbol A serves as a default implementation of an interface requirement B.
   
   The implied inverse of this relationship is an interface requirement B
   has a default implementation of A.
   */
  static inline RelationshipKind DefaultImplementationOf() {
    return RelationshipKind { "defaultImplementationOf" };
  }
  /**
   A symbol A overrides another symbol B, such as through inheritance.
   
   The implied inverse of this relationship is a symbol A is the base
   of symbol B.
   */
  static inline RelationshipKind Overrides() {
    return RelationshipKind { "overrides" };
  }
  /**
   A symbol A is a requirement of interface B.
   
   The implied inverse of this relationship is an interface B
   has a requirement of A.
   */
  static inline RelationshipKind RequirementOf() {
    return RelationshipKind { "requirementOf" };
  }
  /**
   A symbol A is an optional requirement of interface B.
   
   The implied inverse of this relationship is an interface B
   has an optional requirement of A.
   */
  static inline RelationshipKind OptionalRequirementOf() {
    return RelationshipKind { "optionalRequirementOf" };
  }

  /**
   A symbol A extends a symbol B with members or conformances.

   This relationship describes the connection between extension blocks
   (language.extension symbols) and the type they extend.

   The implied inverse of this relationship is a symbol B that is extended
   by an extension block symbol A.
   */
  static inline RelationshipKind ExtensionTo() {
    return RelationshipKind{"extensionTo"};
  }

  bool operator==(const RelationshipKind &Other) const {
    return Name == Other.Name;
  }

  bool operator<(const RelationshipKind &Other) const {
    return Name < Other.Name;
  }
};

/// A relationship between two symbols: an edge in a directed graph.
struct Edge {
  SymbolGraph *Graph;

  /// The kind of relationship this edge represents.
  RelationshipKind Kind;

  /// The precise identifier of the source symbol node.
  Symbol Source;
  
  /// The precise identifier of the target symbol node.
  Symbol Target;

  /// If this is a conformsTo relationship, the extension that defined
  /// the conformance.
  const ExtensionDecl *ConformanceExtension;
  
  void serialize(toolchain::json::OStream &OS) const;
};
  
} // end namespace symbolgraphgen 
} // end namespace language

namespace toolchain {
using SymbolGraph = language::symbolgraphgen::SymbolGraph;
using Symbol = language::symbolgraphgen::Symbol;
using Edge = language::symbolgraphgen::Edge;
using ExtensionDecl = language::ExtensionDecl;
template <> struct DenseMapInfo<Edge> {
  static inline Edge getEmptyKey() {
    return {
      DenseMapInfo<SymbolGraph *>::getEmptyKey(),
      { "Empty" },
      DenseMapInfo<Symbol>::getEmptyKey(),
      DenseMapInfo<Symbol>::getEmptyKey(),
      DenseMapInfo<const ExtensionDecl *>::getEmptyKey(),
    };
  }
  static inline Edge getTombstoneKey() {
    return {
      nullptr,
      { "Tombstone" },
      DenseMapInfo<Symbol>::getTombstoneKey(),
      DenseMapInfo<Symbol>::getTombstoneKey(),
      DenseMapInfo<const ExtensionDecl *>::getTombstoneKey(),
    };
  }
  static unsigned getHashValue(const Edge E) {
    unsigned H = 0;
    H ^= DenseMapInfo<StringRef>::getHashValue(E.Kind.Name);
    H ^= DenseMapInfo<Symbol>::getHashValue(E.Source);
    H ^= DenseMapInfo<Symbol>::getHashValue(E.Target);
    H ^= DenseMapInfo<const ExtensionDecl *>::
         getHashValue(E.ConformanceExtension);
    return H;
  }
  static bool isEqual(const Edge LHS, const Edge RHS) {
    return LHS.Kind == RHS.Kind &&
      DenseMapInfo<Symbol>::isEqual(LHS.Source, RHS.Source) &&
      DenseMapInfo<Symbol>::isEqual(LHS.Target, RHS.Target) &&
      DenseMapInfo<const ExtensionDecl *>::isEqual(LHS.ConformanceExtension,
                                                   RHS.ConformanceExtension);
  }
};
} // end namespace toolchain

#endif // LANGUAGE_SYMBOLGRAPHGEN_EDGE_H
