//===--- StackExhaustionHandler.h - A utility for warning once when close to out
// of stack space -------*- C++ -*-===//
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
///
/// \file
/// Defines a utilitiy for warning once when close to out of stack space.
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_BASIC_STACK_EXHAUSTION_HANDLER_H
#define LANGUAGE_CORE_BASIC_STACK_EXHAUSTION_HANDLER_H

#include "language/Core/Basic/Diagnostic.h"

namespace language::Core {
class StackExhaustionHandler {
public:
  StackExhaustionHandler(DiagnosticsEngine &diags) : DiagsRef(diags) {}

  /// Run some code with "sufficient" stack space. (Currently, at least 256K
  /// is guaranteed). Produces a warning if we're low on stack space and
  /// allocates more in that case. Use this in code that may recurse deeply to
  /// avoid stack overflow.
  void runWithSufficientStackSpace(SourceLocation Loc,
                                   toolchain::function_ref<void()> Fn);

  /// Check to see if we're low on stack space and produce a warning if we're
  /// low on stack space (Currently, at least 256Kis guaranteed).
  void warnOnStackNearlyExhausted(SourceLocation Loc);

private:
  /// Warn that the stack is nearly exhausted.
  void warnStackExhausted(SourceLocation Loc);

  DiagnosticsEngine &DiagsRef;
  bool WarnedStackExhausted = false;
};
} // end namespace language::Core

#endif // LANGUAGE_CORE_BASIC_STACK_EXHAUSTION_HANDLER_H
