//===--- CompactFunctionPointer.h - Compact Function Pointers ---*- C++ -*-===//
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
// Codira's runtime structures often use relative function pointers to reduce the
// size of metadata and also to minimize load-time overhead in PIC.
// This file defines pointer types whose size and interface are compatible with
// the relative pointer types for targets that do not support relative references
// to code from data.
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_ABI_COMPACTFUNCTIONPOINTER_H
#define LANGUAGE_ABI_COMPACTFUNCTIONPOINTER_H

namespace language {

/// A compact unconditional absolute function pointer that can fit in a 32-bit
/// integer.
/// As a trade-off compared to relative pointers, this has load-time overhead in PIC
/// and is only available on 32-bit targets.
template <typename T, bool Nullable = false, typename Offset = int32_t>
class AbsoluteFunctionPointer {
  T *Pointer;
  static_assert(sizeof(T *) == sizeof(int32_t),
                "Function pointer must be 32-bit when using compact absolute pointer");

public:
  using PointerTy = T *;

  PointerTy get() const & { return Pointer; }

  operator PointerTy() const & { return this->get(); }

  bool isNull() const & { return Pointer == nullptr; }

  /// Resolve a pointer from a `base` pointer and a value loaded from `base`.
  template <typename BasePtrTy, typename Value>
  static PointerTy resolve(BasePtrTy *base, Value value) {
    return reinterpret_cast<PointerTy>(value);
  }

  template <typename... ArgTy>
  typename std::invoke_result<T *(ArgTy...)>::type operator()(ArgTy... arg) const {
    static_assert(std::is_function<T>::value,
                  "T must be an explicit function type");
    return this->get()(std::forward<ArgTy>(arg)...);
  }
};

// TODO(katei): Add another pointer structure for 64-bit targets and for efficiency on PIC

} // namespace language

#endif // LANGUAGE_ABI_COMPACTFUNCTIONPOINTER_H
