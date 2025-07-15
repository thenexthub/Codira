//===--- DemanglerAssert.h - Assertions for de/re-mangling ------*- C++ -*-===//
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
// This file implements a macro, DEMANGLE_ASSERT(), which will assert in the
// compiler, but in the runtime will return a ManglingError on failure.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_DEMANGLING_ASSERT_H
#define LANGUAGE_DEMANGLING_ASSERT_H

#include "toolchain/Support/Compiler.h"
#include "language/Demangling/Demangle.h"
#include "language/Demangling/NamespaceMacros.h"

#if defined(NDEBUG) || defined (LANGUAGE_RUNTIME)

// In the runtime and non-asserts builds, DEMANGLER_ASSERT() returns an error.
#define DEMANGLER_ASSERT(expr, node)                                           \
  do {                                                                         \
    if (!(expr))                                                               \
      return ManglingError(ManglingError::AssertionFailed, (node), __LINE__);  \
  } while (0)

#else

// Except in unittests, assert builds cause DEMANGLER_ASSERT() to assert()
#define DEMANGLER_ASSERT(expr, node)                                           \
  do {                                                                         \
    if (!(expr)) {                                                             \
      if (Factory.disableAssertionsForUnitTest)                                \
        return ManglingError(ManglingError::AssertionFailed, (node),           \
                             __LINE__);                                        \
      else                                                                     \
        language::Demangle::failAssert(__FILE__, __LINE__, node, #expr);          \
    }                                                                          \
  } while (0)

#endif

// DEMANGLER_ALWAYS_ASSERT() *always* fails the program, even in the runtime
#define DEMANGLER_ALWAYS_ASSERT(expr, node)                                    \
  do {                                                                         \
    if (!(expr))                                                               \
      language::Demangle::failAssert(__FILE__, __LINE__, node, #expr);            \
  } while (0)

namespace language {
namespace Demangle {
LANGUAGE_BEGIN_INLINE_NAMESPACE

[[noreturn]] void failAssert(const char *file, unsigned line, NodePointer node,
                             const char *expr);

LANGUAGE_END_INLINE_NAMESPACE
} // end namespace Demangle
} // end namespace language

#endif // LANGUAGE_DEMANGLING_ASSERT_H
