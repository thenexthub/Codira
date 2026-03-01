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

#include "plugins/ets/stdlib/native/core/IntlFormattersCache.h"

namespace ark::ets::stdlib::intl {

IntlFormattersCache::IntlFormattersCache()
{
    ASSERT(MAX_SIZE_CACHE > 1U);
    ASSERT(ERASE_RATIO > 0);
    ASSERT(ERASE_RATIO < 1);
    ASSERT(ERASE_AMOUNT > 0);
}

LocNumFmt &IntlFormattersCache::NumFmtsCacheInvalidation(ani_env *env, const ParsedOptions &options, ani_status &status)
{
    static LocNumFmt defaultLocNumFmt;
    auto tag = options.TagString();
    status = ANI_OK;
    CacheUMapIterator it;
    {
        os::memory::LockHolder lh(mtx_);
        it = cache_.find(tag);
        if (it == cache_.end()) {
            EraseRandFmtsGroupByEraseRatio();
            // Create new number formatter, number range formatter is empty
            // Number range formatter will be created via call NumRangeFmtsCacheInvalidation
            auto *ptr = new icu::number::LocalizedNumberFormatter();
            if (UNLIKELY(ptr == nullptr)) {
                status = ANI_PENDING_ERROR;
                std::string message = "Create Icu LocalizedNumberFormatter failed";
                ThrowNewError(env, ERR_CLS_RUNTIME_EXCEPTION, message.c_str(), CTOR_SIGNATURE_STR);
                return defaultLocNumFmt;
            }

            // Set options
            status = InitNumFormatter(env, options, *ptr);
            if (status != ANI_OK) {
                return defaultLocNumFmt;
            }

            // Save into cache
            NumberFormatters f;
            f.numFmt.reset(ptr);
            auto [iter, isNumFmtInserted] = cache_.insert_or_assign(tag, std::move(f));
            if (!isNumFmtInserted) {
                status = ANI_PENDING_ERROR;
                std::string message = "Create and insert Icu LocalizedNumberFormatter failed";
                ThrowNewError(env, ERR_CLS_RUNTIME_EXCEPTION, message.c_str(), CTOR_SIGNATURE_STR);
                return defaultLocNumFmt;
            }
            it = iter;
        } else if (it->second.numFmt == nullptr) {
            // Still not created, now create new number formatter, range formatter is not changed
            auto *ptr = new icu::number::LocalizedNumberFormatter();
            if (UNLIKELY(ptr == nullptr)) {
                status = ANI_PENDING_ERROR;
                std::string message = "Create Icu LocalizedNumberFormatter failed";
                ThrowNewError(env, ERR_CLS_RUNTIME_EXCEPTION, message.c_str(), CTOR_SIGNATURE_STR);
                return defaultLocNumFmt;
            }
            status = InitNumFormatter(env, options, *ptr);
            if (status != ANI_OK) {
                return defaultLocNumFmt;
            }
            it->second.numFmt.reset(ptr);
        }
    }
    return *(it->second.numFmt);
}

LocNumRangeFmt &IntlFormattersCache::NumRangeFmtsCacheInvalidation(ani_env *env, const ParsedOptions &options,
                                                                   ani_status &status)
{
    static LocNumRangeFmt defaultLocNumRangeFmt;
    auto tag = options.TagString();
    status = ANI_OK;
    CacheUMapIterator it;
    {
        os::memory::LockHolder lh(mtx_);
        it = cache_.find(tag);
        if (it == cache_.end()) {
            EraseRandFmtsGroupByEraseRatio();
            // Create new number range formatter, number formatter is empty
            // Number formatter will be created via call IntlFormattersCacheInvalidation
            auto *ptr = new icu::number::LocalizedNumberRangeFormatter();
            if (UNLIKELY(ptr == nullptr)) {
                status = ANI_PENDING_ERROR;
                std::string message = "Create Icu LocalizedNumberRangeFormatter failed";
                ThrowNewError(env, ERR_CLS_RUNTIME_EXCEPTION, message.c_str(), CTOR_SIGNATURE_STR);
                return defaultLocNumRangeFmt;
            }

            // Set options
            status = InitNumRangeFormatter(env, options, *ptr);
            if (status != ANI_OK) {
                return defaultLocNumRangeFmt;
            }

            // Save into cache
            NumberFormatters f;
            f.numRangeFmt.reset(ptr);
            auto [iter, isNumRangeFmtInserted] = cache_.insert_or_assign(tag, std::move(f));
            if (!isNumRangeFmtInserted) {
                status = ANI_PENDING_ERROR;
                std::string message = "Create and insert Icu LocalizedNumberRangeFormatter failed";
                ThrowNewError(env, ERR_CLS_RUNTIME_EXCEPTION, message.c_str(), CTOR_SIGNATURE_STR);
                return defaultLocNumRangeFmt;
            }
            it = iter;
        } else if (it->second.numRangeFmt == nullptr) {
            // Still not created, now create new number range formatter, number formatter is not changed
            auto *ptr = new icu::number::LocalizedNumberRangeFormatter();
            if (UNLIKELY(ptr == nullptr)) {
                status = ANI_PENDING_ERROR;
                std::string message = "Create Icu LocalizedNumberRangeFormatter failed";
                ThrowNewError(env, ERR_CLS_RUNTIME_EXCEPTION, message.c_str(), CTOR_SIGNATURE_STR);
                return defaultLocNumRangeFmt;
            }

            status = InitNumRangeFormatter(env, options, *ptr);
            if (status != ANI_OK) {
                return defaultLocNumRangeFmt;
            }
            it->second.numRangeFmt.reset(ptr);
        }
    }
    return *(it->second.numRangeFmt);
}

void IntlFormattersCache::EraseRandFmtsGroupByEraseRatio()
{
    os::memory::LockHolder lh(mtx_);
    // Remove random N "neighbours" items (group) from cache_ if size is maximum
    if (cache_.size() == MAX_SIZE_CACHE) {
        // NOLINTNEXTLINE(cert-msc51-cpp)
        static std::minstd_rand simpleRand(std::time(nullptr));
        auto delta = static_cast<uint32_t>(simpleRand() % (MAX_SIZE_CACHE - ERASE_AMOUNT + 1U));
        // delta is in random range [0; MAX_SIZE_CACHE - ERASE_AMOUNT]
        auto firstIt = cache_.begin();
        std::advance(firstIt, delta);
        auto lastIt = firstIt;
        std::advance(lastIt, ERASE_AMOUNT);
        cache_.erase(firstIt, lastIt);
    }
}

}  // namespace ark::ets::stdlib::intl
