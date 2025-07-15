//===--- SourceFileExtras.h - Extra data for a source file ------*- C++ -*-===//
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

#ifndef LANGUAGE_AST_SOURCEFILEEXTRAS_H
#define LANGUAGE_AST_SOURCEFILEEXTRAS_H

#include "toolchain/ADT/DenseMap.h"
#include <vector>

namespace language {

class Decl;

/// Extra information associated with a source file that is lazily created and
/// stored in a separately-allocated side structure.
struct SourceFileExtras {
};
  
}

#endif // LANGUAGE_AST_SOURCEFILEEXTRAS_H
