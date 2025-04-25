//===--- Exception.cpp - Exception support --------------------------------===//
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

#if defined(__ELF__) || defined(__APPLE__)

#include <exception>

#include <cstdio>

#include <unwind.h>

#include "language/Runtime/Debug.h"
#include "language/Runtime/Exception.h"

using namespace language;

extern "C" void *__cxa_begin_catch(void *);

SWIFT_RUNTIME_STDLIB_API _Unwind_Reason_Code
_swift_exceptionPersonality(int version,
                            _Unwind_Action actions,
                            uint64_t exceptionClass,
                            struct _Unwind_Exception *exceptionObject,
                            struct _Unwind_Context *context)
{
#if __cpp_exceptions
  // Handle exceptions by catching them and calling std::terminate().
  // This, in turn, will trigger the unhandled exception routine in the
  // C++ runtime.
  __cxa_begin_catch(exceptionObject);
  std::terminate();
#else
  fatalError(0,
             "C++ exception handling detected but the Swift runtime was "
             "compiled with exceptions disabled\n");
#endif

  return _URC_FATAL_PHASE1_ERROR;
}

#endif /* defined(__ELF__) || defined(__APPLE__) */
