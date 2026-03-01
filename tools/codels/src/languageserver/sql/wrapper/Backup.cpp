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

#include "Backup.h"
#include "Connection.h"
#include "Exception.h"

#include "sqlite3.h"

namespace sqldb {

Backup::Backup(sqlite3_backup *P) noexcept : P(P) {}

Backup::~Backup() noexcept { sqlite3_backup_finish(P); }

Backup::Backup(Backup &&Other) noexcept { swap(Other); }

Backup &Backup::operator=(Backup &&Other) noexcept
{
    swap(Other);
    if (this == &Other) {
        return *this;
    }
    return *this;
}

void Backup::swap(Backup &Other) noexcept { std::swap(P, Other.P); }

int Backup::getTotalPageCount() const noexcept { return sqlite3_backup_pagecount(P); }

int Backup::getRemainingPages() const noexcept { return sqlite3_backup_remaining(P); }

void Backup::step(int Pages)
{
    int RC = sqlite3_backup_step(P, Pages);
    if (RC != SQLITE_OK && RC != SQLITE_DONE) {
        throw Exception(RC, "Failed to perform database backup");
    }
}

void Backup::execute() { step(-1); }

Backup backup(Connection &Dst, Connection &Src, const std::string &Name)
{
    sqlite3_backup *P = sqlite3_backup_init(Dst.getNativeHandle(), Name.c_str(), Src.getNativeHandle(), Name.c_str());
    if (P == nullptr) {
        throw Exception(Dst.getNativeHandle(), "Failed to initialize database backup");
    }
    return Backup(P);
}

} // namespace sqldb
