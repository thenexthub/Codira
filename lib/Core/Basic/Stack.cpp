//===--- Stack.cpp - Utilities for dealing with stack space ---------------===//
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

#include "language/Core/Basic/Stack.h"
#include "toolchain/Support/CrashRecoveryContext.h"
#include "toolchain/Support/ProgramStack.h"

static LLVM_THREAD_LOCAL uintptr_t BottomOfStack = 0;

void language::Core::noteBottomOfStack(bool ForceSet) {
  if (!BottomOfStack || ForceSet)
    BottomOfStack = toolchain::getStackPointer();
}

bool language::Core::isStackNearlyExhausted() {
  // We consider 256 KiB to be sufficient for any code that runs between checks
  // for stack size.
  constexpr size_t SufficientStack = 256 << 10;

  // If we don't know where the bottom of the stack is, hope for the best.
  if (!BottomOfStack)
    return false;

  intptr_t StackDiff =
      (intptr_t)toolchain::getStackPointer() - (intptr_t)BottomOfStack;
  size_t StackUsage = (size_t)std::abs(StackDiff);

  // If the stack pointer has a surprising value, we do not understand this
  // stack usage scheme. (Perhaps the target allocates new stack regions on
  // demand for us.) Don't try to guess what's going on.
  if (StackUsage > DesiredStackSize)
    return false;

  return StackUsage >= DesiredStackSize - SufficientStack;
}

void language::Core::runWithSufficientStackSpaceSlow(toolchain::function_ref<void()> Diag,
                                            toolchain::function_ref<void()> Fn) {
  toolchain::CrashRecoveryContext CRC;
  // Preserve the BottomOfStack in case RunSafelyOnNewStack uses split stacks.
  uintptr_t PrevBottom = BottomOfStack;
  CRC.RunSafelyOnNewStack([&] {
    noteBottomOfStack(true);
    Diag();
    Fn();
  }, DesiredStackSize);
  BottomOfStack = PrevBottom;
}
