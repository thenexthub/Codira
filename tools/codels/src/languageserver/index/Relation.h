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

#ifndef LSPSERVER_INDEX_RELATION_H
#define LSPSERVER_INDEX_RELATION_H

#include <cstdint>
#include "Symbol.h"

namespace ark {
namespace lsp {
enum class RelationKind : uint8_t {
    BASE_OF,    // Type inheritance relation. eg: A <: B, B is base of A.
    RIDDEND_BY, // Member relation. eg: open A.f1, override B.f1, A.f1 ridden by B.f1.
    EXTEND,     // Type extend relation. eg: extend A <: I, A extend I.
    CALLED_BY,
    CONTAINED_BY,
    OVERRIDES,
};

struct Relation {
    SymbolID subject;
    RelationKind predicate;
    SymbolID object;
};

using RelationSlab = std::vector<Relation>;

} // namespace lsp
} // namespace ark
#endif // LSPSERVER_INDEX_RELATION_H
