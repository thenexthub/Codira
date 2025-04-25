//===--- Transforms.h -----------------------------------------------------===//
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
/// Top level transforms for SemanticARCOpts
///
//===----------------------------------------------------------------------===//

#ifndef SWIFT_SILOPTIMIZER_SEMANTICARCOPT_TRANSFORMS_H
#define SWIFT_SILOPTIMIZER_SEMANTICARCOPT_TRANSFORMS_H

#include "llvm/Support/Compiler.h"

namespace language {
namespace semanticarc {

struct Context;

/// Given the current map of owned phi arguments to consumed incoming values in
/// ctx, attempt to convert these owned phi arguments to guaranteed phi
/// arguments if the phi arguments are the only thing that kept us from
/// converting these incoming values to be guaranteed.
///
/// \returns true if we converted atleast one phi from owned -> guaranteed and
/// eliminated ARC traffic as a result.
LLVM_LIBRARY_VISIBILITY bool tryConvertOwnedPhisToGuaranteedPhis(Context &ctx);

} // namespace semanticarc
} // namespace language

#endif
