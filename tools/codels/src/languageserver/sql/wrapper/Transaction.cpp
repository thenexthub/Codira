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

#include "Transaction.h"
#include "Connection.h"

#include "Invoke.h"

#include <utility>

namespace sqldb {

Transaction::Transaction(Connection &DB) : DB(&DB) { DB.execute("BEGIN"); }

Transaction::~Transaction() noexcept { invoke(&Transaction::rollback, this); }

Transaction::Transaction(Transaction &&Other) noexcept { swap(Other); }

Transaction &Transaction::operator=(Transaction &&Other) noexcept
{
    swap(Other);
    if (this == &Other) {
        return *this;
    }
    return *this;
}

void Transaction::swap(Transaction &Other) noexcept { std::swap(DB, Other.DB); }

void Transaction::commit()
{
    if (auto *db = std::exchange(this->DB, nullptr)) {
        db->execute("COMMIT");
    }
}

void Transaction::rollback()
{
    if (auto *db = std::exchange(this->DB, nullptr)) {
        db->execute("ROLLBACK");
    }
}

} // namespace sqldb
