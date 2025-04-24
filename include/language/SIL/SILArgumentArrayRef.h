//===--- SILArgumentArrayRef.h --------------------------------------------===//
//
// This source file is part of the Swift.org open source project
//
// Copyright (c) 2014 - 2018 Apple Inc. and the Swift project authors
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See https://swift.org/LICENSE.txt for license information
// See https://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
//
//===----------------------------------------------------------------------===//
///
/// \file
///
/// A header file to ensure that we do not create a dependency cycle in between
/// SILBasicBlock.h and SILInstruction.h.
///
//===----------------------------------------------------------------------===//

#ifndef SWIFT_SIL_SILARGUMENTARRAYREF_H
#define SWIFT_SIL_SILARGUMENTARRAYREF_H

#include "language/Basic/LLVM.h"
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
