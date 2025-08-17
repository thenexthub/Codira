//===--- CIRGenTypeCache.h - Commonly used LLVM types and info -*- C++ --*-===//
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
// This structure provides a set of common types useful during CIR emission.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_LIB_CIR_CIRGENTYPECACHE_H
#define LANGUAGE_CORE_LIB_CIR_CIRGENTYPECACHE_H

#include "language/Core/AST/CharUnits.h"
#include "language/Core/CIR/Dialect/IR/CIRTypes.h"

namespace language::Core::CIRGen {

/// This structure provides a set of types that are commonly used
/// during IR emission. It's initialized once in CodeGenModule's
/// constructor and then copied around into new CIRGenFunction's.
struct CIRGenTypeCache {
  CIRGenTypeCache() {}

  // ClangIR void type
  cir::VoidType VoidTy;

  // ClangIR signed integral types of common sizes
  cir::IntType SInt8Ty;
  cir::IntType SInt16Ty;
  cir::IntType SInt32Ty;
  cir::IntType SInt64Ty;
  cir::IntType SInt128Ty;

  // ClangIR unsigned integral type of common sizes
  cir::IntType UInt8Ty;
  cir::IntType UInt16Ty;
  cir::IntType UInt32Ty;
  cir::IntType UInt64Ty;
  cir::IntType UInt128Ty;

  // ClangIR floating-point types with fixed formats
  cir::FP16Type FP16Ty;
  cir::BF16Type BFloat16Ty;
  cir::SingleType FloatTy;
  cir::DoubleType DoubleTy;
  cir::FP80Type FP80Ty;
  cir::FP128Type FP128Ty;

  /// intptr_t, size_t, and ptrdiff_t, which we assume are the same size.
  union {
    mlir::Type UIntPtrTy;
    mlir::Type SizeTy;
  };

  mlir::Type PtrDiffTy;

  /// void* in address space 0
  cir::PointerType VoidPtrTy;
  cir::PointerType UInt8PtrTy;

  /// The size and alignment of a pointer into the generic address space.
  union {
    unsigned char PointerAlignInBytes;
    unsigned char PointerSizeInBytes;
  };

  /// The alignment of size_t.
  unsigned char SizeAlignInBytes;

  language::Core::CharUnits getSizeAlign() const {
    return language::Core::CharUnits::fromQuantity(SizeAlignInBytes);
  }

  language::Core::CharUnits getPointerAlign() const {
    return language::Core::CharUnits::fromQuantity(PointerAlignInBytes);
  }
};

} // namespace language::Core::CIRGen

#endif // LANGUAGE_CORE_LIB_CIR_CODEGEN_CIRGENTYPECACHE_H
