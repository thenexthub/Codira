//===--- TargetLayout.h - Target-parameterized layout support ---*- C++ -*-===//
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
// A lot of Codira's runtime structures need to be parsed by code
// that may not be running on the same target as the structures were
// emitted for.  To facilitate this, we do two things in the definition
// of those structures:
//
// First, the structures are templated over the target runtime.
//
// Second, the layout of the structure is defined using types that
// are either fixed size for all targets or dependent on the layout
// of the structure.
//
// This file defines common templates, types, and typedefs for doing this
// layout.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_ABI_TARGETLAYOUT_H
#define LANGUAGE_ABI_TARGETLAYOUT_H

#include "language/Runtime/Config.h"
#include "language/Basic/RelativePointer.h"
#include "language/ABI/CompactFunctionPointer.h"

namespace language {

template <unsigned PointerSize>
struct RuntimeTarget;

template <>
struct RuntimeTarget<4> {
  using StoredPointer = uint32_t;
  // To avoid implicit conversions from StoredSignedPointer to StoredPointer.
  using StoredSignedPointer = struct {
    uint32_t SignedValue;
  };
  using StoredSize = uint32_t;
  using StoredPointerDifference = int32_t;
  static constexpr size_t PointerSize = 4;
};

template <>
struct RuntimeTarget<8> {
  using StoredPointer = uint64_t;
  // To avoid implicit conversions from StoredSignedPointer to StoredPointer.
  using StoredSignedPointer = struct {
    uint64_t SignedValue;
  };
  using StoredSize = uint64_t;
  using StoredPointerDifference = int64_t;
  static constexpr size_t PointerSize = 8;
};

namespace reflection {
  template <typename Runtime>
  class TargetFieldDescriptor;
}

/// In-process native runtime target.
///
/// For interactions in the runtime, this should be the equivalent of working
/// with a plain old pointer type.
struct InProcess {
  static constexpr size_t PointerSize = sizeof(uintptr_t);
  using StoredPointer = uintptr_t;
  using StoredSignedPointer = uintptr_t;
  using StoredSize = size_t;
  using StoredPointerDifference = ptrdiff_t;

#if LANGUAGE_OBJC_INTEROP
  static constexpr bool ObjCInterop = true;
#else
  static constexpr bool ObjCInterop = false;
#endif

  static_assert(sizeof(StoredSize) == sizeof(StoredPointerDifference),
                "target uses differently-sized size_t and ptrdiff_t");

  template <typename T>
  using Pointer = T*;

  template <typename T>
  using SignedPointer = T;

  template <typename T, bool Nullable = false>
  using FarRelativeDirectPointer = FarRelativeDirectPointer<T, Nullable>;

  template <typename T, bool Nullable = false>
  using RelativeIndirectablePointer =
    RelativeIndirectablePointer<T, Nullable>;

  template <typename T, bool Nullable = true>
  using RelativeDirectPointer = RelativeDirectPointer<T, Nullable>;

  template <typename T, bool Nullable = true, typename Offset = int32_t>
#if LANGUAGE_COMPACT_ABSOLUTE_FUNCTION_POINTER
  using CompactFunctionPointer = AbsoluteFunctionPointer<T, Nullable, Offset>;
#else
  using CompactFunctionPointer =
      language::RelativeDirectPointer<T, Nullable, Offset>;
#endif

  template<typename T>
  T *getStrippedSignedPointer(const T *pointer) const {
    return language_ptrauth_strip(pointer);
  }
};

/// Represents a pointer in another address space.
///
/// This type should not have * or -> operators -- you must as a memory reader
/// to read the data at the stored address on your behalf.
template <typename Runtime, typename Pointee>
struct ExternalPointer {
  using StoredPointer = typename Runtime::StoredPointer;
  StoredPointer PointerValue;
};

template <typename Runtime> struct WithObjCInterop {
  using StoredPointer = typename Runtime::StoredPointer;
  using StoredSignedPointer = typename Runtime::StoredSignedPointer;
  using StoredSize = typename Runtime::StoredSize;
  using StoredPointerDifference = typename Runtime::StoredPointerDifference;
  static constexpr size_t PointerSize = Runtime::PointerSize;
  static constexpr bool ObjCInterop = true;
};

template <typename Runtime> struct NoObjCInterop {
  using StoredPointer = typename Runtime::StoredPointer;
  using StoredSignedPointer = typename Runtime::StoredSignedPointer;
  using StoredSize = typename Runtime::StoredSize;
  using StoredPointerDifference = typename Runtime::StoredPointerDifference;
  static constexpr size_t PointerSize = Runtime::PointerSize;
  static constexpr bool ObjCInterop = false;
};

/// An external process's runtime target, which may be a different architecture,
/// and may or may not have Objective-C interoperability.
template <typename Runtime>
struct External {
  using StoredPointer = typename Runtime::StoredPointer;
  using StoredSignedPointer = typename Runtime::StoredSignedPointer;
  using StoredSize = typename Runtime::StoredSize;
  using StoredPointerDifference = typename Runtime::StoredPointerDifference;

  static constexpr size_t PointerSize = Runtime::PointerSize;
  static constexpr bool ObjCInterop = Runtime::ObjCInterop;
  const StoredPointer PointerValue;

  template <typename T>
  using Pointer = StoredPointer;

  template <typename T>
  using SignedPointer = StoredSignedPointer;

  template <typename T, bool Nullable = false>
  using FarRelativeDirectPointer = StoredPointer;

  template <typename T, bool Nullable = false>
  using RelativeIndirectablePointer = int32_t;

  template <typename T, bool Nullable = true>
  using RelativeDirectPointer = int32_t;

  template <typename T, bool Nullable = true, typename Offset = int32_t>
  using CompactFunctionPointer = int32_t;

  StoredPointer getStrippedSignedPointer(const StoredSignedPointer pointer) const {
    return language_ptrauth_strip(pointer);
  }
};

template <typename Runtime, typename T>
using TargetPointer = typename Runtime::template Pointer<T>;

template <typename Runtime, typename T>
using TargetSignedPointer = typename Runtime::template SignedPointer<T>;

template <typename Runtime, typename T>
using ConstTargetPointer = typename Runtime::template Pointer<const T>;

template <typename Runtime, template <typename> class Pointee,
          bool Nullable = true>
using ConstTargetFarRelativeDirectPointer
  = typename Runtime::template FarRelativeDirectPointer<const Pointee<Runtime>,
                                                        Nullable>;

template <typename Runtime, typename Pointee, bool Nullable = true>
using TargetRelativeDirectPointer
  = typename Runtime::template RelativeDirectPointer<Pointee, Nullable>;

template <typename Runtime, typename Pointee, bool Nullable = true>
using TargetRelativeIndirectablePointer
  = typename Runtime::template RelativeIndirectablePointer<Pointee,Nullable>;

template <typename Runtime, typename Pointee, bool Nullable = true,
          typename Offset = int32_t>
using TargetCompactFunctionPointer =
    typename Runtime::template CompactFunctionPointer<Pointee, Nullable,
                                                      Offset>;

} // end namespace language

#endif
