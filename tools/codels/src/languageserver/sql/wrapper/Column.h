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
#include "BlobView.h"

#include <cstdint>
#include <string_view>

namespace sqldb {
/**
 * Non-owning view for a column in a row of data extracted by SQL query.
 */
class Column {
public:
    /**
     * Create a new column view object with the specified index for SQLite native
     * handle. The column view does not own or manager the lifetime of the
     * underlying SQLite native handle object.
     */
    Column(sqlite3_stmt *Stmt, int Index) noexcept;

    /**
     * Column objects are copy-constructible.
     */
    Column(const Column &) noexcept = default;

    /**
     * Column objects are not copy-assignable.
     */
    Column &operator=(const Column &) noexcept = delete;

    /**
     * Return the column name.
     */
    std::string_view getName() const noexcept;

    /**
     * Return the column value as BLOB.
     */
    BlobView getBlob() const noexcept;

    /**
     * Return the column value as TEXT.
     */
    std::string_view getText() const noexcept;

    /**
     * Return the column value as INTEGER.
     */
    std::int64_t getInt64() const noexcept;

    /**
     * Return the column value as FLOAT.
     */
    double getDouble() const noexcept;

    /**
     * Return true if the column datatype is BLOB.
     */
    bool isBlob() const noexcept;

    /**
     * Return true if the column datatype is TEXT.
     */
    bool isText() const noexcept;

    /**
     * Return true if the column datatype is INTEGER.
     */
    bool isInteger() const noexcept;

    /**
     * Return true if the column datatype is FLOAT.
     */
    bool isFloat() const noexcept;

    /**
     * Return true if the column datatype is NULL.
     */
    bool isNull() const noexcept;

private:
    sqlite3_stmt *Stmt;
    const int Index;
};

} // namespace sqldb
