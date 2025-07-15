//===--- AddressUseKind.h -------------------------------------------------===//
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

#ifndef LANGUAGE_SIL_ADDRESSUSEKIND_H
#define LANGUAGE_SIL_ADDRESSUSEKIND_H

namespace language {

enum class AddressUseKind { NonEscaping, Dependent, PointerEscape, Unknown };

inline AddressUseKind meet(AddressUseKind lhs, AddressUseKind rhs) {
  return (lhs > rhs) ? lhs : rhs;
}

} // namespace language

#endif
