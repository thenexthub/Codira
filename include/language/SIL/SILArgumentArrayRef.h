//===--- SILArgumentArrayRef.h --------------------------------------------===//
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
///
/// A header file to ensure that we do not create a dependency cycle in between
/// SILBasicBlock.h and SILInstruction.h.
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_SIL_SILARGUMENTARRAYREF_H
#define LANGUAGE_SIL_SILARGUMENTARRAYREF_H

#include "language/Basic/Toolchain.h"
#include "language/Basic/STLExtras.h"

namespace language {

class SILArgument;

#define ARGUMENT(NAME, PARENT)                                                 \
  class NAME;                                                                  \
  using NAME##ArrayRef =                                                       \
      TransformRange<ArrayRef<SILArgument *>, NAME *(*)(SILArgument *)>;
#include "language/SIL/SILNodes.def"

} // namespace language

#endif
