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

#include "plugins/ets/stdlib/native/core/IntlRelativeTimeFormatCache.h"
#include <ctime>
#include <random>
#include <unicode/numfmt.h>

namespace ark::ets::stdlib::intl {

constexpr uint32_t MAX_SIZE_CACHE = 10;
constexpr uint32_t ERASE_AMOUNT = 2;

IntlRelativeTimeFormatCache::IntlRelativeTimeFormatCache()
{
    ASSERT(MAX_SIZE_CACHE > 1U);
    ASSERT(ERASE_AMOUNT > 0);
    ASSERT(ERASE_AMOUNT <= MAX_SIZE_CACHE);
}

icu::RelativeDateTimeFormatter *IntlRelativeTimeFormatCache::GetOrCreateFormatter(
    const std::string &cacheKey, const icu::Locale &locale, UDateRelativeDateTimeFormatterStyle style)
{
    os::memory::LockHolder lock(mtx_);
    auto it = cache_.find(cacheKey);
    if (it != cache_.end()) {
        return it->second.get();
    }

    // cache miss
    EraseRandFmtsGroupByEraseRatio();

    UErrorCode status = U_ZERO_ERROR;
    icu::NumberFormat *icuNumberFormat = icu::NumberFormat::createInstance(locale, UNUM_DECIMAL, status);
    if (U_FAILURE(status)) {
        return nullptr;
    }

    auto newFormatter = std::make_unique<icu::RelativeDateTimeFormatter>(locale, icuNumberFormat, style,
                                                                         UDISPCTX_CAPITALIZATION_NONE, status);
    if (U_FAILURE(status)) {
        return nullptr;
    }

    auto *formatterPtr = newFormatter.get();
    cache_[cacheKey] = std::move(newFormatter);
    return formatterPtr;
}

void IntlRelativeTimeFormatCache::EraseRandFmtsGroupByEraseRatio()
{
    if (cache_.size() >= MAX_SIZE_CACHE) {
        static std::minstd_rand simpleRand(std::time(nullptr));
        auto numToErase = ERASE_AMOUNT;
        if (cache_.size() < MAX_SIZE_CACHE - 1 + numToErase) {
            numToErase = cache_.size() - MAX_SIZE_CACHE + 1;
        }

        for (uint32_t i = 0; i < numToErase; ++i) {
            auto delta = static_cast<uint32_t>(simpleRand() % cache_.size());
            auto it = cache_.begin();
            std::advance(it, delta);
            cache_.erase(it);
        }
    }
}

}  // namespace ark::ets::stdlib::intl