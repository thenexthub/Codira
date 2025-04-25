//===-- LayoutConstraintKind.h - Layout constraints kinds -------*- C++ -*-===//
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
// This file defines types and APIs for layout constraints.
//
//===----------------------------------------------------------------------===//

#include <cstdint>

#ifndef SWIFT_LAYOUT_CONSTRAINTKIND_H
#define SWIFT_LAYOUT_CONSTRAINTKIND_H

namespace language {
/// Describes a layout constraint information.
enum class LayoutConstraintKind : uint8_t {
  // It is not a known layout constraint.
  UnknownLayout,
  // It is a layout constraint representing a trivial type of a known size.
  TrivialOfExactSize,
  // It is a layout constraint representing a trivial type of a size known to
  // be no larger than a given size.
  TrivialOfAtMostSize,
  // It is a layout constraint representing a trivial type of an unknown size.
  Trivial,
  // It is a layout constraint representing a reference counted class instance.
  Class,
  // It is a layout constraint representing a reference counted native class
  // instance.
  NativeClass,
  // It is a layout constraint representing a reference counted object.
  RefCountedObject,
  // It is a layout constraint representing a native reference counted object.
  NativeRefCountedObject,
  // It is a layout constraint representing a bridge object
  BridgeObject,
  // It is a layout constraint representing a trivial type of a known stride.
  TrivialStride,
  LastLayout = TrivialStride,
};
} // namespace language

#endif // SWIFT_LAYOUT_CONSTRAINTKIND_H
