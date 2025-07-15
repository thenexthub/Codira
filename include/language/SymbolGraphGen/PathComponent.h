//===--- SymbolGraphPathComponent.h - Codira SymbolGraph Path Component ----===//
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

#ifndef LANGUAGE_SYMBOLGRAPHGEN_PATHCOMPONENT_H
#define LANGUAGE_SYMBOLGRAPHGEN_PATHCOMPONENT_H

#include "toolchain/ADT/SmallString.h"

namespace language {
class ValueDecl;

namespace symbolgraphgen {

/// Summary information for a node along a path through a symbol graph.
struct PathComponent {
  /// The title of the corresponding symbol graph node.
  SmallString<32> Title;
  /// The kind of the corresponding symbol graph node.
  StringRef Kind;
  /// The language decl associated with the corresponding symbol graph node.
  const ValueDecl *VD;
};

} // end namespace symbolgraphgen
} // end namespace language

#endif // LANGUAGE_SYMBOLGRAPHGEN_PATHCOMPONENT_H
