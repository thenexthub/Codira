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

#ifndef LSPSERVER_INDEX_REF_H
#define LSPSERVER_INDEX_REF_H

#include <cstdint>
#include "Symbol.h"
namespace ark {
namespace lsp {
enum class RefKind : uint8_t {
    UNKNOWN = 0,
    DECLARATION = 1 << 0,
    DEFINITION = 1 << 1,
    REFERENCE = 1 << 2,
    IMPORT = 1 << 3,
    ALL = DEFINITION | REFERENCE | DECLARATION
};

inline RefKind operator&(RefKind a, RefKind b)
{
    return static_cast<RefKind>(static_cast<uint8_t>(a) & static_cast<uint8_t>(b));
}

struct Ref {
    SymbolLocation location;
    RefKind kind = RefKind::UNKNOWN;
    SymbolID container{};
    bool isCodeoRef{false};
    bool isSuper{false};
};

using RefSlab = std::map<SymbolID, std::vector<Ref>>;

} // namespace lsp
} // namespace ark
#endif // LSPSERVER_INDEX_REF_H
