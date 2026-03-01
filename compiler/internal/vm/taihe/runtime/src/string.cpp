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
#include <taihe/string.abi.h>

#include <algorithm>

// Converts a TString into its corresponding heap-allocated TStringData
// structure.
// # Arguments
// - `tstr`: The TString to be converted.
// # Returns
// - A pointer to the TStringData structure if the TString is heap-allocated.
// - `nullptr` if the TString is a reference (TSTRING_REF is set).
TH_INLINE struct TStringData *to_heap(struct TString tstr)
{
    if (tstr.flags & TSTRING_REF) {
        return nullptr;
    }
    return reinterpret_cast<struct TStringData *>(const_cast<char *>(tstr.ptr) - offsetof(struct TStringData, buffer));
}

char *tstr_initialize(struct TString *tstr_ptr, uint32_t capacity)
{
    size_t bytes_required = sizeof(struct TStringData) + sizeof(char) * capacity;
    struct TStringData *sh = reinterpret_cast<struct TStringData *>(malloc(bytes_required));
    tref_init(&sh->count, 1);
    tstr_ptr->flags = 0;
    tstr_ptr->ptr = sh->buffer;
    return sh->buffer;
}

struct TString tstr_new(char const *value TH_NONNULL, size_t len)
{
    struct TString tstr;
    char *buf = tstr_initialize(&tstr, len + 1);
    buf = std::copy(value, value + len, buf);
    *buf = '\0';
    tstr.length = len;
    return tstr;
}

struct TString tstr_new_ref(char const *buf TH_NONNULL, size_t len)
{
    struct TString tstr;
    tstr.flags = TSTRING_REF;
    tstr.length = len;
    tstr.ptr = buf;
    return tstr;
}

struct TString tstr_dup(struct TString tstr)
{
    struct TStringData *sh = to_heap(tstr);
    if (!sh) {
        return tstr_new(tstr.ptr, tstr.length);
    }
    tref_inc(&sh->count);
    return tstr;
}

void tstr_drop(struct TString tstr)
{
    struct TStringData *sh = to_heap(tstr);
    if (!sh) {
        return;
    }
    if (tref_dec(&sh->count)) {
        free(sh);
    }
}

struct TString tstr_concat(size_t count, struct TString const *tstr_list)
{
    size_t len = 0;
    for (size_t i = 0; i < count; ++i) {
        len += tstr_list[i].length;
    }
    struct TString tstr;
    char *buf = tstr_initialize(&tstr, len + 1);
    for (size_t i = 0; i < count; ++i) {
        buf = std::copy(tstr_list[i].ptr, tstr_list[i].ptr + tstr_list[i].length, buf);
    }
    *buf = '\0';
    tstr.length = len;
    return tstr;
}

struct TString tstr_substr(struct TString tstr, size_t pos, size_t len)
{
    if (pos > tstr.length) {
        len = 0;
    } else if (pos + len > tstr.length) {
        len = tstr.length - pos;
    }
    return tstr_new_ref(tstr.ptr + pos, len);
}