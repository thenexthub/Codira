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

#include "Exception.h"
#include "Function.h"
#include "SQLiteAPI.h"
#include "Statement.h"
#include "Tokenize.h"
#include "Traits.h"

#if __cplusplus > 201402L
#if defined(__has_include)
#if __has_include(<filesystem> )
#include <filesystem>
#else
#include <experimental/filesystem>
namespace std {
namespace filesystem = experimental::filesystem;
} // namespace std
#endif // __has_include(<filesystem>)
#endif // defined(__has_include)
#endif // __cplusplus > 201402L

#include <string>

namespace sqldb {
/**
 * Database connection object.
 */
class Connection {
public:
    /**
     * Create connection object from SQLite native handle. The connection
     * object will manage the lifetime of the passed native handle.
     */
    explicit Connection(sqlite3 *DB) noexcept;

    /**
     * Create connection object with invalid state.
     */
    Connection() noexcept = default;

    /**
     * Close the connection and release all resources associated with the managed
     * SQLite native handle.
     */
    ~Connection() noexcept;

    /**
     * Connection objects are move-constructible.
     */
    Connection(Connection &&Other) noexcept;

    /**
     * Connection objects are move-assignable.
     */
    Connection &operator=(Connection &&Other) noexcept;

    /**
     * Connection objects are not copy-constructible.
     */
    Connection(const Connection &) = delete;

    /**
     * Connection objects are not copy-assignable.
     */
    Connection &operator=(const Connection &) = delete;

    /**
     * Exchange SQLite native handle of the connection with those of Other
     * connection object.
     */
    void swap(Connection &Other) noexcept;

    /**
     * Return true if the managed SQLite native handle is valid.
     */
    [[nodiscard]] explicit operator bool() const noexcept;

    /**
     * Transfer ownership of SQLite native handle to the caller.
     */
    [[nodiscard]] auto release() noexcept { return std::exchange(DB, nullptr); }

    /**
     * Return SQLite native handle.
     */
    [[nodiscard]] auto getNativeHandle() const noexcept { return DB; }

    /**
     * Return name of the file associated with the specified database.
     */
    [[nodiscard]] std::filesystem::path getFileName(const std::string &Database = "main") const;

    /**
     * Return true if the specified database is opened in read-only mode.
     */
    [[nodiscard]] bool isReadOnly(const std::string &Database = "main") const noexcept;

    /**
     * Interrupt any pending database operation and cause it to return at its
     * earliest opportunity.
     */
    void interrupt() noexcept;

    /**
     * Return true if interrupt is currently in effect for the connection.
     */
    [[nodiscard]] bool isInterrupted() const noexcept;

    /**
     * Enable loading of SQLite extension shared libraries.
     */
    void enableLoadExtension();

    /**
     * Load an SQLite extension shared library from the specified file.
     */
    void loadExtension(const std::filesystem::path &FileName);

    /**
     * Create prepared SQL statement.
     */
    [[nodiscard]] Statement prepare(std::string_view SQL);

    /**
     * Execute one or more SQL statements separated by semicolon.
     */
    void execute(std::string_view SQL);

    /**
     * Register new application-defined scalar SQL function.
     */
    template <typename Function>
    void scalar(const std::string &Name, Function F);

    /**
     * Register new application-defined aggregate SQL function.
     */
    template <typename Step, typename Final>
    void aggregate(const std::string &Name, Step S, Final F);

    /**
     * Register new custom FTS5 tokenizer function.
     */
    template <typename Function>
    void tokenizer(const std::string &Name, Function F);

    /**
     * Set a callback function which will be invoked periodically during long
     * running calls like execution of a SQL statement. If the progress callback
     * returns true then the operation is interrupted with error.
     */
    void progress(std::add_pointer_t<bool()> F) noexcept;

    /**
     * Set a callback function that might be invoked whenever an attempt is
     * made to access a database table when another thread or process has
     * the table locked. The argument to the busy handler callback is the number
     * of times that the busy handler has been invoked previously for the same
     * locking event. If the busy callback returns false then no additional
     * attempts are made to access the database and operation fails with error.
     */
    void busy(std::add_pointer_t<bool(int)> F) noexcept;

    /**
     * Return the number of rows modified, inserted or deleted by the most
     * recently completed INSERT, UPDATE or DELETE statement on this database
     * connection. Executing any other type of SQL statement does not modify
     * the value returned by this function.
     */
    [[nodiscard]] int getChanges() const noexcept;

    /**
     * Free as much memory used by a database connection as possible.
     */
    void releaseMemory();

private:
    sqlite3 *DB = nullptr;
};

template <typename Function>
void Connection::scalar(const std::string &Name, Function F)
{
    auto Scalar = new impl::Scalar<Function>({std::move(F)});
    int RC = sqlite::create_scalar(DB, Name.c_str(), traits::function<Function>::arity,
        sqlite::UTF8 | sqlite::Deterministic | sqlite::Innocuous, Scalar,
        impl::scalar<std::remove_reference_t<decltype(*Scalar)>>,
        [](void *P) { delete static_cast<decltype(Scalar)>(P); });
#ifndef NO_EXCEPTIONS
    if (RC != sqlite::OK) {
        throw Exception(DB, "Failed to create scalar function " + Name);
    }
#endif
}

template <typename Step, typename Final>
void Connection::aggregate(const std::string &Name, Step S, Final F)
{
    auto Aggregate = new impl::Aggregate<Step, Final>({std::move(S), std::move(F)});
    int RC = sqlite::create_aggregate(DB, Name.c_str(), traits::function<Step>::arity - 1,
        sqlite::UTF8 | sqlite::Deterministic | sqlite::DirectOnly, Aggregate,
        impl::step<std::remove_reference_t<decltype(*Aggregate)>>,
        impl::final<std::remove_reference_t<decltype(*Aggregate)>>,
        [](void *P) { delete static_cast<decltype(Aggregate)>(P); });
#ifndef NO_EXCEPTIONS
    if (RC != sqlite::OK) {
        throw Exception(DB, "Failed to create aggregate function " + Name);
    }
#endif
}

template <typename Function>
void Connection::tokenizer(const std::string &Name, Function F)
{
    auto Tokenizer = new impl::Tokenizer<Function>({std::move(F)});
    int RC = sqlite::fts5::create_tokenizer(DB, Name.c_str(), Tokenizer,
        impl::tokenize<std::remove_reference_t<decltype(*Tokenizer)>>,
        [](void *P) { delete static_cast<decltype(Tokenizer)>(P); });
#ifndef NO_EXCEPTIONS
    if (RC != sqlite::OK) {
        throw Exception(DB, "Failed to create FTS5 tokenizer " + Name);
    }
#endif
}

/**
 * NOLINTBEGIN(misc-unused-using-decls)
 * Open flags.
 */
using sqlite::OpenCreate;
using sqlite::OpenReadOnly;
using sqlite::OpenReadWrite;
using sqlite::OpenURI;
/**
 * NOLINTEND(misc-unused-using-decls)
 */

/**
 * Open specified SQLite database file.
 */
Connection open(const std::filesystem::path &File, int Flags = OpenCreate | OpenReadWrite);

/**
 * Open in-memory SQLite database.
 * An unnamed in-memory database can be created using the special name
 * ":memory:". The database will be visible only to the database connection
 * that originally opened it. The database ceases to exist as soon as the
 * database connection is closed. Every ":memory:" database is distinct from
 * every other.
 *
 * A named in-memory database can be created using an URI name, e.g.
 * "file:/name". The named database can be shared between different
 * connections. The shared named database will be deleted after closing
 * the last connection.
 */
Connection memdb(const std::string &Name = ":memory:", int Flags = OpenCreate | OpenReadWrite);

/**
 * Create and open a temporary database. The database ceases to exist as soon
 * as the database connection is closed.
 */
Connection tempdb();

} // namespace sqldb
