//===--- CodeGenTypeCache.h - Commonly used LLVM types and info -*- C++ -*-===//
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
// This structure provides a set of common types useful during IR emission.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_LIB_CODEGEN_CODEGENTYPECACHE_H
#define LANGUAGE_CORE_LIB_CODEGEN_CODEGENTYPECACHE_H

#include "language/Core/AST/CharUnits.h"
#include "language/Core/Basic/AddressSpaces.h"
#include "toolchain/IR/CallingConv.h"

namespace toolchain {
  class Type;
  class IntegerType;
  class PointerType;
}

namespace language::Core {
namespace CodeGen {

/// This structure provides a set of types that are commonly used
/// during IR emission.  It's initialized once in CodeGenModule's
/// constructor and then copied around into new CodeGenFunctions.
struct CodeGenTypeCache {
  /// void
  toolchain::Type *VoidTy;

  /// i8, i16, i32, and i64
  toolchain::IntegerType *Int8Ty, *Int16Ty, *Int32Ty, *Int64Ty;
  /// half, bfloat, float, double
  toolchain::Type *HalfTy, *BFloatTy, *FloatTy, *DoubleTy;

  /// int
  toolchain::IntegerType *IntTy;

  /// char
  toolchain::IntegerType *CharTy;

  /// intptr_t, size_t, and ptrdiff_t, which we assume are the same size.
  union {
    toolchain::IntegerType *IntPtrTy;
    toolchain::IntegerType *SizeTy;
    toolchain::IntegerType *PtrDiffTy;
  };

  /// void*, void** in the target's default address space (often 0)
  union {
    toolchain::PointerType *UnqualPtrTy;
    toolchain::PointerType *VoidPtrTy;
    toolchain::PointerType *Int8PtrTy;
    toolchain::PointerType *VoidPtrPtrTy;
    toolchain::PointerType *Int8PtrPtrTy;
  };

  /// void* in alloca address space
  union {
    toolchain::PointerType *AllocaVoidPtrTy;
    toolchain::PointerType *AllocaInt8PtrTy;
  };

  /// void* in default globals address space
  union {
    toolchain::PointerType *GlobalsVoidPtrTy;
    toolchain::PointerType *GlobalsInt8PtrTy;
  };

  /// void* in the address space for constant globals
  toolchain::PointerType *ConstGlobalsPtrTy;

  /// The size and alignment of the builtin C type 'int'.  This comes
  /// up enough in various ABI lowering tasks to be worth pre-computing.
  union {
    unsigned char IntSizeInBytes;
    unsigned char IntAlignInBytes;
  };
  CharUnits getIntSize() const {
    return CharUnits::fromQuantity(IntSizeInBytes);
  }
  CharUnits getIntAlign() const {
    return CharUnits::fromQuantity(IntAlignInBytes);
  }

  /// The width of a pointer into the generic address space.
  unsigned char PointerWidthInBits;

  /// The size and alignment of a pointer into the generic address space.
  union {
    unsigned char PointerAlignInBytes;
    unsigned char PointerSizeInBytes;
  };

  /// The size and alignment of size_t.
  union {
    unsigned char SizeSizeInBytes; // sizeof(size_t)
    unsigned char SizeAlignInBytes;
  };

  LangAS ASTAllocaAddressSpace;

  CharUnits getSizeSize() const {
    return CharUnits::fromQuantity(SizeSizeInBytes);
  }
  CharUnits getSizeAlign() const {
    return CharUnits::fromQuantity(SizeAlignInBytes);
  }
  CharUnits getPointerSize() const {
    return CharUnits::fromQuantity(PointerSizeInBytes);
  }
  CharUnits getPointerAlign() const {
    return CharUnits::fromQuantity(PointerAlignInBytes);
  }

  toolchain::CallingConv::ID RuntimeCC;
  toolchain::CallingConv::ID getRuntimeCC() const { return RuntimeCC; }

  LangAS getASTAllocaAddressSpace() const { return ASTAllocaAddressSpace; }
};

}  // end namespace CodeGen
}  // end namespace language::Core

#endif
