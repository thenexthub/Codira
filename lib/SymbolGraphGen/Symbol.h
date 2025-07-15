//===--- Symbol.h- Symbol Graph Node --------------------------------------===//
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

#ifndef LANGUAGE_SYMBOLGRAPHGEN_SYMBOL_H
#define LANGUAGE_SYMBOLGRAPHGEN_SYMBOL_H

#include "toolchain/Support/JSON.h"
#include "language/AST/Attr.h"
#include "language/AST/Type.h"
#include "language/Basic/Toolchain.h"
#include "language/Markup/Markup.h"
#include "language/SymbolGraphGen/PathComponent.h"
#include "language/SymbolGraphGen/FragmentInfo.h"

namespace language {
namespace symbolgraphgen {

struct Availability;
struct SymbolGraphASTWalker;
struct SymbolGraph;

/// A symbol from a module: a node in a graph.
class Symbol {
  /// The symbol graph in which this symbol resides.
  SymbolGraph *Graph;
  /// Either a ValueDecl* or ExtensionDecl*.
  const Decl *D;
  Type BaseType;
  const ValueDecl *SynthesizedBaseTypeDecl;

  Symbol(SymbolGraph *Graph, const ValueDecl *VD, const ExtensionDecl *ED,
         const ValueDecl *SynthesizedBaseTypeDecl,
         Type BaseTypeForSubstitution = Type());

  language::DeclName getName(const Decl *D) const;

  void serializeKind(StringRef Identifier, StringRef DisplayName,
                     toolchain::json::OStream &OS) const;

  void serializeKind(toolchain::json::OStream &OS) const;

  void serializeIdentifier(toolchain::json::OStream &OS) const;

  void serializePathComponents(toolchain::json::OStream &OS) const;

  void serializeNames(toolchain::json::OStream &OS) const;

  void serializePosition(StringRef Key, SourceLoc Loc,
                         SourceManager &SourceMgr,
                         toolchain::json::OStream &OS) const;

  void serializeRange(size_t InitialIndentation,
                      SourceRange Range, SourceManager &SourceMgr,
                      toolchain::json::OStream &OS) const;

  void serializeDocComment(toolchain::json::OStream &OS) const;

  void serializeFunctionSignature(toolchain::json::OStream &OS) const;

  void serializeGenericParam(const language::GenericTypeParamType &Param,
                             toolchain::json::OStream &OS) const;

  void serializeCodiraGenericMixin(toolchain::json::OStream &OS) const;

  void serializeCodiraExtensionMixin(toolchain::json::OStream &OS) const;

  void serializeDeclarationFragmentMixin(toolchain::json::OStream &OS) const;

  void serializeAccessLevelMixin(toolchain::json::OStream &OS) const;

  void serializeMetadataMixin(toolchain::json::OStream &OS) const;

  void serializeLocationMixin(toolchain::json::OStream &OS) const;

  void serializeAvailabilityMixin(toolchain::json::OStream &OS) const;

  void serializeSPIMixin(toolchain::json::OStream &OS) const;

public:
  Symbol(SymbolGraph *Graph, const ExtensionDecl *ED,
         const ValueDecl *SynthesizedBaseTypeDecl,
         Type BaseTypeForSubstitution = Type());

  Symbol(SymbolGraph *Graph, const ValueDecl *VD,
         const ValueDecl *SynthesizedBaseTypeDecl,
         Type BaseTypeForSubstitution = Type());

  void serialize(toolchain::json::OStream &OS) const;

  const SymbolGraph *getGraph() const {
    return Graph;
  }

  const ValueDecl *getSymbolDecl() const;

  const Decl *getLocalSymbolDecl() const { return D; }

  Type getBaseType() const {
    return BaseType;
  }

  const ValueDecl *getSynthesizedBaseTypeDecl() const {
    return SynthesizedBaseTypeDecl;
  }

  /// Retrieve the path components associated with this symbol, from outermost
  /// to innermost (this symbol).
  void getPathComponents(SmallVectorImpl<PathComponent> &Components) const;

  /// Retrieve information about all symbols referenced in the declaration
  /// fragment printed for this symbol.
  void getFragmentInfo(SmallVectorImpl<FragmentInfo> &FragmentInfo) const;

  /// Print the symbol path to an output stream.
  void printPath(toolchain::raw_ostream &OS) const;

  void getUSR(SmallVectorImpl<char> &USR) const;
  
  /// If this symbol is inheriting docs from a parent class, protocol, or default
  /// implementation, returns that decl. Returns null if there are no docs or if
  /// the symbol has its own doc comments to render.
  const ValueDecl *getDeclInheritingDocs() const;

  /// If this symbol is an implementation of a protocol requirement for a
  /// protocol declared outside its module, returns the upstream decl for that
  /// requirement.
  const ValueDecl *getForeignProtocolRequirement() const;

  /// If this symbol is an implementation of a protocol requirement, returns the
  /// upstream decl for that requirement.
  const ValueDecl *getProtocolRequirement() const;

  /// If this symbol is a synthesized symbol or an implementation of a protocol
  /// requirement, returns the upstream decl.
  const ValueDecl *getInheritedDecl() const;

  static bool supportsKind(DeclKind Kind);

  /// Determines the effective access level of the given extension.
  ///
  /// The effective access level is defined as the minimum of:
  ///  - the maximum access level of a property or conformance
  ///  - the access level of the extended nominal
  ///
  /// The effective access level is defined this way so that the extension
  /// symbol's access level equals the highest access level of any of the
  /// symbols the extension symbol has a relationship to.
  ///
  /// This function is not logically equivalent to
  /// `ExtensionDecl.getMaxAccessLevel()`, which computes the maximum access
  /// level any of the `ExtensionDecl`'s members
  /// **can** have based on the extended type and types used in constraints.
  static AccessLevel getEffectiveAccessLevel(const ExtensionDecl *ED);

  /// Determines the kind of Symbol the given declaration produces and
  /// returns the respective symbol kind identifier and kind name.
  static std::pair<StringRef, StringRef> getKind(const Decl *D);
};

} // end namespace symbolgraphgen
} // end namespace language

namespace toolchain {
using Symbol = language::symbolgraphgen::Symbol;
using SymbolGraph = language::symbolgraphgen::SymbolGraph;

template <> struct DenseMapInfo<Symbol> {
  static inline Symbol getEmptyKey() {
    return Symbol{
        DenseMapInfo<SymbolGraph *>::getEmptyKey(),
        DenseMapInfo<const language::ValueDecl *>::getEmptyKey(),
        DenseMapInfo<const language::ValueDecl *>::getTombstoneKey(),
        DenseMapInfo<language::Type>::getEmptyKey(),
    };
  }
  static inline Symbol getTombstoneKey() {
    return Symbol{
        DenseMapInfo<SymbolGraph *>::getTombstoneKey(),
        DenseMapInfo<const language::ValueDecl *>::getTombstoneKey(),
        DenseMapInfo<const language::ValueDecl *>::getTombstoneKey(),
        DenseMapInfo<language::Type>::getTombstoneKey(),
    };
  }
  static unsigned getHashValue(const Symbol S) {
    unsigned H = 0;
    H ^= DenseMapInfo<SymbolGraph *>::getHashValue(S.getGraph());
    H ^=
        DenseMapInfo<const language::Decl *>::getHashValue(S.getLocalSymbolDecl());
    H ^= DenseMapInfo<const language::ValueDecl *>::getHashValue(
        S.getSynthesizedBaseTypeDecl());
    H ^= DenseMapInfo<language::Type>::getHashValue(S.getBaseType());
    return H;
  }
  static bool isEqual(const Symbol LHS, const Symbol RHS) {
    return LHS.getGraph() == RHS.getGraph() &&
           LHS.getLocalSymbolDecl() == RHS.getLocalSymbolDecl() &&
           LHS.getSynthesizedBaseTypeDecl() ==
               RHS.getSynthesizedBaseTypeDecl() &&
           DenseMapInfo<language::Type>::isEqual(LHS.getBaseType(),
                                              RHS.getBaseType());
  }
};
} // end namespace toolchain


#endif // LANGUAGE_SYMBOLGRAPHGEN_SYMBOL_H
