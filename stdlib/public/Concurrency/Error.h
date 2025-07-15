//===--- Error.h - Codira Concurrency error helpers --------------*- C++ -*-===//
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

#ifndef LANGUAGE_CONCURRENCY_ERRORS_H
#define LANGUAGE_CONCURRENCY_ERRORS_H

#include "language/Basic/Compiler.h"

#include "language/shims/Visibility.h"
#include <cstdarg>
#include <cstdint>
#include <cstdlib>

namespace language {

LANGUAGE_NORETURN LANGUAGE_FORMAT(2, 3) void language_Concurrency_fatalError(
    uint32_t flags, const char *format, ...);
LANGUAGE_NORETURN LANGUAGE_VFORMAT(2) void language_Concurrency_fatalErrorv(
    uint32_t flags, const char *format, va_list val);

} // namespace language

#endif
