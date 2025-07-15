//===--- RuntimeFnWrappersGen.h - LLVM IR Generation for runtime functions ===//
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
//  Helper functions providing the LLVM IR generation for runtime entry points.
//
//===----------------------------------------------------------------------===//
#ifndef LANGUAGE_RUNTIME_RUNTIMEFNWRAPPERSGEN_H
#define LANGUAGE_RUNTIME_RUNTIMEFNWRAPPERSGEN_H

#include "language/SIL/RuntimeEffect.h"
#include "toolchain/ADT/ArrayRef.h"
#include "toolchain/IR/Module.h"

namespace language {

class AvailabilityRange;
class ASTContext;

namespace irgen {
class IRGenModule;
}

enum class RuntimeAvailability {
  AlwaysAvailable,
  AvailableByCompatibilityLibrary,
  ConditionallyAvailable
};

/// Generate an toolchain declaration for a runtime entry with a
/// given name, return types, argument types, attributes and
/// a calling convention.
toolchain::Constant *getRuntimeFn(toolchain::Module &Module, toolchain::Constant *&cache,
                             const char *ModuleName, char const *FunctionName,
                             toolchain::CallingConv::ID cc,
                             RuntimeAvailability availability,
                             toolchain::ArrayRef<toolchain::Type *> retTypes,
                             toolchain::ArrayRef<toolchain::Type *> argTypes,
                             toolchain::ArrayRef<toolchain::Attribute::AttrKind> attrs,
                             toolchain::ArrayRef<toolchain::MemoryEffects> memEffects,
                             irgen::IRGenModule *IGM = nullptr);

toolchain::FunctionType *getRuntimeFnType(toolchain::Module &Module,
                             toolchain::ArrayRef<toolchain::Type *> retTypes,
                             toolchain::ArrayRef<toolchain::Type *> argTypes);

} // namespace language
#endif
