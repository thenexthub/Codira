//===--- LLVMARCOpts.h - LLVM level ARC Opts Util. Declarations -*- C++ -*-===//
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
#ifndef LANGUAGE_LLVMPASSES_LLVMARCOPTS_H
#define LANGUAGE_LLVMPASSES_LLVMARCOPTS_H

#include "language/Basic/Toolchain.h"
#include "language/Runtime/Config.h"
#include "toolchain/ADT/StringSwitch.h"
#include "toolchain/IR/Function.h"
#include "toolchain/IR/Instructions.h"
#include "toolchain/IR/Intrinsics.h"

namespace language {

enum RT_Kind {
#define KIND(Name, MemBehavior) RT_ ## Name,
#include "LLVMCodira.def"
};

/// Take a look at the specified instruction and classify it into what kind of
/// runtime entrypoint it is, if any.
inline RT_Kind classifyInstruction(const toolchain::Instruction &I) {
  if (!I.mayReadOrWriteMemory())
    return RT_NoMemoryAccessed;

  // Non-calls or calls to indirect functions are unknown.
  auto *CI = dyn_cast<toolchain::CallInst>(&I);
  if (CI == 0) return RT_Unknown;

  // First check if we have an objc intrinsic.
  auto intrinsic = CI->getIntrinsicID();
  switch (intrinsic) {
  // This is an intrinsic that we do not understand. It can not be one of our
  // "special" runtime functions as well... so return RT_Unknown early.
  default:
    return RT_Unknown;
  case toolchain::Intrinsic::not_intrinsic:
    // If we do not have an intrinsic, break and move onto runtime functions
    // that we identify by name.
    break;
#define OBJC_FUNC(Name, MemBehavior, TextualName)                              \
  case toolchain::Intrinsic::objc_##TextualName:                                    \
    return RT_##Name;
#include "LLVMCodira.def"
  }

  toolchain::Function *F = CI->getCalledFunction();
  if (F == nullptr)
    return RT_Unknown;

  return toolchain::StringSwitch<RT_Kind>(F->getName())
#define LANGUAGE_FUNC(Name, MemBehavior, TextualName) \
    .Case("language_" #TextualName, RT_ ## Name)
#define LANGUAGE_INTERNAL_FUNC_NEVER_NONATOMIC(Name, MemBehavior, TextualName) \
    .Case("__language_" #TextualName, RT_ ## Name)
#include "LLVMCodira.def"

    // Support non-atomic versions of reference counting entry points.
#define LANGUAGE_FUNC(Name, MemBehavior, TextualName) \
    .Case("language_nonatomic_" #TextualName, RT_ ## Name)
#define OBJC_FUNC(Name, MemBehavior, TextualName) \
    .Case("objc_nonatomic_" #TextualName, RT_ ## Name)
#define LANGUAGE_INTERNAL_FUNC_NEVER_NONATOMIC(Name, MemBehavior, TextualName)
#include "LLVMCodira.def"

    .Default(RT_Unknown);
}

} // end namespace language
#endif
