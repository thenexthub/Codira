//===--- Exception.h - Exception support ------------------------*- C++ -*-===//
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
// Codira doesn't support exception handlers, but might call code that uses
// exceptions, and when they leak out into Codira code, we want to trap them.
//
// To that end, we have our own exception personality routine, which we use
// to trap exceptions and terminate.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_RUNTIME_EXCEPTION_H
#define LANGUAGE_RUNTIME_EXCEPTION_H

#include "language/Runtime/Config.h"

#if defined(__ELF__) || defined(__APPLE__)
#include <unwind.h>

namespace language {

LANGUAGE_RUNTIME_STDLIB_API _Unwind_Reason_Code
language_exceptionPersonality(int version,
                           _Unwind_Action actions,
                           uint64_t exceptionClass,
                           struct _Unwind_Exception *exceptionObject,
                           struct _Unwind_Context *context);

} // end namespace language

#endif // defined(__ELF__) || defined(__APPLE__)

#endif // LANGUAGE_RUNTIME_EXCEPTION_H
