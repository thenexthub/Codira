//===--- APNumericStorage.h - Store APInt/APFloat in ASTContext -*- C++ -*-===//
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

#ifndef LANGUAGE_CORE_AST_APNUMERICSTORAGE_H
#define LANGUAGE_CORE_AST_APNUMERICSTORAGE_H

#include "toolchain/ADT/APFloat.h"
#include "toolchain/ADT/APInt.h"

namespace language::Core {
class ASTContext;

/// Used by IntegerLiteral/FloatingLiteral/EnumConstantDecl to store the
/// numeric without leaking memory.
///
/// For large floats/integers, APFloat/APInt will allocate memory from the heap
/// to represent these numbers.  Unfortunately, when we use a BumpPtrAllocator
/// to allocate IntegerLiteral/FloatingLiteral nodes the memory associated with
/// the APFloat/APInt values will never get freed. APNumericStorage uses
/// ASTContext's allocator for memory allocation.
class APNumericStorage {
  union {
    uint64_t VAL;   ///< Used to store the <= 64 bits integer value.
    uint64_t *pVal; ///< Used to store the >64 bits integer value.
  };
  unsigned BitWidth;

  bool hasAllocation() const { return toolchain::APInt::getNumWords(BitWidth) > 1; }

  APNumericStorage(const APNumericStorage &) = delete;
  void operator=(const APNumericStorage &) = delete;

protected:
  APNumericStorage() : VAL(0), BitWidth(0) {}

  toolchain::APInt getIntValue() const {
    unsigned NumWords = toolchain::APInt::getNumWords(BitWidth);
    if (NumWords > 1)
      return toolchain::APInt(BitWidth, NumWords, pVal);
    else
      return toolchain::APInt(BitWidth, VAL);
  }
  void setIntValue(const ASTContext &C, const toolchain::APInt &Val);
};

class APIntStorage : private APNumericStorage {
public:
  toolchain::APInt getValue() const { return getIntValue(); }
  void setValue(const ASTContext &C, const toolchain::APInt &Val) {
    setIntValue(C, Val);
  }
};

class APFloatStorage : private APNumericStorage {
public:
  toolchain::APFloat getValue(const toolchain::fltSemantics &Semantics) const {
    return toolchain::APFloat(Semantics, getIntValue());
  }
  void setValue(const ASTContext &C, const toolchain::APFloat &Val) {
    setIntValue(C, Val.bitcastToAPInt());
  }
};

} // end namespace language::Core

#endif // LANGUAGE_CORE_AST_APNUMERICSTORAGE_H
