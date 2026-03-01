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

#include "Memory.h"

#include "sqlite3.h"

#include <stdexcept>

namespace sqldb {

std::size_t getMemoryUsed() noexcept { return static_cast<std::size_t>(sqlite3_memory_used()); }

std::size_t getMemoryHighWater(bool Reset) noexcept
{
    return static_cast<std::size_t>(sqlite3_memory_highwater(Reset));
}

std::size_t releaseMemory() noexcept { return static_cast<std::size_t>(sqlite3_release_memory(-1)); }

std::size_t getSoftHeapLimit() noexcept { return static_cast<std::size_t>(sqlite3_soft_heap_limit64(-1)); }

std::size_t getHardHeapLimit() noexcept { return static_cast<std::size_t>(sqlite3_hard_heap_limit64(-1)); }

void setSoftHeapLimit(std::size_t N)
{
    if (sqlite3_soft_heap_limit64(static_cast<sqlite_int64>(N)) < 0) {
        throw std::runtime_error("Failed to set SQLite soft heap limit");
    }
}

void setHardHeapLimit(std::size_t N)
{
    if (sqlite3_hard_heap_limit64(static_cast<sqlite_int64>(N)) < 0) {
        throw std::runtime_error("Failed to set SQLite hard heap limit");
    }
}

} // namespace sqldb
