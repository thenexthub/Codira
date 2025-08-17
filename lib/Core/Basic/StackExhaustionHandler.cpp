//===--- StackExhaustionHandler.cpp -  - A utility for warning once when close
// to out of stack space -------*- C++ -*-===//
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

#include "language/Core/Basic/StackExhaustionHandler.h"
#include "language/Core/Basic/Stack.h"

void language::Core::StackExhaustionHandler::runWithSufficientStackSpace(
    SourceLocation Loc, toolchain::function_ref<void()> Fn) {
  language::Core::runWithSufficientStackSpace([&] { warnStackExhausted(Loc); }, Fn);
}

void language::Core::StackExhaustionHandler::warnOnStackNearlyExhausted(
    SourceLocation Loc) {
  if (isStackNearlyExhausted())
    warnStackExhausted(Loc);
}

void language::Core::StackExhaustionHandler::warnStackExhausted(SourceLocation Loc) {
  // Only warn about this once.
  if (!WarnedStackExhausted) {
    DiagsRef.Report(Loc, diag::warn_stack_exhausted);
    WarnedStackExhausted = true;
  }
}
