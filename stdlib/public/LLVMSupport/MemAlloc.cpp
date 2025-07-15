//===- MemAlloc.cpp - Memory allocation functions -------------------------===//
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

#include "toolchain/Support/MemAlloc.h"

// These are out of line to have __cpp_aligned_new not affect ABI.

TOOLCHAIN_ATTRIBUTE_RETURNS_NONNULL TOOLCHAIN_ATTRIBUTE_RETURNS_NOALIAS void *
toolchain::allocate_buffer(size_t Size, size_t Alignment) {
  return ::operator new(Size
#ifdef __cpp_aligned_new
                        ,
                        std::align_val_t(Alignment)
#endif
  );
}

void toolchain::deallocate_buffer(void *Ptr, size_t Size, size_t Alignment) {
  ::operator delete(Ptr
#ifdef __cpp_sized_deallocation
                    ,
                    Size
#endif
#ifdef __cpp_aligned_new
                    ,
                    std::align_val_t(Alignment)
#endif
  );
}
