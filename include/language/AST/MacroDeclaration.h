//===--- MacroDeclaration.h - Swift Macro Declaration -----------*- C++ -*-===//
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
// Data structures that configure the declaration of a macro.
//
//===----------------------------------------------------------------------===//

#ifndef SWIFT_AST_MACRO_DECLARATION_H
#define SWIFT_AST_MACRO_DECLARATION_H

#include "language/AST/Identifier.h"

namespace language {

/// Describes the syntax that is used for a macro, which affects its role.
enum class MacroSyntax: uint8_t {
  /// Freestanding macro syntax starting with an explicit '#' followed by the
  /// macro name and optional arguments.
  Freestanding,

  /// Attached macro syntax written as an attribute with a leading `@` followed
  /// by the macro name an optional arguments.
  Attached,
};

enum class MacroRoleBits: uint8_t {
#define MACRO_ROLE(Name, Description) Name,
#include "language/Basic/MacroRoles.def"
};

/// The context in which a macro can be used, which determines the syntax it
/// uses.
enum class MacroRole: uint32_t {
#define MACRO_ROLE(Name, Description) \
    Name = 1 << static_cast<uint8_t>(MacroRoleBits::Name),
#include "language/Basic/MacroRoles.def"
};

/// Returns an enumeratable list of all macro roles.
std::vector<MacroRole> getAllMacroRoles();

/// The contexts in which a particular macro declaration can be used.
using MacroRoles = OptionSet<MacroRole>;

void simple_display(llvm::raw_ostream &out, MacroRoles roles);
bool operator==(MacroRoles lhs, MacroRoles rhs);
llvm::hash_code hash_value(MacroRoles roles);

/// Retrieve the string form of the given macro role, as written on the
/// corresponding attribute.
StringRef getMacroRoleString(MacroRole role);

/// Whether a macro with the given set of macro contexts is freestanding, i.e.,
/// written in the source code with the `#` syntax.
bool isFreestandingMacro(MacroRoles contexts);

MacroRoles getFreestandingMacroRoles();

/// Whether a macro with the given set of macro contexts is attached, i.e.,
/// written in the source code as an attribute with the `@` syntax.
bool isAttachedMacro(MacroRoles contexts);

MacroRoles getAttachedMacroRoles();

/// Checks if the macro is supported or guarded behind an experimental flag.
bool isMacroSupported(MacroRole role, ASTContext &ctx);

enum class MacroIntroducedDeclNameKind {
  Named,
  Overloaded,
  Prefixed,
  Suffixed,
  Arbitrary,

  // NOTE: When adding a new name kind, also add it to
  // `getAllMacroIntroducedDeclNameKinds`.
};

/// Returns an enumeratable list of all macro introduced decl name kinds.
std::vector<MacroIntroducedDeclNameKind> getAllMacroIntroducedDeclNameKinds();

/// Whether a macro-introduced name of this kind requires an argument.
bool macroIntroducedNameRequiresArgument(MacroIntroducedDeclNameKind kind);

StringRef getMacroIntroducedDeclNameString(
    MacroIntroducedDeclNameKind kind);

class CustomAttr;

class MacroIntroducedDeclName {
public:
  using Kind = MacroIntroducedDeclNameKind;

private:
  Kind kind;
  DeclName name;

public:
  MacroIntroducedDeclName(Kind kind, DeclName name = DeclName())
      : kind(kind), name(name) {};

  static MacroIntroducedDeclName getNamed(DeclName name) {
    return MacroIntroducedDeclName(Kind::Named, name);
  }

  static MacroIntroducedDeclName getOverloaded() {
    return MacroIntroducedDeclName(Kind::Overloaded);
  }

  static MacroIntroducedDeclName getPrefixed(Identifier prefix) {
    return MacroIntroducedDeclName(Kind::Prefixed, prefix);
  }

  static MacroIntroducedDeclName getSuffixed(Identifier suffix) {
    return MacroIntroducedDeclName(Kind::Suffixed, suffix);
  }

  static MacroIntroducedDeclName getArbitrary() {
    return MacroIntroducedDeclName(Kind::Arbitrary);
  }

  Kind getKind() const { return kind; }
  DeclName getName() const { return name; }
};

}

#endif // SWIFT_AST_MACRO_DECLARATION_H
