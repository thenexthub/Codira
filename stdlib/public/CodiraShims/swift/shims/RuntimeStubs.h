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
// library, but are difficult or impossible to write in Codira at the
// moment.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_STDLIB_SHIMS_RUNTIMESTUBS_H_
#define LANGUAGE_STDLIB_SHIMS_RUNTIMESTUBS_H_

#include "LibcShims.h"

#ifdef __cplusplus
extern "C" {
#endif

LANGUAGE_BEGIN_NULLABILITY_ANNOTATIONS

LANGUAGE_RUNTIME_STDLIB_API
__language_ssize_t
language_stdlib_readLine_stdin(unsigned char * _Nullable * _Nonnull LinePtr);

LANGUAGE_END_NULLABILITY_ANNOTATIONS

#ifdef __cplusplus
} // extern "C"
#endif

#endif // LANGUAGE_STDLIB_SHIMS_RUNTIMESTUBS_H_

