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

#include "Column.h"
#include "BlobView.h"

#include "sqlite3.h"

namespace sqldb {

Column::Column(sqlite3_stmt *Stmt, int Index) noexcept : Stmt(Stmt), Index(Index) {}

std::string_view Column::getName() const noexcept { return sqlite3_column_name(Stmt, Index); }

BlobView Column::getBlob() const noexcept
{
    return {static_cast<const void *>(sqlite3_column_blob(Stmt, Index)),
        static_cast<std::size_t>(sqlite3_column_bytes(Stmt, Index))};
}

std::string_view Column::getText() const noexcept
{
    return {static_cast<const char *>(sqlite3_column_blob(Stmt, Index)),
        static_cast<std::size_t>(sqlite3_column_bytes(Stmt, Index))};
}

std::int64_t Column::getInt64() const noexcept { return sqlite3_column_int64(Stmt, Index); }

double Column::getDouble() const noexcept { return sqlite3_column_double(Stmt, Index); }

bool Column::isBlob() const noexcept { return sqlite3_column_type(Stmt, Index) == SQLITE_BLOB; }

bool Column::isFloat() const noexcept { return sqlite3_column_type(Stmt, Index) == SQLITE_FLOAT; }

bool Column::isInteger() const noexcept { return sqlite3_column_type(Stmt, Index) == SQLITE_INTEGER; }

bool Column::isText() const noexcept { return sqlite3_column_type(Stmt, Index) == SQLITE_TEXT; }

bool Column::isNull() const noexcept { return sqlite3_column_type(Stmt, Index) == SQLITE_NULL; }

} // namespace sqldb
