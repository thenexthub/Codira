//=== Move.h - Tracking moved-from objects. ------------------------*- C++ -*-//
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
//
// Defines inter-checker API for the use-after-move checker. It allows
// dependent checkers to figure out if an object is in a moved-from state.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_LIB_STATICANALYZER_CHECKERS_MOVE_H
#define LANGUAGE_CORE_LIB_STATICANALYZER_CHECKERS_MOVE_H

#include "language/Core/StaticAnalyzer/Core/PathSensitive/ProgramState.h"

namespace language::Core {
namespace ento {
namespace move {

/// Returns true if the object is known to have been recently std::moved.
bool isMovedFrom(ProgramStateRef State, const MemRegion *Region);

} // namespace move
} // namespace ento
} // namespace language::Core

#endif // LANGUAGE_CORE_LIB_STATICANALYZER_CHECKERS_MOVE_H
