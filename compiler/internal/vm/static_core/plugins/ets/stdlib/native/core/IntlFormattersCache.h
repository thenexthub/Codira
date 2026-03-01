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

#ifndef PANDA_PLUGINS_ETS_STDLIB_NATIVE_CORE_INTLFORMATTERSCACHE_H
#define PANDA_PLUGINS_ETS_STDLIB_NATIVE_CORE_INTLFORMATTERSCACHE_H

#include "plugins/ets/stdlib/native/core/stdlib_ani_helpers.h"
#include "libarkbase/macros.h"
#include "libarkbase/os/mutex.h"
#include "IntlNumberFormatters.h"
#include "unicode/coll.h"

#include <unordered_map>
#include <memory>
#include <iterator>
#include <random>

namespace ark::ets::stdlib::intl {

/// Contains information about cache with LocalizedNumberFormatter and LocalizedNumberRangeFormatter
class IntlFormattersCache final {
public:
    NO_COPY_SEMANTIC(IntlFormattersCache);
    NO_MOVE_SEMANTIC(IntlFormattersCache);

    IntlFormattersCache();
    ~IntlFormattersCache() = default;

    LocNumFmt &NumFmtsCacheInvalidation(ani_env *env, const ParsedOptions &options, ani_status &status);

    LocNumRangeFmt &NumRangeFmtsCacheInvalidation(ani_env *env, const ParsedOptions &options, ani_status &status);

private:
    struct NumberFormatters {
        std::unique_ptr<LocNumFmt> numFmt;
        std::unique_ptr<LocNumRangeFmt> numRangeFmt;
        std::unique_ptr<ParsedOptions> options;
    };

    using CacheUMap = std::unordered_map<std::string, NumberFormatters>;
    using CacheUMapIterator = CacheUMap::iterator;

    os::memory::RecursiveMutex mtx_;
    CacheUMap cache_ GUARDED_BY(mtx_);
    static constexpr uint32_t MAX_SIZE_CACHE = 512U;
    static constexpr double ERASE_RATIO = 0.1;  // Erase 10% of MAX_SIZE_CACHE
    static constexpr uint32_t ERASE_AMOUNT = std::max(1U, static_cast<uint32_t>(MAX_SIZE_CACHE *ERASE_RATIO));

    void EraseRandFmtsGroupByEraseRatio();
};

}  // namespace ark::ets::stdlib::intl

#endif  //  PANDA_PLUGINS_ETS_STDLIB_NATIVE_CORE_INTLFORMATTERSCACHE_H
