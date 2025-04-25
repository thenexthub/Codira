//===--- Error.cpp - Error handling support code --------------------------===//
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

#include "Concurrency/Error.h"

// swift::fatalError is not exported from libswiftCore and not shared, so define another
// internal function instead.
SWIFT_NORETURN void swift::swift_Concurrency_fatalError(uint32_t flags, const char *format, ...) {
  abort();
}
