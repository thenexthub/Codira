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
// Swift doesn't support exception handlers, but might call code that uses
// exceptions, and when they leak out into Swift code, we want to trap them.
//
// To that end, we have our own exception personality routine, which we use
// to trap exceptions and terminate.
//
//===----------------------------------------------------------------------===//

#ifndef SWIFT_RUNTIME_EXCEPTION_H
#define SWIFT_RUNTIME_EXCEPTION_H

#include "language/Runtime/Config.h"

#if defined(__ELF__) || defined(__APPLE__)
#include <unwind.h>

namespace language {

SWIFT_RUNTIME_STDLIB_API _Unwind_Reason_Code
swift_exceptionPersonality(int version,
                           _Unwind_Action actions,
                           uint64_t exceptionClass,
                           struct _Unwind_Exception *exceptionObject,
                           struct _Unwind_Context *context);

} // end namespace language

#endif // defined(__ELF__) || defined(__APPLE__)

#endif // SWIFT_RUNTIME_EXCEPTION_H
