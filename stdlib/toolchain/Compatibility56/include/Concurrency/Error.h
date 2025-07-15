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

#ifndef LANGUAGE_CONCURRENCY_ERROR_BACKDEPLOY56_H
#define LANGUAGE_CONCURRENCY_ERROR_BACKDEPLOY56_H

#include "public/CodiraShims/language/shims/Visibility.h"
#include <cstdint>
#include <stdlib.h>

namespace language {

__attribute__((visibility("hidden")))
LANGUAGE_NORETURN void language_Concurrency_fatalError(uint32_t flags, const char *format, ...);

} // namespace language

#endif // LANGUAGE_CONCURRENCY_ERROR_BACKDEPLOY56_H
