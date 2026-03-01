/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "plugins/ets/stdlib/native/core/IntlDateTimeFormatCache.h"
#include "plugins/ets/stdlib/native/core/stdlib_ani_helpers.h"
#include <ctime>
#include <sstream>

namespace ark::ets::stdlib::intl {

IntlDateTimeFormatCache::IntlDateTimeFormatCache()
{
    ASSERT(MAX_SIZE_CACHE > 1U);
    ASSERT(ERASE_RATIO > 0);
    ASSERT(ERASE_RATIO < 1);
    ASSERT(ERASE_AMOUNT > 0);
}

icu::DateFormat *IntlDateTimeFormatCache::GetOrCreateDateFormat(ani_env *env, ani_object self,
                                                                const std::string &cacheKey)
{
    // --- Cache Lookup ---
    os::memory::LockHolder lock(mtx_);

    auto it = cache_.find(cacheKey);
    if (it != cache_.end()) {
        // Cache Hit
        return it->second.get();
    }
    // --- Cache Miss ---
    EraseRandFmtsGroupByEraseRatio();
    // Create the formatter by calling the factory function.
    std::unique_ptr<icu::DateFormat> new_format = CreateICUDateFormat(env, self);
    if (new_format == nullptr) {
        return nullptr;
    }

    auto *format_ptr = new_format.get();
    cache_.emplace(cacheKey, std::move(new_format));

    return format_ptr;
}

void IntlDateTimeFormatCache::EraseRandFmtsGroupByEraseRatio()
{
    if (cache_.size() == MAX_SIZE_CACHE) {
        // NOLINTNEXTLINE(cert-msc51-cpp)
        static std::minstd_rand simpleRand(std::time(nullptr));
        auto delta = static_cast<uint32_t>(simpleRand() % (MAX_SIZE_CACHE - ERASE_AMOUNT + 1U));

        auto firstIt = cache_.begin();
        std::advance(firstIt, delta);
        auto lastIt = firstIt;
        std::advance(lastIt, ERASE_AMOUNT);
        cache_.erase(firstIt, lastIt);
    }
}

}  // namespace ark::ets::stdlib::intl