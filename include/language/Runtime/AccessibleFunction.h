//===---- AccessibleFunction.h - Runtime accessible functions ---*- C++ -*-===//
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
// The runtime interface for functions accessible by name.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_RUNTIME_ACCESSIBLE_FUNCTION_H
#define LANGUAGE_RUNTIME_ACCESSIBLE_FUNCTION_H

#include "language/ABI/Metadata.h"

#include <cstdint>

namespace language {
namespace runtime {

LANGUAGE_RUNTIME_STDLIB_SPI
const AccessibleFunctionRecord *
language_findAccessibleFunction(const char *targetNameStart,
                             size_t targetNameLength);

} // end namespace runtime
} // end namespace language

#endif
