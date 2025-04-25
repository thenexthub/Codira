//===--- Sanitizers.h - Helpers related to sanitizers -----------*- C++ -*-===//
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

#ifndef SWIFT_BASIC_SANITIZERS_H
#define SWIFT_BASIC_SANITIZERS_H

namespace language {

// Enabling bitwise masking.
enum class SanitizerKind : unsigned {
  #define SANITIZER(enum_bit, kind, name, file) kind = (1 << enum_bit),
  #include "Sanitizers.def"
};

} // end namespace language

#endif // SWIFT_BASIC_SANITIZERS_H
