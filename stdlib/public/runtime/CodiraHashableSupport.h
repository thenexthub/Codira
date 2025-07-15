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

#ifndef LANGUAGE_RUNTIME_LANGUAGE_HASHABLE_SUPPORT_H
#define LANGUAGE_RUNTIME_LANGUAGE_HASHABLE_SUPPORT_H

#include "language/Runtime/Metadata.h"
#include <stdint.h>

namespace language {
namespace hashable_support {

extern "C" const ProtocolDescriptor PROTOCOL_DESCR_SYM(SH);
static constexpr auto &HashableProtocolDescriptor = PROTOCOL_DESCR_SYM(SH);

struct HashableWitnessTable;

/// Calls `Equatable.==` through a `Hashable` (not Equatable!) witness
/// table.
LANGUAGE_CC(language) LANGUAGE_RUNTIME_STDLIB_INTERNAL
bool _language_stdlib_Hashable_isEqual_indirect(
    const void *lhsValue, const void *rhsValue, const Metadata *type,
    const HashableWitnessTable *wt);

/// Calls `Hashable.hashValue.get` through a `Hashable` witness table.
LANGUAGE_CC(language) LANGUAGE_RUNTIME_STDLIB_INTERNAL
intptr_t _language_stdlib_Hashable_hashValue_indirect(
    const void *value, const Metadata *type, const HashableWitnessTable *wt);

LANGUAGE_CC(language) LANGUAGE_RUNTIME_STDLIB_INTERNAL
void _language_convertToAnyHashableIndirect(
    OpaqueValue *source, OpaqueValue *destination, const Metadata *sourceType,
    const HashableWitnessTable *sourceConformance);

LANGUAGE_CC(language) LANGUAGE_RUNTIME_STDLIB_INTERNAL
bool _language_anyHashableDownCastConditionalIndirect(
    OpaqueValue *source, OpaqueValue *destination, const Metadata *targetType);

/// Find the base type that introduces the `Hashable` conformance.
/// Because the provided type is known to conform to `Hashable`, this
/// function always returns non-null.
///
/// - Precondition: `type` conforms to `Hashable` (not checked).
const Metadata *findHashableBaseTypeOfHashableType(
    const Metadata *type);

/// Find the base type that introduces the `Hashable` conformance.
/// If `type` does not conform to `Hashable`, `nullptr` is returned.
const Metadata *findHashableBaseType(const Metadata *type);

} // namespace hashable_support
} // namespace language

#endif

