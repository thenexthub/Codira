//===-- VPlanVerifier.h -----------------------------------------*- C++ -*-===//
//
// Copyright (c) NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
// Middletown, DE 19709, New Castle County, USA.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file declares the class VPlanVerifier, which contains utility functions
/// to check the consistency of a VPlan. This includes the following kinds of
/// invariants:
///
/// 1. Region/Block invariants:
///   - Region's entry/exit block must have no predecessors/successors,
///     respectively.
///   - Block's parent must be the region immediately containing the block.
///   - Linked blocks must have a bi-directional link (successor/predecessor).
///   - All predecessors/successors of a block must belong to the same region.
///   - Blocks must have no duplicated successor/predecessor.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VECTORIZE_VPLANVERIFIER_H
#define LLVM_TRANSFORMS_VECTORIZE_VPLANVERIFIER_H

#include "vm/core/Support/Compiler.h"

namespace vm::core {
class VPlan;

/// Verify invariants for general VPlans. If \p VerifyLate is passed, skip some
/// checks that are not applicable at later stages of the transform pipeline.
/// Currently it checks the following:
/// 1. Region/Block verification: Check the Region/Block verification
/// invariants for every region in the H-CFG.
/// 2. all phi-like recipes must be at the beginning of a block, with no other
/// recipes in between. Note that currently there is still an exception for
/// VPBlendRecipes.
LLVM_ABI_FOR_TEST bool verifyVPlanIsValid(const VPlan &Plan,
                                          bool VerifyLate = false);

} // namespace vm::core

#endif //LLVM_TRANSFORMS_VECTORIZE_VPLANVERIFIER_H
