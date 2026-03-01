/*
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 */

#ifndef LSPSERVER_INDEX_CALLRELATION_H
#define LSPSERVER_INDEX_CALLRELATION_H

#include <string>
#include <unordered_map>
#include <vector>
#include "Symbol.h"

namespace ark {
namespace lsp {

enum class CallRelationKind : uint8_t {
    CALLED_BY,
    CALLS,
    CONTAINED_BY,
    CONTAINS
};

// Called by -> Calls, Calls -> Called by
// Contained by -> Contains, Contains -> Contained by
inline CallRelationKind InvertCallRelationKind(CallRelationKind kind)
{
    switch (kind) {
        case CallRelationKind::CALLED_BY:
            return CallRelationKind::CALLS;
        case CallRelationKind::CALLS:
            return CallRelationKind::CALLED_BY;
        case CallRelationKind::CONTAINED_BY:
            return CallRelationKind::CONTAINS;
        case CallRelationKind::CONTAINS:
            return CallRelationKind::CONTAINED_BY;
    }
}

/// Represents a relation between two symbols.
/// For an example "B calls A" may be represented
/// as { Subject = A, Predicate = CALLED_BY, Object = B, Location }.
struct CallRelation {
    SymbolID subject;
    CallRelationKind predicate;
    SymbolID object;
    SymbolLocation location;
};
} // namespace lsp
} // namespace ark
#endif // LSPSERVER_INDEX_CALLRELATION_H
