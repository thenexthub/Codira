//===--- Stack.h - Utilities for dealing with stack space -------*- C++ -*-===//
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
/// Defines utilities for dealing with stack allocation and stack space.
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_BASIC_STACK_H
#define LANGUAGE_CORE_BASIC_STACK_H

#include <cstddef>

#include "toolchain/ADT/STLExtras.h"
#include "toolchain/Support/Compiler.h"

namespace language::Core {
  /// The amount of stack space that Clang would like to be provided with.
  /// If less than this much is available, we may be unable to reach our
  /// template instantiation depth limit and other similar limits.
  constexpr size_t DesiredStackSize = 8 << 20;

  /// Call this once on each thread, as soon after starting the thread as
  /// feasible, to note the approximate address of the bottom of the stack.
  ///
  /// \param ForceSet set to true if you know the call is near the bottom of a
  ///                 new stack. Used for split stacks.
  void noteBottomOfStack(bool ForceSet = false);

  /// Determine whether the stack is nearly exhausted.
  bool isStackNearlyExhausted();

  void runWithSufficientStackSpaceSlow(toolchain::function_ref<void()> Diag,
                                       toolchain::function_ref<void()> Fn);

  /// Run a given function on a stack with "sufficient" space. If stack space
  /// is insufficient, calls Diag to emit a diagnostic before calling Fn.
  inline void runWithSufficientStackSpace(toolchain::function_ref<void()> Diag,
                                          toolchain::function_ref<void()> Fn) {
#if LLVM_ENABLE_THREADS
    if (LLVM_UNLIKELY(isStackNearlyExhausted()))
      runWithSufficientStackSpaceSlow(Diag, Fn);
    else
      Fn();
#else
    if (LLVM_UNLIKELY(isStackNearlyExhausted()))
      Diag();
    Fn();
#endif
  }
} // end namespace language::Core

#endif // LANGUAGE_CORE_BASIC_STACK_H
