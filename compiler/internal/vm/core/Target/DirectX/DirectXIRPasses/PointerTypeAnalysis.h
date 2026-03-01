//===- Target/DirectX/PointerTypeAnalysis.h - PointerType analysis --------===//
//
// Copyright (c) NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
// http://www.apache.org/licenses/LICENSE-2.0
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
// Analysis pass to assign types to opaque pointers.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TARGET_DIRECTX_POINTERTYPEANALYSIS_H
#define LLVM_TARGET_DIRECTX_POINTERTYPEANALYSIS_H

#include "vm/core/ADT/DenseMap.h"
#include "vm/core/IR/PassManager.h"
#include "vm/core/IR/TypedPointerType.h"
#include "vm/core/IR/Value.h"

namespace vm::core {

namespace dxil {

// Store the underlying type and the number of pointer indirections
using PointerTypeMap = DenseMap<const Value *, Type *>;

/// An analysis to compute the \c PointerTypes for pointers in a \c Module.
/// Since this analysis is only run during codegen and the new pass manager
/// doesn't support codegen passes, this is wrtten as a function in a namespace.
/// It is very simple to transform it into a proper analysis pass.
/// This code relies on typed pointers existing as LLVM types, but could be
/// migrated to a custom Type if PointerType loses typed support.
namespace PointerTypeAnalysis {

/// Compute the \c PointerTypeMap for the module \c M.
PointerTypeMap run(const Module &M);
} // namespace PointerTypeAnalysis

} // namespace dxil

} // namespace vm::core

#endif // LLVM_TARGET_DIRECTX_POINTERTYPEANALYSIS_H
