//===--- Symbol.h - The generics rewrite system alphabet --------*- C++ -*-===//
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

#include "toolchain/ADT/ArrayRef.h"
#include "toolchain/ADT/DenseMap.h"
#include "toolchain/ADT/StringRef.h"
#include <optional>

#ifndef LANGUAGE_RQM_SYMBOL_H
#define LANGUAGE_RQM_SYMBOL_H

namespace toolchain {
  class raw_ostream;
}

namespace language {

class CanType;
class ProtocolDecl;
class GenericTypeParamType;
class Identifier;
class LayoutConstraint;

namespace rewriting {

class MutableTerm;
class RewriteContext;
class Term;

/// The smallest element in the rewrite system.
///
/// enum Symbol {
///   case conformance(CanType, substitutions: [Term], proto: Protocol)
///   case protocol(Protocol)
///   case associatedType(Protocol, Identifier)
///   case genericParam(index: Int, depth: Int)
///   case name(Identifier)
///   case layout(LayoutConstraint)
///   case superclass(CanType, substitutions: [Term])
///   case concrete(CanType, substitutions: [Term])
/// }
///
/// For the concrete type symbol kinds (`superclass`, `concrete` and
/// `conformance`), arbitrary type parameters are replaced with generic
/// parameters with depth 0. The index is the generic parameter is an
/// index into the `substitutions` array.
///
/// This transformation allows DependentMemberTypes to be manipulated as
/// terms, with the actual concrete type structure remaining opaque to
/// the requirement machine. This transformation is implemented in
/// RewriteContext::getSubstitutionSchemaFromType().
///
/// For example, the superclass requirement
/// "T : MyClass<U.X, (Int) -> V.A.B>" is denoted with a symbol
/// structured as follows:
///
/// - type: MyClass<τ_0_0, (Int) -> τ_0_1>
/// - substitutions:
///   - U.X
///   - V.A.B
///
/// Out-of-line methods are documented in Symbol.cpp.
class Symbol final {
public:
  enum class Kind : uint8_t {
    //////
    ////// Special symbol kind that is both type-like and property-like:
    //////

    /// When appearing at the end of a term, denotes that the term's
    /// concrete type or superclass conforms concretely to a protocol.
    ///
    /// Introduced by property map construction when a term has both
    /// a concrete type or superclass requirement and a protocol
    /// conformance requirement.
    ///
    /// This orders before Kind::Protocol, so that a rule of the form
    /// T.[concrete: C : P] => T orders before T.[P] => T. This ensures
    /// that homotopy reduction will try to eliminate the latter rule
    /// first, if possible.
    ConcreteConformance,

    /// When appearing at the start of a term, denotes a nested
    /// type of a protocol 'Self' type.
    ///
    /// When appearing at the end of a term, denotes that the
    /// term's type conforms to the protocol.
    Protocol,

    //////
    ////// "Type-like" symbol kinds:
    //////

    /// An associated type [P:T]. The parent term must be known to
    /// conform to P.
    AssociatedType,

    /// A generic parameter, uniquely identified by depth and
    /// index. Can only appear at the beginning of a term, where
    /// it denotes a generic parameter of the top-level generic
    /// signature.
    GenericParam,

    /// An unbound identifier name.
    Name,

    /// A shape term 'T.[shape]'. The parent term must be a
    /// generic parameter.
    Shape,

    /// A pack element [element].(each T) where 'each T' is a type
    /// parameter pack.
    PackElement,

    //////
    ////// "Property-like" symbol kinds:
    //////

    /// When appearing at the end of a term, denotes that the
    /// term's type satisfies the layout constraint.
    Layout,

    /// When appearing at the end of a term, denotes that the term
    /// is a subclass of the superclass constraint.
    Superclass,

    /// When appearing at the end of a term, denotes that the term
    /// is exactly equal to the concrete type.
    ConcreteType,
  };

  static const unsigned NumKinds = 9;

  static const toolchain::StringRef Kinds[];

private:
  friend class RewriteContext;

  struct Storage;

private:
  const Storage *Ptr;

  Symbol(const Storage *ptr) : Ptr(ptr) {}

public:
  Kind getKind() const;

  /// A property records something about a type term; either a protocol
  /// conformance, a layout constraint, or a superclass or concrete type
  /// constraint.
  bool isProperty() const {
    auto kind = getKind();
    return (kind == Symbol::Kind::ConcreteConformance ||
            kind == Symbol::Kind::Protocol ||
            kind == Symbol::Kind::Layout ||
            kind == Symbol::Kind::Superclass ||
            kind == Symbol::Kind::ConcreteType);
  }

  bool hasSubstitutions() const {
    auto kind = getKind();
    return (kind == Kind::Superclass ||
            kind == Kind::ConcreteType ||
            kind == Kind::ConcreteConformance);
  }

  Identifier getName() const;

  const ProtocolDecl *getProtocol() const;

  GenericTypeParamType *getGenericParam() const;

  LayoutConstraint getLayoutConstraint() const;

  CanType getConcreteType() const;

  toolchain::ArrayRef<Term> getSubstitutions() const;

  /// Returns an opaque pointer that uniquely identifies this symbol.
  const void *getOpaquePointer() const {
    return Ptr;
  }

  static Symbol fromOpaquePointer(void *ptr) {
    return Symbol((Storage *) ptr);
  }

  static Symbol forName(Identifier name,
                        RewriteContext &ctx);

  static Symbol forProtocol(const ProtocolDecl *proto,
                            RewriteContext &ctx);

  static Symbol forAssociatedType(const ProtocolDecl *proto,
                                  Identifier name,
                                  RewriteContext &ctx);

  static Symbol forGenericParam(GenericTypeParamType *param,
                                RewriteContext &ctx);

  static Symbol forShape(RewriteContext &ctx);

  static Symbol forPackElement(RewriteContext &Ctx);

  static Symbol forLayout(LayoutConstraint layout,
                          RewriteContext &ctx);

  static Symbol forSuperclass(CanType type, toolchain::ArrayRef<Term> substitutions,
                              RewriteContext &ctx);

  static Symbol forConcreteType(CanType type,
                                toolchain::ArrayRef<Term> substitutions,
                                RewriteContext &ctx);

  static Symbol forConcreteConformance(CanType type,
                                       toolchain::ArrayRef<Term> substitutions,
                                       const ProtocolDecl *proto,
                                       RewriteContext &ctx);

  const ProtocolDecl *getRootProtocol() const;

  std::optional<int> compare(Symbol other, RewriteContext &ctx) const;

  Symbol withConcreteSubstitutions(toolchain::ArrayRef<Term> substitutions,
                                   RewriteContext &ctx) const;

  Symbol transformConcreteSubstitutions(
      toolchain::function_ref<Term(Term)> fn,
      RewriteContext &ctx) const;

  Symbol prependPrefixToConcreteSubstitutions(
      const MutableTerm &prefix,
      RewriteContext &ctx) const;

  bool containsNameSymbols() const;

  void dump(toolchain::raw_ostream &out) const;

  friend bool operator==(Symbol lhs, Symbol rhs) {
    return lhs.Ptr == rhs.Ptr;
  }

  friend bool operator!=(Symbol lhs, Symbol rhs) {
    return !(lhs == rhs);
  }

  friend toolchain::raw_ostream &operator<<(toolchain::raw_ostream &out, Symbol symbol) {
    symbol.dump(out);
    return out;
  }
};

} // end namespace rewriting

} // end namespace language

namespace toolchain {
  template<> struct DenseMapInfo<language::rewriting::Symbol> {
    static language::rewriting::Symbol getEmptyKey() {
      return language::rewriting::Symbol::fromOpaquePointer(
        toolchain::DenseMapInfo<void *>::getEmptyKey());
    }
    static language::rewriting::Symbol getTombstoneKey() {
      return language::rewriting::Symbol::fromOpaquePointer(
        toolchain::DenseMapInfo<void *>::getTombstoneKey());
    }
    static unsigned getHashValue(language::rewriting::Symbol Val) {
      return DenseMapInfo<void *>::getHashValue(Val.getOpaquePointer());
    }
    static bool isEqual(language::rewriting::Symbol LHS,
                        language::rewriting::Symbol RHS) {
      return LHS == RHS;
    }
  };

  template<>
  struct PointerLikeTypeTraits<language::rewriting::Symbol> {
  public:
    static inline void *getAsVoidPointer(language::rewriting::Symbol Val) {
      return const_cast<void *>(Val.getOpaquePointer());
    }
    static inline language::rewriting::Symbol getFromVoidPointer(void *Ptr) {
      return language::rewriting::Symbol::fromOpaquePointer(Ptr);
    }
    enum { NumLowBitsAvailable = 1 };
  };
} // end namespace toolchain

#endif
