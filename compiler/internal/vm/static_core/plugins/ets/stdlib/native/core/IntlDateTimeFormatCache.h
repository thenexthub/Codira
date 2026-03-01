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

#ifndef PANDA_PLUGINS_ETS_STDLIB_NATIVE_CORE_INTLDATETIMEFORMATCACHE_H
#define PANDA_PLUGINS_ETS_STDLIB_NATIVE_CORE_INTLDATETIMEFORMATCACHE_H

#include "libarkbase/macros.h"
#include "libarkbase/os/mutex.h"
#include <ani.h>
#include <unicode/datefmt.h>
#include <string>
#include <memory>
#include <unordered_map>
#include <random>

namespace ark::ets::stdlib::intl {

// Forward declaration of the factory function from IntlDateTimeFormat.cpp
// This function will be used by the cache on a cache miss.
std::unique_ptr<icu::DateFormat> CreateICUDateFormat(ani_env *env, ani_object self);

class IntlDateTimeFormatCache {
public:
    NO_COPY_SEMANTIC(IntlDateTimeFormatCache);
    NO_MOVE_SEMANTIC(IntlDateTimeFormatCache);

    IntlDateTimeFormatCache();
    ~IntlDateTimeFormatCache() = default;

    /**
     * @brief Retrieves a cached DateFormat instance or creates a new one.
     *
     * This is the main entry point for the cache. It generates a unique key
     * from the ETS DateTimeFormat object's properties and uses it to
     * look up a pre-configured icu::DateFormat object.
     *
     * @param env The ANI environment.
     * @param self The ETS `std.core.Intl.DateTimeFormat` instance.
     * @return A pointer to the cached or newly created icu::DateFormat object.
     */
    icu::DateFormat *GetOrCreateDateFormat(ani_env *env, ani_object self, const std::string &cacheKey);

private:
    using CacheUMap = std::unordered_map<std::string, std::unique_ptr<icu::DateFormat>>;

    void EraseRandFmtsGroupByEraseRatio() REQUIRES(mtx_);

    os::memory::RecursiveMutex mtx_;
    CacheUMap cache_ GUARDED_BY(mtx_);

    // Cache size constants, consistent with other Intl caches.
    static constexpr uint32_t MAX_SIZE_CACHE = 512U;
    static constexpr double ERASE_RATIO = 0.1;
    static constexpr uint32_t ERASE_AMOUNT = std::max(1U, static_cast<uint32_t>(MAX_SIZE_CACHE *ERASE_RATIO));
};

}  // namespace ark::ets::stdlib::intl

#endif  // PANDA_PLUGINS_ETS_STDLIB_NATIVE_CORE_INTLDATETIMEFORMATCACHE_H