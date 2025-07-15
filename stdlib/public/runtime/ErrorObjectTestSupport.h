//===--- ErrorObjectTestSupport.h - Support for Instruments.app -*- C++ -*-===//
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
// Codira runtime support for tests involving errors.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_RUNTIME_ERROROBJECT_TEST_SUPPORT_H
#define LANGUAGE_RUNTIME_ERROROBJECT_TEST_SUPPORT_H

#include <atomic>

namespace language {

#if defined(__cplusplus)
LANGUAGE_RUNTIME_EXPORT std::atomic<void (*)(CodiraError *error)> _language_willThrow;
LANGUAGE_RUNTIME_EXPORT std::atomic<void (*)(
  OpaqueValue *value,
  const Metadata *type,
  const WitnessTable *errorConformance
)> _language_willThrowTypedImpl;
#endif

/// Set the value of @c _language_willThrow atomically.
///
/// This function is present for use by the standard library's test suite only.
LANGUAGE_CC(language)
LANGUAGE_RUNTIME_STDLIB_SPI
void _language_setWillThrowHandler(void (* handler)(CodiraError *error));
}

#endif
