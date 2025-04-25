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
#ifndef SWIFT_RUNTIME_RUNTIMEFNWRAPPERSGEN_H
#define SWIFT_RUNTIME_RUNTIMEFNWRAPPERSGEN_H

#include "language/SIL/RuntimeEffect.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/IR/Module.h"

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

/// Generate an llvm declaration for a runtime entry with a
/// given name, return types, argument types, attributes and
/// a calling convention.
llvm::Constant *getRuntimeFn(llvm::Module &Module, llvm::Constant *&cache,
                             const char *ModuleName, char const *FunctionName,
                             llvm::CallingConv::ID cc,
                             RuntimeAvailability availability,
                             llvm::ArrayRef<llvm::Type *> retTypes,
                             llvm::ArrayRef<llvm::Type *> argTypes,
                             llvm::ArrayRef<llvm::Attribute::AttrKind> attrs,
                             llvm::ArrayRef<llvm::MemoryEffects> memEffects,
                             irgen::IRGenModule *IGM = nullptr);

llvm::FunctionType *getRuntimeFnType(llvm::Module &Module,
                             llvm::ArrayRef<llvm::Type *> retTypes,
                             llvm::ArrayRef<llvm::Type *> argTypes);

} // namespace language
#endif
