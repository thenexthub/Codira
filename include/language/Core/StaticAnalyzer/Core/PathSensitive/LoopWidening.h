//===--- LoopWidening.h - Widen loops ---------------------------*- C++ -*-===//
//
// Copyright (c) 2025, NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
// 
// Author: Tunjay Akbarli
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
// 
//     http://www.apache.org/licenses/LICENSE-2.0
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
/// \file
/// This header contains the declarations of functions which are used to widen
/// loops which do not otherwise exit. The widening is done by invalidating
/// anything which might be modified by the body of the loop.
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_STATICANALYZER_CORE_PATHSENSITIVE_LOOPWIDENING_H
#define LANGUAGE_CORE_STATICANALYZER_CORE_PATHSENSITIVE_LOOPWIDENING_H

#include "language/Core/Analysis/CFG.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/ProgramState.h"

namespace language::Core {
namespace ento {

/// Get the states that result from widening the loop.
///
/// Widen the loop by invalidating anything that might be modified
/// by the loop body in any iteration.
ProgramStateRef getWidenedLoopState(ProgramStateRef PrevState,
                                    const LocationContext *LCtx,
                                    unsigned BlockCount,
                                    ConstCFGElementRef Elem);

} // end namespace ento
} // end namespace language::Core

#endif
