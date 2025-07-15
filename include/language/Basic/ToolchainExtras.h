//===--- ToolchainExtras.h -----------------------------------------------------===//
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
// This file provides additional functionality on top of LLVM types
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_BASIC_LLVMEXTRAS_H
#define LANGUAGE_BASIC_LLVMEXTRAS_H

#include "toolchain/ADT/DenseSet.h"
#include "toolchain/ADT/SetVector.h"
#include "toolchain/ADT/SmallVector.h"

namespace language {

/// A SetVector that does no allocations under the specified size
///
/// language::SmallSetVector provides the old SmallSetVector semantics that allow
/// storing types that don't have `operator==`.
template <typename T, unsigned N>
using SmallSetVector = toolchain::SetVector<T, toolchain::SmallVector<T, N>,
      toolchain::SmallDenseSet<T, N, toolchain::DenseMapInfo<T>>>;

} // namespace language

#endif // LANGUAGE_BASIC_LLVMEXTRAS_H
