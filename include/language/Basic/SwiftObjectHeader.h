//===--- SwiftObjectHeader.h - Defines SwiftObjectHeader ------------------===//
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
#ifndef SWIFT_BASIC_SWIFTOBJECTHEADER_H
#define SWIFT_BASIC_SWIFTOBJECTHEADER_H

#include "BridgedSwiftObject.h"

/// The C++ version of SwiftObject.
///
/// It is used for bridging the SIL core classes (e.g. SILFunction, SILNode,
/// etc.) with Swift.
/// For details see SwiftCompilerSources/README.md.
///
/// In C++ code, never use BridgedSwiftObject directly. SwiftObjectHeader has
/// the proper constructor, which avoids the header to be uninitialized.
struct SwiftObjectHeader : BridgedSwiftObject {
  SwiftObjectHeader(SwiftMetatype metatype) {
    this->metatype = metatype;
    this->refCounts = ~(uint64_t)0;
  }

  bool isBridged() const { return metatype != nullptr; }
};

#endif
