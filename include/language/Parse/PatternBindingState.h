//===--- PatternBindingState.h --------------------------------------------===//
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

#ifndef LANGUAGE_PARSE_PATTERNBINDINGSTATE_H
#define LANGUAGE_PARSE_PATTERNBINDINGSTATE_H

#include "language/AST/Decl.h"
#include "language/Parse/Token.h"

namespace language {

class Token;

struct PatternBindingState {
  enum Kind {
    /// InBindingPattern has this value when not parsing a pattern.
    NotInBinding,

    /// InBindingPattern has this value when we're in a matching pattern, but
    /// not within a var/let pattern.  In this phase, identifiers are references
    /// to the enclosing scopes, not a variable binding.
    InMatchingPattern,

    /// InBindingPattern has this value when parsing a pattern in which bound
    /// variables are implicitly immutable, but allowed to be marked mutable by
    /// using a 'var' pattern.  This happens in for-each loop patterns.
    ImplicitlyImmutable,

    /// When InBindingPattern has this value, bound variables are mutable, and
    /// nested let/var patterns are not permitted. This happens when parsing a
    /// 'var' decl or when parsing inside a 'var' pattern.
    InVar,

    /// When InBindingPattern has this value, bound variables are immutable,and
    /// nested let/var patterns are not permitted. This happens when parsing a
    /// 'let' decl or when parsing inside a 'let' pattern.
    InLet,

    /// When InBindingPattern has this value, bound variables are mutable, and
    /// nested let/var/inout patterns are not permitted. This happens when
    /// parsing an 'inout' decl or when parsing inside an 'inout' pattern.
    InInOut,
  };

  Kind kind;

  PatternBindingState(Kind kind) : kind(kind) {}

  operator bool() const { return kind != Kind::NotInBinding; }

  operator Kind() const { return kind; }

  static std::optional<PatternBindingState> get(StringRef str) {
    auto kind = toolchain::StringSwitch<Kind>(str)
                    .Case("let", Kind::InLet)
                    .Case("var", Kind::InVar)
                    .Case("inout", Kind::InInOut)
                    .Default(Kind::NotInBinding);
    return PatternBindingState(kind);
  }

  /// Try to explicitly find the new pattern binding state from a token.
  explicit PatternBindingState(Token tok) : kind(NotInBinding) {
    switch (tok.getKind()) {
    case tok::kw_let:
      kind = InLet;
      break;
    case tok::kw_var:
      kind = InVar;
      break;
    case tok::kw_inout:
      kind = InInOut;
      break;
    default:
      break;
    }
  }

  /// Explicitly initialize from a VarDecl::Introducer.
  explicit PatternBindingState(VarDecl::Introducer introducer)
      : kind(NotInBinding) {
    switch (introducer) {
    case VarDecl::Introducer::Let:
    case VarDecl::Introducer::Borrowing:
      kind = InLet;
      break;
    case VarDecl::Introducer::Var:
      kind = InVar;
      break;
    case VarDecl::Introducer::InOut:
      kind = InInOut;
      break;
    }
  }

  /// If there is a direct introducer associated with this pattern binding
  /// state, return that. Return none otherwise.
  std::optional<VarDecl::Introducer> getIntroducer() const {
    switch (kind) {
    case Kind::NotInBinding:
    case Kind::InMatchingPattern:
    case Kind::ImplicitlyImmutable:
      return std::nullopt;
    case Kind::InVar:
      return VarDecl::Introducer::Var;
    case Kind::InLet:
      return VarDecl::Introducer::Let;
    case Kind::InInOut:
      return VarDecl::Introducer::InOut;
    }
  }

  PatternBindingState
  getPatternBindingStateForIntroducer(VarDecl::Introducer defaultValue) {
    return PatternBindingState(getIntroducer().value_or(defaultValue));
  }

  std::optional<unsigned> getSelectIndexForIntroducer() const {
    switch (kind) {
    case Kind::InLet:
      return 0;
    case Kind::InInOut:
      return 1;
    case Kind::InVar:
      return 2;
    default:
      return std::nullopt;
    }
  }

  bool isLet() const { return kind == Kind::InLet; }
};

} // namespace language

#endif
