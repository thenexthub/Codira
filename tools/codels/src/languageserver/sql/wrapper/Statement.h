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
#include "Exception.h"
#include "Parameter.h"
#include "Result.h"
#include "SQLiteAPI.h"

#include <functional>
#include <string>
#include <tuple>

namespace sqldb {

/**
 * Prepare a named SQL parameter for binding.
 */
template <typename T>
constexpr auto named(const char *Name, const T &Value) noexcept
{
    return Parameter<T>{Name, Value};
}

template <typename... Ts>
using Bind = std::tuple<const Ts &...>;

/**
 * Prepare values for binding to parameters of prepared SQL statement.
 */
template <typename... Ts>
constexpr auto with(const Ts &...Args) noexcept
{
    return Bind<Ts...>(Args...);
}

/**
 * Invoke callback passing fetched row of data via callback arguments.
 */
template <typename Callable>
auto invoke(Callable &&Callback) noexcept
{
    return [callback = std::forward<Callable>(Callback)](Result Row) { return Row.invoke(callback); };
}

/**
 * Copy row of data extracted by SQL query into the provided variables.
 * Pass std::ignore to ignore certain columns in the query result.
 */
template <typename... Variables>
auto into(Variables &...Vars) noexcept
{
    return [&](Result Row) {
        Row.store(Vars...);
        return false;
    };
}

/**
 * Prepared statement object. An instance of this object represents a single
 * SQL statement that has been compiled into binary form and is ready to be
 * evaluated.
 */
class Statement {
public:
    /**
     * Create prepared statement object from SQLite native handle. The prepared
     * statement object will manage the lifetime of the passed native handle.
     */
    explicit Statement(sqlite3_stmt *Stmt) noexcept;

    /**
     * Create a Statement object with invalid state.
     */
    Statement() = default;

    /**
     * Release all resources associated with managed SQLite native handle.
     */
    ~Statement() noexcept;

    /**
     * Statement objects are move-constructible.
     */
    Statement(Statement &&Other) noexcept;

    /**
     * Statement objects are move-assignable.
     */
    Statement &operator=(Statement &&Other) noexcept;

    /**
     * Statement objects are not copy-constructible.
     */
    Statement(const Statement &) = delete;

    /**
     * Statement objects are not copy-assignable.
     */
    Statement &operator=(const Statement &) = delete;

    /**
     * Exchange SQLite native handle of the prepared statement with those of
     * Other prepared statement object.
     */
    void swap(Statement &Other) noexcept;

    /**
     * Return true if the managed SQLite native handle is valid.
     */
    [[nodiscard]] explicit operator bool() const noexcept;

    /**
     * Return the SQL associated with this prepared statement.
     */
    [[nodiscard]] std::string_view getSQL() const noexcept;

    /**
     * Return the SQL associated with this prepared statement with bound
     * parameters expanded.
     */
    [[nodiscard]] std::string getExpandedSQL() const;

    /**
     * Return true if and only if the prepared statement makes no direct
     * changes to the content of the database file.
     */
    [[nodiscard]] bool isReadOnly() const noexcept;

    /**
     * Return true if execution of this prepared statement has been started
     * and has not yet run to completion.
     */
    [[nodiscard]] bool isBusy() const noexcept;

    /**
     * Bind a single Value to an SQL parameter with a specified Index on
     * the prepared statement. Indexes of bind parameters are one-based.
     * If the value is bound as TEXT or BLOB then it must remain valid until
     * either the prepared statement is finalized or the same SQL parameter is
     * bound to something else, whichever occurs sooner.
     */
    template <typename T>
    void bind(int Index, const T &Value);

    /**
     * Bind values to the SQL parameters of this prepared statement.
     * Named SQL parameters can be bound using named helper function.
     * Named SQL parameters of the form "?NNN" or ":AAA" or "@AAA" or "$AAA"
     * have a name which is the string "?NNN" or ":AAA" or "@AAA" or "$AAA"
     * respectively. In other words, the initial ":" or "$" or "@" or "?" is
     * included as part of the name.
     * Each value in the Binds should have a corresponding placeholder in the
     * SQL query associated with this prepared statement. Unbound parameters will
     * be interpreted as NULL.
     *
     * Any values bound as TEXT or BLOB must remain valid until either the
     * prepared statement is finalized or the same SQL parameter is bound to
     * something else, whichever occurs sooner.
     */
    template <typename... Ts>
    void bind(const Bind<Ts...> &Binds);

    /**
     * Reset all bound parameters to NULL.
     */
    void clearBindings() noexcept;

    /**
     * Reset this prepared statement object back to its initial state, ready to
     * be re-executed.
     * Any SQL statement variables that had values bound to them using bind
     * methods retain their values. Use clearBindings to reset the bindings.
     */
    void reset();

    /**
     * Execute a single step of this prepared statement.
     *
     * This method may return a Result object which represent a row of data
     * or an empty Result if the query execution did not return any data.
     *
     * If the prepared statement didn't run to completion then it is caller
     * responsibility to reset the prepared statement object back to its initial
     * state using reset method. Bound values can be cleared using
     * clearBindings method.
     */
    Result step();

    /**
     * Bind values to SQL parameters of this prepared statement and then execute
     * the statement and extract query results.
     *
     * Each value in the Binds should have a corresponding placeholder in the
     * SQL query associated with this prepared statement. Unbound parameters will
     * be interpreted as NULL. Named SQL parameters can be bound using named()
     * helper function.
     *
     * Any values bound as TEXT or BLOB must remain valid during the execution
     * of this method.
     *
     * The supplied Callback will be invoked for each row of data returned as
     * a result of prepared statement execution.
     *
     * Returning false from the callback will stop further query execution.
     *
     * This method will invoke reset() method to reset the prepared statement
     * to its initial state and clearBindings() to clear all bound parameters
     * before returning.
     */
    template <typename... Ts, typename Callable>
    void execute(const Bind<Ts...> &Binds, Callable &&Callback);

    /**
     * Execute this prepared statement.
     *
     * The supplied Callback will be invoked for each row of data returned as
     * a result of prepared statement execution.
     *
     * Returning false from the callback will stop further query execution.
     *
     * This method will invoke reset() method to reset the prepared statement
     * to its initial state and clearBindings() to clear all bound parameters
     * before returning.
     */
    void execute(std::function<bool(Result)> Callback);

    /**
     * Bind values to SQL parameters of this prepared statement and then execute
     * this prepared statement.
     *
     * Each value in the Binds should have a corresponding placeholder in the
     * SQL query associated with this prepared statement. Unbound parameters will
     * be interpreted as NULL. Named SQL parameters can be bound using named()
     * helper function.
     *
     * Any values bound as TEXT or BLOB must remain valid during the execution
     * of this method.
     *
     * Any rows of data returned as a result of execution of the prepared
     * statement will be silently ignored.
     *
     * This method will invoke reset() method to reset the prepared statement
     * to its initial state and clearBindings() to clear all bound parameters
     * before returning.
     */
    template <typename... Ts>
    void execute(const Bind<Ts...> &Binds);

    /**
     * Execute this prepared statement.
     *
     * Any rows of data returned as a result of execution of the prepared
     * statement will be silently ignored.
     *
     * This method will invoke reset() method to reset the prepared statement
     * to its initial state and clearBindings() to clear all bound parameters
     * before returning.
     */
    void execute();

    sqlite3_stmt *GetStmt() { return Stmt; }

private:
    sqlite3_stmt *Stmt = nullptr;
};

template <typename T>
void Statement::bind(int Index, const T &Value)
{
    traits::bind(Stmt, Index, Value);
}

template <typename... Ts, typename Callable>
void Statement::execute(const Bind<Ts...> &Binds, Callable &&Callback)
{
    bind(Binds);
    execute(std::forward<Callable>(Callback));
}

template <typename... Ts>
void Statement::execute(const Bind<Ts...> &Binds)
{
    bind(Binds);
    execute();
}

template <typename... Ts>
void Statement::bind(const Bind<Ts...> &Binds)
{
    tuple_for_each(Binds, [&, Index = 0](const auto &Value) mutable {
#ifndef NO_EXCEPTIONS
        if (int RC = traits::bind(Stmt, ++Index, Value); RC != sqlite::OK) {
            throw Exception(RC, "Failed to bind value to parameter at index " + std::to_string(Index));
        }
#else
    traits::bind(Stmt, ++Index, Value);
#endif
    });
}

} // namespace sqldb
