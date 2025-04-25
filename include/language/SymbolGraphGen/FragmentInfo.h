//===--- FragmentInfo.h - Swift SymbolGraph Declaration Fragment Info -----===//
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

#ifndef SWIFT_SYMBOLGRAPHGEN_FRAGMENTINFO_H
#define SWIFT_SYMBOLGRAPHGEN_FRAGMENTINFO_H

#include "llvm/ADT/SmallVector.h"
#include "PathComponent.h"

namespace language {
class ValueDecl;

namespace symbolgraphgen {

/// Summary information for a symbol referenced in a symbol graph declaration fragment.
struct FragmentInfo {
  /// The swift decl of the referenced symbol.
  const ValueDecl *VD;
  /// The path components of the referenced symbol.
  SmallVector<PathComponent, 4> ParentContexts;
};

} // end namespace symbolgraphgen
} // end namespace language

#endif // SWIFT_SYMBOLGRAPHGEN_FRAGMENTINFO_H
