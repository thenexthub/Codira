//===--- Error.h - Swift Concurrency error helpers --------------*- C++ -*-===//
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
// Error handling support.
//
//===----------------------------------------------------------------------===//

#ifndef SWIFT_CONCURRENCY_ERRORS_H
#define SWIFT_CONCURRENCY_ERRORS_H

#include "language/Basic/Compiler.h"

#include "language/shims/Visibility.h"
#include <cstdarg>
#include <cstdint>
#include <cstdlib>

namespace language {

SWIFT_NORETURN SWIFT_FORMAT(2, 3) void swift_Concurrency_fatalError(
    uint32_t flags, const char *format, ...);
SWIFT_NORETURN SWIFT_VFORMAT(2) void swift_Concurrency_fatalErrorv(
    uint32_t flags, const char *format, va_list val);

} // namespace language

#endif
