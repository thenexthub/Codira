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

#pragma once

#include "SQLiteAPI.h"

#include <string_view>

namespace sqldb {
namespace impl {

template <typename Function>
struct Tokenizer {
    Function Tokenize;
};

template <typename Tokenizer>
auto &tokenize(Fts5Tokenizer *Tok)
{
    return reinterpret_cast<Tokenizer *>(Tok)->Tokenize;
}

template <typename Tokenizer>
int tokenize(Fts5Tokenizer *Tok,
    void *Ctx,
    int,
    const char *Text,
    int TextLen,
    int (*Callback)(void *, int, const char *, int, int, int))
{
    tokenize<Tokenizer>(Tok)(std::string_view(Text, TextLen), [&](std::string_view Token, size_t Pos) {
        Callback(Ctx, 0, Token.data(), Token.size(), Pos, Pos + Token.size());
    });
    return sqlite::OK;
}

} // namespace impl
} // namespace sqldb
