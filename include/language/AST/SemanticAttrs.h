//===--- Semantics.h - Semantics Attribute Definitions -------------*- C++ -*-===//
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
///
/// \file
/// Implementation of the matching definition file.
/// This file holds all semantics attributes as constant string literals.
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_SEMANTICS_H
#define LANGUAGE_SEMANTICS_H

#include "language/Basic/Toolchain.h"
#include "toolchain/ADT/StringRef.h"

namespace language {
namespace semantics {
#define SEMANTICS_ATTR(NAME, C_STR) constexpr static const StringLiteral NAME = C_STR;
#include "SemanticAttrs.def"
}
}

#endif
