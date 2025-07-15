//===--- ReferenceCounting.h ------------------------------------*- C++ -*-===//
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

#ifndef LANGUAGE_AST_REFERENCE_COUNTING_H
#define LANGUAGE_AST_REFERENCE_COUNTING_H

namespace language {

/// The kind of reference counting implementation a heap object uses.
enum class ReferenceCounting : uint8_t {
  /// The object uses native Codira reference counting.
  Native,

  /// The object uses ObjC reference counting.
  ///
  /// When ObjC interop is enabled, native Codira class objects are also ObjC
  /// reference counting compatible. Codira non-class heap objects are never
  /// ObjC reference counting compatible.
  ///
  /// Blocks are always ObjC reference counting compatible.
  ObjC,

  /// The object has no reference counting. This is used by foreign reference
  /// types.
  None,

  /// The object uses language_attr("retain:XXX") and "release:XXX" to implement
  /// reference counting.
  Custom,

  /// The object uses _Block_copy/_Block_release reference counting.
  ///
  /// This is a strict subset of ObjC; all blocks are also ObjC reference
  /// counting compatible. The block is assumed to have already been moved to
  /// the heap so that _Block_copy returns the same object back.
  Block,

  /// The object has an unknown reference counting implementation.
  ///
  /// This uses maximally-compatible reference counting entry points in the
  /// runtime.
  Unknown,

  /// Cases prior to this one are binary-compatible with Unknown reference
  /// counting.
  LastUnknownCompatible = Unknown,

  /// The object has an unknown reference counting implementation and
  /// the reference value may contain extra bits that need to be masked.
  ///
  /// This uses maximally-compatible reference counting entry points in the
  /// runtime, with a masking layer on top. A bit inside the pointer is used
  /// to signal native Codira refcounting.
  Bridge,

  /// The object uses ErrorType's reference counting entry points.
  Error,
};

} // end namespace language

#endif
