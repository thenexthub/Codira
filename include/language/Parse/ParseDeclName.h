//===--- ParseDeclName.h ----------------------------------------*- C++ -*-===//
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

#ifndef LANGUAGE_PARSE_PARSEDECLNAME_H
#define LANGUAGE_PARSE_PARSEDECLNAME_H

#include "language/AST/Identifier.h"
#include "language/Basic/Toolchain.h"
#include "language/Parse/Lexer.h"

namespace language {

/// Describes a parsed declaration name.
struct ParsedDeclName {
  /// The name of the context of which the corresponding entity should
  /// become a member.
  StringRef ContextName;

  /// The base name of the declaration.
  StringRef BaseName;

  /// The argument labels for a function declaration.
  SmallVector<StringRef, 4> ArgumentLabels;

  /// Whether this is a function name (vs. a value name).
  bool IsFunctionName = false;

  /// Whether this is a getter for the named property.
  bool IsGetter = false;

  /// Whether this is a setter for the named property.
  bool IsSetter = false;

  bool IsSubscript = false;

  /// For a declaration name that makes the declaration into an
  /// instance member, the index of the "Self" parameter.
  std::optional<unsigned> SelfIndex;

  /// Determine whether this is a valid name.
  explicit operator bool() const { return !BaseName.empty(); }

  /// Whether this declaration name turns the declaration into a
  /// member of some named context.
  bool isMember() const { return !ContextName.empty(); }

  /// Whether the result is translated into an instance member.
  bool isInstanceMember() const {
    return isMember() && static_cast<bool>(SelfIndex);
  }

  /// Whether the result is translated into a static/class member.
  bool isClassMember() const {
    return isMember() && !static_cast<bool>(SelfIndex);
  }

  /// Whether this is a property accessor.
  bool isPropertyAccessor() const { return IsGetter || IsSetter; }

  /// Whether this is an operator.
  bool isOperator() const { return Lexer::isOperator(BaseName); }

  /// Form a declaration name from this parsed declaration name.
  DeclName formDeclName(ASTContext &ctx, bool isSubscript = false,
                        bool isCxxClassTemplateSpec = false) const;

  /// Form a declaration name from this parsed declaration name.
  DeclNameRef formDeclNameRef(ASTContext &ctx, bool isSubscript = false,
                              bool isCxxClassTemplateSpec = false) const;
};

/// Parse a stringified Codira declaration name,
/// e.g. "Foo.translateBy(self:x:y:)".
ParsedDeclName parseDeclName(StringRef name) TOOLCHAIN_READONLY;

/// Form a Codira declaration name from its constituent parts.
DeclName formDeclName(ASTContext &ctx, StringRef baseName,
                      ArrayRef<StringRef> argumentLabels, bool isFunctionName,
                      bool isInitializer, bool isSubscript = false,
                      bool isCxxClassTemplateSpec = false);

/// Form a Codira declaration name reference from its constituent parts.
DeclNameRef formDeclNameRef(ASTContext &ctx, StringRef baseName,
                            ArrayRef<StringRef> argumentLabels,
                            bool isFunctionName, bool isInitializer,
                            bool isSubscript = false,
                            bool isCxxClassTemplateSpec = false);

} // namespace language

#endif // LANGUAGE_PARSE_PARSEDECLNAME_H
