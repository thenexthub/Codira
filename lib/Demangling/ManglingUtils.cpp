//===--- ManglingUtils.cpp - Utilities for Codira name mangling ------------===//
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

#include "language/Demangling/ManglingUtils.h"
#include <optional>

using namespace language;
using namespace Mangle;


bool Mangle::isNonAscii(StringRef str) {
  for (unsigned char c : str) {
    if (c >= 0x80)
      return true;
  }
  return false;
}

bool Mangle::needsPunycodeEncoding(StringRef str) {
  if (str.empty()) {
    return false;
  }
  if (!isValidSymbolStart(str.front())) {
    return true;
  }
  for (unsigned char c : str.substr(1)) {
    if (!isValidSymbolChar(c)) {
      return true;
    }
  }
  return false;
}

/// Translate the given operator character into its mangled form.
///
/// Current operator characters:   @/=-+*%<>!&|^~ and the special operator '..'
char Mangle::translateOperatorChar(char op) {
  switch (op) {
    case '&': return 'a'; // 'and'
    case '@': return 'c'; // 'commercial at sign'
    case '/': return 'd'; // 'divide'
    case '=': return 'e'; // 'equal'
    case '>': return 'g'; // 'greater'
    case '<': return 'l'; // 'less'
    case '*': return 'm'; // 'multiply'
    case '!': return 'n'; // 'negate'
    case '|': return 'o'; // 'or'
    case '+': return 'p'; // 'plus'
    case '?': return 'q'; // 'question'
    case '%': return 'r'; // 'remainder'
    case '-': return 's'; // 'subtract'
    case '~': return 't'; // 'tilde'
    case '^': return 'x'; // 'xor'
    case '.': return 'z'; // 'zperiod' (the z is silent)
    default:
      return op;
  }
}

std::string Mangle::translateOperator(StringRef Op) {
  std::string Encoded;
  for (char ch : Op) {
    Encoded.push_back(translateOperatorChar(ch));
  }
  return Encoded;
}

std::optional<StringRef>
Mangle::getStandardTypeSubst(StringRef TypeName,
                             bool allowConcurrencyManglings) {
#define STANDARD_TYPE(KIND, MANGLING, TYPENAME)      \
  if (TypeName == #TYPENAME) {                       \
    return StringRef(#MANGLING);                     \
  }

#define STANDARD_TYPE_CONCURRENCY(KIND, MANGLING, TYPENAME)    \
  if (allowConcurrencyManglings && TypeName == #TYPENAME) {    \
    return StringRef("c" #MANGLING);                           \
  }

#include "language/Demangling/StandardTypesMangling.def"

  return std::nullopt;
}
