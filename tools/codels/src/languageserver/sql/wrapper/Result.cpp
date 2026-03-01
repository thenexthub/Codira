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

#include "Result.h"
#include "Column.h"

#include "sqlite3.h"

namespace sqldb {

Result::Result(sqlite3_stmt *Stmt) noexcept : Stmt(Stmt) {}

Result::operator bool() const noexcept { return Stmt != nullptr; }

int Result::getColumnCount() const noexcept { return sqlite3_data_count(Stmt); }

Column Result::getColumn(int Index) const noexcept { return {Stmt, Index}; }

Column Result::operator[](int Index) const noexcept { return {Stmt, Index}; }

Result::Iterator Result::begin() const noexcept { return {Stmt, getColumnCount(), 0}; }

Result::Iterator Result::end() const noexcept
{
    int Count = getColumnCount();
    return {Stmt, Count, Count};
}

Result::Iterator::Iterator(sqlite3_stmt *Stmt, int Count, int Index) noexcept : Stmt(Stmt), Count(Count), Index(Index)
{
}

Result::Iterator::reference Result::Iterator::operator*() const noexcept { return {Stmt, Index}; }

Result::Iterator &Result::Iterator::operator++() noexcept
{
    ++Index;
    return *this;
}

Result::Iterator Result::Iterator::operator++(int) noexcept { return {Stmt, Count, Index++}; }

bool operator==(const Result::Iterator &LHS, const Result::Iterator &RHS) noexcept
{
    return LHS.Stmt == RHS.Stmt && LHS.Index == RHS.Index;
}

bool operator!=(const Result::Iterator &LHS, const Result::Iterator &RHS) noexcept
{
    return LHS.Stmt != RHS.Stmt || LHS.Index != RHS.Index;
}

} // namespace sqldb
