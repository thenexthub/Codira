//===------------------------------------------------------------*- C++ -*-===//
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

#ifndef LANGUAGE_RUNTIME_LANGUAGE_EQUATABLE_SUPPORT_H
#define LANGUAGE_RUNTIME_LANGUAGE_EQUATABLE_SUPPORT_H

#include "language/Runtime/Metadata.h"
#include <stdint.h>

namespace language {
namespace equatable_support {

extern "C" const ProtocolDescriptor PROTOCOL_DESCR_SYM(SQ);
static constexpr auto &EquatableProtocolDescriptor = PROTOCOL_DESCR_SYM(SQ);

struct EquatableWitnessTable;

/// Calls `Equatable.==` through a `Equatable` (not Equatable!) witness
/// table.
LANGUAGE_CC(language) LANGUAGE_RUNTIME_STDLIB_INTERNAL
bool _language_stdlib_Equatable_isEqual_indirect(
    const void *lhsValue, const void *rhsValue, const Metadata *type,
    const EquatableWitnessTable *wt);

} // namespace equatable_support
} // namespace language

#endif

