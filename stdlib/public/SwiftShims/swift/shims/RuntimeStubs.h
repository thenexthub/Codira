//===--- RuntimeStubs.h -----------------------------------------*- C++ -*-===//
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
// Misc stubs for functions which should be defined in the core standard
// library, but are difficult or impossible to write in Swift at the
// moment.
//
//===----------------------------------------------------------------------===//

#ifndef SWIFT_STDLIB_SHIMS_RUNTIMESTUBS_H_
#define SWIFT_STDLIB_SHIMS_RUNTIMESTUBS_H_

#include "LibcShims.h"

#ifdef __cplusplus
extern "C" {
#endif

SWIFT_BEGIN_NULLABILITY_ANNOTATIONS

SWIFT_RUNTIME_STDLIB_API
__swift_ssize_t
swift_stdlib_readLine_stdin(unsigned char * _Nullable * _Nonnull LinePtr);

SWIFT_END_NULLABILITY_ANNOTATIONS

#ifdef __cplusplus
} // extern "C"
#endif

#endif // SWIFT_STDLIB_SHIMS_RUNTIMESTUBS_H_

