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

#include "IntlCollatorCache.h"
#include "IntlLocaleMatch.h"
#include "stdlib_ani_helpers.h"
#include <unicode/ucol.h>

namespace ark::ets::stdlib::intl {

IntlCollatorCache::IntlCollatorCache()
{
    ASSERT(MAX_SIZE_CACHE > 1U);
    ASSERT(ERASE_RATIO > 0);
    ASSERT(ERASE_RATIO < 1);
    ASSERT(ERASE_AMOUNT > 0);
}

icu::Collator *IntlCollatorCache::GetOrCreateCollator(ani_env *env, const std::string &lang,
                                                      const std::string &collation, const std::string &caseFirst)
{
    std::string cacheKey = lang + ";" + collation + ";" + caseFirst;

    os::memory::LockHolder lh(mtx_);
    auto it = cache_.find(cacheKey);
    if (it != cache_.end()) {
        return it->second.get();
    }
    // Cache miss, create a new instance
    EraseRandFmtsGroupByEraseRatio();

    auto locale = GetLocale(env, const_cast<std::string &>(lang));
    UErrorCode status = U_ZERO_ERROR;
    if (!collation.empty() && collation != "default") {
        locale.setKeywordValue("collation", collation.c_str(), status);
        if (UNLIKELY(U_FAILURE(status))) {
            const auto errorMessage =
                std::string("Collation '").append(collation).append("' is invalid or not supported");
            ThrowNewError(env, "std.core.RuntimeError", errorMessage.c_str(), "C{std.core.String}:");
            return nullptr;
        }
    }

    status = U_ZERO_ERROR;
    icu::Collator *newCollator = icu::Collator::createInstance(locale, status);
    if (UNLIKELY(U_FAILURE(status)) || newCollator == nullptr) {
        icu::UnicodeString dispName;
        locale.getDisplayName(dispName);
        std::string localeName;
        dispName.toUTF8String(localeName);
        const auto errorMessage = std::string("Failed to create the collator for ").append(localeName);
        ThrowNewError(env, "std.core.RuntimeError", errorMessage.c_str(), "C{std.core.String}:");

        if (newCollator != nullptr) {
            delete newCollator;
        }
        return nullptr;
    }

    status = U_ZERO_ERROR;
    if (caseFirst == "upper") {
        newCollator->setAttribute(UCOL_CASE_FIRST, UCOL_UPPER_FIRST, status);
    } else if (caseFirst == "lower") {
        newCollator->setAttribute(UCOL_CASE_FIRST, UCOL_LOWER_FIRST, status);
    } else {
        newCollator->setAttribute(UCOL_CASE_FIRST, UCOL_OFF, status);
    }

    if (UNLIKELY(U_FAILURE(status))) {
        delete newCollator;
        return nullptr;
    }

    status = U_ZERO_ERROR;
    newCollator->setAttribute(UCOL_NORMALIZATION_MODE, UCOL_ON, status);

    if (UNLIKELY(U_FAILURE(status))) {
        const auto errorMessage = std::string("Failed to enable normalization for collator.");
        ThrowNewError(env, "std.core.RuntimeError", errorMessage.c_str(), "C{std.core.String}:");
        delete newCollator;
        return nullptr;
    }

    auto [iter, inserted] = cache_.insert_or_assign(cacheKey, std::unique_ptr<icu::Collator>(newCollator));
    return iter->second.get();
}

void IntlCollatorCache::EraseRandFmtsGroupByEraseRatio()
{
    os::memory::LockHolder lh(mtx_);
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

}  //  namespace ark::ets::stdlib::intl
