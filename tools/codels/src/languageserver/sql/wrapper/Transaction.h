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

#include "Connection.h"

namespace sqldb {
/**
 * RAII-style wrapper object to deal with transactions.
 */
class Transaction {
public:
    /**
     * Create transaction object and start a new transaction.
     */
    explicit Transaction(Connection &DB);

    /**
     * Rollback current transaction if it was not already
     * committed or rolled back.
     */
    ~Transaction() noexcept;

    /**
     * Transaction objects are move-constructible.
     */
    Transaction(Transaction &&Other) noexcept;

    /**
     * Transaction objects are move-assignable.
     */
    Transaction &operator=(Transaction &&Other) noexcept;

    /**
     * Transaction objects are not copy-constructible.
     */
    Transaction(const Transaction &) = delete;

    /**
     * Transaction objects are not copy-assignable.
     */
    Transaction &operator=(const Transaction &) = delete;

    /**
     * Exchange the connection handle and current state
     * of the transaction with those of Other.
     */
    void swap(Transaction &Other) noexcept;

    /**
     * Commit current transaction. Does nothing if the transaction
     * was already committed or rolled back.
     */
    void commit();

    /**
     * Rollback current transaction. Does nothing if the transaction
     * was already committed or rolled back.
     */
    void rollback();

private:
    Connection *DB = nullptr;
};

} // namespace sqldb
