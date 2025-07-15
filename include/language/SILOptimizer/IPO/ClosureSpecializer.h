//===-------------------------- ClosureSpecializer.h ------------------------------===//
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

//===-----------------------------------------------------------------------------===//
#ifndef LANGUAGE_SILOPTIMIZER_CLOSURESPECIALIZER_H
#define LANGUAGE_SILOPTIMIZER_CLOSURESPECIALIZER_H

#include "language/SIL/SILFunction.h"

namespace language {

/// If \p function is a function-signature specialization for a constant-
/// propagated function argument, returns 1.
/// If \p function is a specialization of such a specialization, returns 2.
/// And so on.
int getSpecializationLevel(SILFunction *f);

enum class AutoDiffFunctionComponent : char { JVP = 'f', VJP = 'r' };

/// Returns true if the function is the JVP or the VJP corresponding to
/// a differentiable function.
bool isDifferentiableFuncComponent(
    SILFunction *f,
    AutoDiffFunctionComponent component = AutoDiffFunctionComponent::VJP);

} // namespace language
#endif