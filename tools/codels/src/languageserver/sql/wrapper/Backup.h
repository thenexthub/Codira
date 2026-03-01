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
#include "SQLiteAPI.h"

#include <string>

namespace sqldb {
/**
 * Backup operation between source and destination SQLite databases.
 */
class Backup {
public:
    /**
     * Create backup operation object from SQLite native handle. The backup
     * object will manage the lifetime of the passed native handle.
     */
    explicit Backup(sqlite3_backup *P) noexcept;

    /**
     * Release all resources associated with managed SQLite native handle.
     */
    ~Backup() noexcept;

    /**
     * Backup objects are move-constructible.
     */
    Backup(Backup &&Other) noexcept;

    /**
     * Backup objects are move-assignable.
     */
    Backup &operator=(Backup &&Other) noexcept;

    /**
     * Backup objects are not copy-constructible.
     */
    Backup(const Connection &) = delete;

    /**
     * Backup objects are not copy-assignable.
     */
    Backup &operator=(const Backup &) = delete;

    /**
     * Exchange SQLite native handle of the backup with those of Other
     * backup object.
     */
    void swap(Backup &Other) noexcept;

    /**
     * Return the total number of pages in the source database.
     */
    int getTotalPageCount() const noexcept;

    /**
     * Return the number of pages still to be backed up.
     */
    int getRemainingPages() const noexcept;

    /**
     * Copy up to Pages between the source and the destination databases.
     * If Pages is negative, all remaining source pages are copied.
     */
    void step(int Pages);

    /**
     * Copy all pages between the source and the destination databases.
     */
    void execute();

private:
    sqlite3_backup *P = nullptr;
};

/**
 * Creates SQLite database backup operation object.
 */
Backup backup(Connection &Dst, Connection &Src, const std::string &Name = "main");

} // namespace sqldb
