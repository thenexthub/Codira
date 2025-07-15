//===--- Identifier.cpp - Uniqued Identifier ------------------------------===//
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
// This file implements the Identifier interface.
//
//===----------------------------------------------------------------------===//

#include "language/AST/ASTContext.h"
#include "language/AST/Identifier.h"
#include "language/Basic/Assertions.h"
#include "language/Parse/Lexer.h"
#include "toolchain/ADT/StringExtras.h"
#include "toolchain/Support/raw_ostream.h"
#include "toolchain/Support/ConvertUTF.h"
#include "clang/Basic/CharInfo.h"
using namespace language;

constexpr const Identifier::Aligner DeclBaseName::SubscriptIdentifierData{};
constexpr const Identifier::Aligner DeclBaseName::ConstructorIdentifierData{};
constexpr const Identifier::Aligner DeclBaseName::DestructorIdentifierData{};

raw_ostream &toolchain::operator<<(raw_ostream &OS, Identifier I) {
  if (I.get() == nullptr)
    return OS << "_";
  return OS << I.get();
}

raw_ostream &toolchain::operator<<(raw_ostream &OS, DeclBaseName D) {
  return OS << D.userFacingName();
}

raw_ostream &toolchain::operator<<(raw_ostream &OS, DeclName I) {
  if (I.isSimpleName())
    return OS << I.getBaseName();

  OS << I.getBaseName() << "(";
  for (auto c : I.getArgumentNames()) {
    OS << c << ':';
  }
  OS << ")";
  return OS;
}

void language::simple_display(toolchain::raw_ostream &out, DeclName name) {
  out << "'" << name << "'";
}

raw_ostream &toolchain::operator<<(raw_ostream &OS, DeclNameRef I) {
  if (I.hasModuleSelector())
    OS << I.getModuleSelector() << "::";
  OS << I.getFullName();
  return OS;
}

void language::simple_display(toolchain::raw_ostream &out, DeclNameRef name) {
  out << "'" << name << "'";
}

raw_ostream &toolchain::operator<<(raw_ostream &OS, language::ObjCSelector S) {
  unsigned n = S.getNumArgs();
  if (n == 0) {
    OS << S.getSelectorPieces()[0];
    return OS;
  }

  for (auto piece : S.getSelectorPieces()) {
    if (!piece.empty())
      OS << piece;
    OS << ":";
  }
  return OS;
}

bool Identifier::isOperatorSlow() const { return Lexer::isOperator(str()); }

bool Identifier::mustAlwaysBeEscaped() const {
  return Lexer::identifierMustAlwaysBeEscaped(str());
}

int Identifier::compare(Identifier other) const {
  // Handle empty identifiers.
  if (empty() || other.empty()) {
    if (empty() != other.empty()) {
      return other.empty() ? -1 : 1;
    }

    return 0;
  }

  return str().compare(other.str());
}

int DeclName::compare(DeclName other) const {
  // Fast equality comparsion.
  if (getOpaqueValue() == other.getOpaqueValue())
    return 0;


  // Compare base names.
  if (int result = getBaseName().compare(other.getBaseName()))
    return result;

  // Compare argument names.
  auto argNames = getArgumentNames();
  auto otherArgNames = other.getArgumentNames();
  for (unsigned i = 0, n = std::min(argNames.size(), otherArgNames.size());
       i != n; ++i) {
    if (int result = argNames[i].compare(otherArgNames[i]))
      return result;
  }

  if (argNames.size() != otherArgNames.size())
    return argNames.size() < otherArgNames.size() ? -1 : 1;

  // Order based on if it is compound name or not.
  assert(isSimpleName() != other.isSimpleName() &&
         "equality should be covered by opaque value comparsion");
  return isSimpleName() ? -1 : 1;
}

static bool equals(ArrayRef<Identifier> idents, ArrayRef<StringRef> strings) {
  if (idents.size() != strings.size())
    return false;
  for (size_t i = 0, e = idents.size(); i != e; ++i) {
    if (!idents[i].is(strings[i]))
      return false;
  }
  return true;
}

bool DeclName::isCompoundName(DeclBaseName baseName,
                              ArrayRef<StringRef> argNames) const {
  return (isCompoundName() &&
          getBaseName() == baseName &&
          equals(getArgumentNames(), argNames));
}

bool DeclName::isCompoundName(StringRef baseName,
                              ArrayRef<StringRef> argNames) const {
  return (isCompoundName() &&
          getBaseName() == baseName &&
          equals(getArgumentNames(), argNames));
}

void DeclName::dump() const {
  toolchain::errs() << *this << "\n";
}

StringRef DeclName::getString(toolchain::SmallVectorImpl<char> &scratch,
                              bool skipEmptyArgumentNames) const {
  {
    toolchain::raw_svector_ostream out(scratch);
    print(out, skipEmptyArgumentNames);
  }

  return StringRef(scratch.data(), scratch.size());
}

toolchain::raw_ostream &DeclName::print(toolchain::raw_ostream &os,
                                   bool skipEmptyArgumentNames,
                                   bool escapeIfNeeded) const {
  // Print the base name.
  auto baseName = getBaseName();
  if (escapeIfNeeded && baseName.mustAlwaysBeEscaped()) {
    os << "`" << baseName << "`";
  } else {
    os << baseName;
  }

  // If this is a simple name, we're done.
  if (isSimpleName())
    return os;

  if (skipEmptyArgumentNames) {
    // If there is more than one argument yet none of them have names,
    // we're done.
    if (!getArgumentNames().empty()) {
      bool anyNonEmptyNames = false;
      for (auto c : getArgumentNames()) {
        if (!c.empty()) {
          anyNonEmptyNames = true;
          break;
        }
      }

      if (!anyNonEmptyNames)
        return os;
    }
  }

  // Print the argument names.
  os << "(";
  for (auto argName : getArgumentNames()) {
    if (escapeIfNeeded && argName.mustAlwaysBeEscaped()) {
      os << "`" << argName << "`";
    } else {
      os << argName;
    }
    os << ':';
  }
  os << ")";
  return os;

}

toolchain::raw_ostream &DeclName::printPretty(toolchain::raw_ostream &os) const {
  return print(os, /*skipEmptyArgumentNames=*/!isSpecial());
}

void DeclNameRef::dump() const {
  toolchain::errs() << *this << "\n";
}

StringRef DeclNameRef::getString(toolchain::SmallVectorImpl<char> &scratch,
                                 bool skipEmptyArgumentNames) const {
  {
    toolchain::raw_svector_ostream out(scratch);
    print(out, skipEmptyArgumentNames);
  }

  return StringRef(scratch.data(), scratch.size());
}

toolchain::raw_ostream &
DeclNameRef::print(toolchain::raw_ostream &os,
                   bool skipEmptyArgumentNames) const {
  if (hasModuleSelector())
    os << getModuleSelector() << "::";
  return getFullName().print(os, skipEmptyArgumentNames);
}

toolchain::raw_ostream &DeclNameRef::printPretty(toolchain::raw_ostream &os) const {
  if (hasModuleSelector())
    os << getModuleSelector() << "::";
  return getFullName().printPretty(os);
}

ObjCSelector::ObjCSelector(ASTContext &ctx, unsigned numArgs,
                           ArrayRef<Identifier> pieces) {
  if (numArgs == 0) {
    assert(pieces.size() == 1 && "No-argument selector requires one piece");
    Storage = DeclName(pieces[0]);
    return;
  }

  assert(numArgs == pieces.size() && "Wrong number of selector pieces");
  Storage = DeclName(ctx, Identifier(), pieces);
}

std::optional<ObjCSelector> ObjCSelector::parse(ASTContext &ctx,
                                                StringRef string) {
  // Find the first colon.
  auto colonPos = string.find(':');

  // If there is no colon, we have a nullary selector.
  if (colonPos == StringRef::npos) {
    if (string.empty() || !Lexer::isIdentifier(string))
      return std::nullopt;
    return ObjCSelector(ctx, 0, { ctx.getIdentifier(string) });
  }

  SmallVector<Identifier, 2> pieces;
  do {
    // Check whether we have a valid selector piece.
    auto piece = string.substr(0, colonPos);
    if (piece.empty()) {
      pieces.push_back(Identifier());
    } else {
      if (!Lexer::isIdentifier(piece))
        return std::nullopt;
      pieces.push_back(ctx.getIdentifier(piece));
    }

    // Move to the next piece.
    string = string.substr(colonPos+1);
    colonPos = string.find(':');
  } while (colonPos != StringRef::npos);

  // If anything remains of the string, it's not a selector.
  if (!string.empty())
    return std::nullopt;

  return ObjCSelector(ctx, pieces.size(), pieces);
}

ObjCSelectorFamily ObjCSelector::getSelectorFamily() const {
  StringRef text = getSelectorPieces().front().get();
  while (!text.empty() && text[0] == '_') text = text.substr(1);

  // Does the given selector start with the given string as a prefix, in the
  // sense of the selector naming conventions?
  // This implementation matches the one used by
  // clang::Selector::getMethodFamily, to make sure we behave the same as
  // Clang ARC. We're not just calling that method here because it means
  // allocating a clang::IdentifierInfo, which requires a Clang ASTContext.
  auto hasPrefix = [](StringRef text, StringRef prefix) {
    if (!text.starts_with(prefix)) return false;
    if (text.size() == prefix.size()) return true;
    assert(text.size() > prefix.size());
    return !clang::isLowercase(text[prefix.size()]);
  };

  if (false) /*for #define purposes*/;
#define OBJC_SELECTOR_FAMILY(LABEL, PREFIX) \
  else if (hasPrefix(text, PREFIX)) return ObjCSelectorFamily::LABEL;
#include "language/AST/ObjCSelectorFamily.def"
  else return ObjCSelectorFamily::None;
}

StringRef ObjCSelector::getString(toolchain::SmallVectorImpl<char> &scratch) const {
  // Fast path for zero-argument selectors.
  if (getNumArgs() == 0) {
    auto name = getSelectorPieces()[0];
    if (name.empty())
      return "";
    return name.str();
  }

  scratch.clear();
  toolchain::raw_svector_ostream os(scratch);
  os << *this;
  return os.str();
}

void ObjCSelector::dump() const {
  toolchain::errs() << *this << "\n";
}


