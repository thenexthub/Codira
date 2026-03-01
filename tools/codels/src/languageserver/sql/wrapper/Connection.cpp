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

#include "Connection.h"
#include "Exception.h"
#include "Statement.h"
#include "Transaction.h"

#include "ScopeExit.h"

#include "sqlite3.h"

namespace sqldb {

Connection::Connection(sqlite3 *DB) noexcept : DB(DB) {}

Connection::~Connection() noexcept { sqlite3_close_v2(DB); }

Connection::Connection(Connection &&Other) noexcept { swap(Other); }

Connection &Connection::operator=(Connection &&Other) noexcept
{
    swap(Other);
    if (this == &Other) {
        return *this;
    }
    return *this;
}

void Connection::swap(Connection &Other) noexcept { std::swap(DB, Other.DB); }

Connection::operator bool() const noexcept { return DB != nullptr; }

std::filesystem::path Connection::getFileName(const std::string &Database) const
{
    if (const char *File = sqlite3_db_filename(DB, Database.c_str())) {
        return std::filesystem::path(File);
    }
    return {};
}

bool Connection::isReadOnly(const std::string &Database) const noexcept
{
    return sqlite3_db_readonly(DB, Database.c_str());
}

void Connection::interrupt() noexcept { return sqlite3_interrupt(DB); }

bool Connection::isInterrupted() const noexcept { return sqlite3_is_interrupted(DB); }

void Connection::enableLoadExtension()
{
    int RC = sqlite3_enable_load_extension(DB, 1);
    if (RC != SQLITE_OK) {
        throw Exception(RC, "Failed to enable extension loading");
    }
}

void Connection::loadExtension(const std::filesystem::path &File)
{
    char *ErrMsg = nullptr;
    ScopeExit FreeErrMsg = [&]() noexcept { sqlite3_free(ErrMsg); };
    int RC = sqlite3_load_extension(DB, File.string().c_str(), nullptr, &ErrMsg);
    if (RC != SQLITE_OK) {
        throw Exception(RC, "Failed to load extension from \"" + File.string() + "\"" +
                                (ErrMsg != nullptr ? ": " + std::string(ErrMsg) : ""));
    }
}

Statement Connection::prepare(std::string_view SQL)
{
    sqlite3_stmt *Stmt = nullptr;
    int RC =
        sqlite3_prepare_v3(DB, SQL.data(), static_cast<int>(SQL.size()), SQLITE_PREPARE_PERSISTENT, &Stmt, nullptr);
    if (RC != SQLITE_OK) {
        throw Exception(RC, "Failed to prepare statement \"" + std::string(SQL) + "\"");
    }
    if (Stmt == nullptr) {
        throw Exception(sqlite::Error, "Failed to prepare SQL statement");
    }
    return Statement(Stmt);
}

void Connection::execute(std::string_view SQL)
{
    auto Prepare = [this](std::string_view &innerSQL) {
        const char *Tail = nullptr;
        sqlite3_stmt *Stmt = nullptr;
        int RC = sqlite3_prepare_v2(DB, innerSQL.data(), static_cast<int>(innerSQL.size()), &Stmt, &Tail);
        if (RC != SQLITE_OK) {
            throw Exception(DB, "Failed to prepare statement \"" + std::string(innerSQL) + "\"");
        }
        innerSQL.remove_prefix(Tail - innerSQL.data());
        return Statement(Stmt);
    };
    while (!SQL.empty()) {
        Prepare(SQL).execute();
    }
}

static int progressHandler(void *P) noexcept { return reinterpret_cast<std::add_pointer_t<bool()>>(P)(); }

void Connection::progress(std::add_pointer_t<bool()> F) noexcept
{
    int nOps = 1000;
    sqlite3_progress_handler(DB, nOps, progressHandler, reinterpret_cast<void *>(F));
}

static int busyHandler(void *P, int N) noexcept { return reinterpret_cast<std::add_pointer_t<bool(int)>>(P)(N); }

void Connection::busy(std::add_pointer_t<bool(int)> F) noexcept
{
    sqlite3_busy_handler(DB, busyHandler, reinterpret_cast<void *>(F));
}

int Connection::getChanges() const noexcept { return sqlite3_changes(DB); }

// Transaction Connection::begin() { return Transaction(*this); }

void Connection::releaseMemory()
{
    int RC = sqlite3_db_release_memory(DB);
    if (RC != SQLITE_OK) {
        throw Exception(DB, "Failed to release database connection memory");
    }
}

Connection open(const std::filesystem::path &File, int Flags)
{
    sqlite3 *SQLite = nullptr;
    int RC = sqlite3_open_v2(File.string().c_str(), &SQLite, Flags, nullptr);
    auto DB = Connection(SQLite); // Non-null even on error.
    if (RC != SQLITE_OK) {
        throw Exception(RC, "Cannot open database file " + File.string());
    }
    return DB;
}

Connection memdb(const std::string &Name, int Flags)
{
    sqlite3 *SQLite = nullptr;
    int RC = sqlite3_open_v2(Name.c_str(), &SQLite, Flags, "memdb");
    auto DB = Connection(SQLite); // Non-null even on error.
    if (RC != SQLITE_OK) {
        throw Exception(RC, "Cannot open in-memory database " + Name);
    }
    return DB;
}

Connection tempdb()
{
    sqlite3 *SQLite = nullptr;
    int RC = sqlite3_open("", &SQLite);
    auto DB = Connection(SQLite); // Non-null even on error.
    if (RC != SQLITE_OK) {
        throw Exception(RC, "Cannot open temporary database");
    }
    return DB;
}

} // namespace sqldb
