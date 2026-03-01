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

#include "Bind.h"
#include "Value.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>

namespace sqldb {

/**
 * Non-owning BLOB (Binary Large Object) view object. BLOB represents a
 * contiguous chunk of binary data stored in the form of a single entity
 * in an SQLite database.
 */
class BlobView {
public:
    using size_type = std::size_t;
    using value_type = std::uint8_t;
    using pointer = const value_type *;
    using iterator = const value_type *;
    using reference = const value_type &;

public:
    /**
     * Construct an empty BlobView object.
     */
    BlobView() noexcept = default;

    /**
     * BlobView objects are copy-constructible.
     */
    BlobView(const BlobView &) noexcept = default;

    /**
     * BlobView objects are copy-assignable.
     */
    BlobView &operator=(const BlobView &) noexcept = default;

    /**
     * Constuct a new BlobView object of the specified Size using the
     * specified Data buffer. The created BlobView object does not own or
     * manage the lifetime of the underlying memory buffer.
     */
    constexpr BlobView(const void *Data, size_type Size) noexcept : Data(static_cast<pointer>(Data)), Size(Size) {}

    /**
     * Constuct a new BlobView object as a slice of the specified Size from
     * the specified Blob object.
     */
    constexpr BlobView(BlobView Blob, size_type Size) noexcept : BlobView(Blob.Data, std::min(Blob.Size, Size)) {}

    constexpr reference operator[](size_type I) noexcept { return Data[I]; }

    constexpr iterator begin() const noexcept { return Data; }
    constexpr iterator end() const noexcept { return Data + Size; }

    constexpr pointer data() const noexcept { return Data; }
    constexpr size_type size() const noexcept { return Size; }

    constexpr bool empty() const noexcept { return Size == 0; }

    constexpr size_type consume(size_type N) noexcept
    {
        N = std::min(N, Size);
        Data += N;
        Size -= N;
        return N;
    }

    constexpr size_type consume() noexcept
    {
        if (Size > 0) {
            ++Data;
            --Size;
            return 1;
        }
        return 0;
    }

    bool operator==(const BlobView &Other) const noexcept
    {
        return std::equal(begin(), end(), Other.begin(), Other.end());
    }

    bool operator!=(const BlobView &Other) const noexcept { return !operator==(Other); }

private:
    pointer Data = nullptr;
    size_type Size = 0;
};

#ifndef DOXYGEN_IGNORE

template <>
struct traits::Bind<BlobView> {
    static int call(sqlite3_stmt *S, int I, BlobView V, sqlite3_destructor_type D = sqlite::Static)
    {
        return sqlite::bind_blob64(S, I, V.data(), V.size(), D);
    }
};

template <>
struct traits::Value<BlobView> {
    static BlobView call(sqlite3_value *V)
    {
        using T = BlobView::value_type;
        return {data<T>(V), count<T>(V)};
    }
};

#endif // DOXYGEN_IGNORE

} // namespace sqldb
