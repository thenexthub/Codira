//===--- Markup.h - Markup --------------------------------------*- C++ -*-===//
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

#ifndef SWIFT_MARKUP_MARKUP_H
#define SWIFT_MARKUP_MARKUP_H

#include "llvm/ADT/ArrayRef.h"
#include "llvm/Support/Allocator.h"
#include "llvm/Support/raw_ostream.h"
#include "language/Basic/SourceLoc.h"
#include "language/Markup/AST.h"
#include "language/Markup/LineList.h"

namespace language {

struct RawComment;

namespace markup {

class LineList;

class MarkupContext final {
  llvm::BumpPtrAllocator Allocator;

public:
  void *allocate(unsigned long Bytes, unsigned Alignment) {
    return Allocator.Allocate(Bytes, Alignment);
  }

  template <typename T, typename It>
  T *allocateCopy(It Begin, It End) {
    T *Res =
    static_cast<T *>(allocate(sizeof(T) * (End - Begin), alignof(T)));
    for (unsigned i = 0; Begin != End; ++Begin, ++i)
      new (Res + i) T(*Begin);
    return Res;
  }

  template <typename T>
  MutableArrayRef<T> allocateCopy(ArrayRef<T> Array) {
    return MutableArrayRef<T>(allocateCopy<T>(Array.begin(), Array.end()),
                              Array.size());
  }

  StringRef allocateCopy(StringRef Str) {
    ArrayRef<char> Result =
        allocateCopy(llvm::ArrayRef(Str.data(), Str.size()));
    return StringRef(Result.data(), Result.size());
  }

  LineList getLineList(swift::RawComment RC);
};

Document *parseDocument(MarkupContext &MC, StringRef String);
Document *parseDocument(MarkupContext &MC, LineList &LL);

} // namespace markup
} // namespace language

#endif // SWIFT_MARKUP_MARKUP_H
