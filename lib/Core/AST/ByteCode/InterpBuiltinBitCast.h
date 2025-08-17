//===------------------ InterpBuiltinBitCast.h ------------------*- C++ -*-===//
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

#ifndef LANGUAGE_CORE_AST_INTERP_BUILTIN_BIT_CAST_H
#define LANGUAGE_CORE_AST_INTERP_BUILTIN_BIT_CAST_H

#include "BitcastBuffer.h"
#include <cstddef>

namespace language::Core {
namespace interp {
class Pointer;
class InterpState;
class CodePtr;
class Context;

inline static void swapBytes(std::byte *M, size_t N) {
  for (size_t I = 0; I != (N / 2); ++I)
    std::swap(M[I], M[N - 1 - I]);
}

bool DoBitCast(InterpState &S, CodePtr OpPC, const Pointer &Ptr,
               std::byte *Buff, Bits BitWidth, Bits FullBitWidth,
               bool &HasIndeterminateBits);
bool DoBitCastPtr(InterpState &S, CodePtr OpPC, const Pointer &FromPtr,
                  Pointer &ToPtr);
bool DoBitCastPtr(InterpState &S, CodePtr OpPC, const Pointer &FromPtr,
                  Pointer &ToPtr, size_t Size);
bool readPointerToBuffer(const Context &Ctx, const Pointer &FromPtr,
                         BitcastBuffer &Buffer, bool ReturnOnUninit);

bool DoMemcpy(InterpState &S, CodePtr OpPC, const Pointer &SrcPtr,
              const Pointer &DestPtr, Bits Size);

} // namespace interp
} // namespace language::Core

#endif
